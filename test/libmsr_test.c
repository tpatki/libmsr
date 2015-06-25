#include <unistd.h>
#include <stdio.h>
#include "../include/msr_core.h"
#include "../include/msr_rapl.h"
#include "../include/msr_thermal.h"
#ifdef MPI
#include <mpi.h>
#endif

#define MASK_RANGE(m,n) ((((uint64_t)1<<((m)-(n)+1))-1)<<(n))
#define MASK_VAL(x,m,n) (((uint64_t)(x)&MASK_RANGE((m),(n)))>>(n))

struct rapl_limit l1, l2, l3, l4;

void 
rapl_test(const uint64_t * rapl_flags){
//	read_rapl_data(0, NULL, rapl_flags);	// Initialize
//	sleep(3);

//	dump_rapl_terse_label(stdout);
//	fprintf(stdout, "\n");
//	dump_rapl_terse(stdout, rapl_flags);		// Read and dump.
//	fprintf(stdout, "\n");

}

void
get_limits()
{
	int i;
    fprintf(stderr, "\nGetting limits...\n");
	for(i=0; i<NUM_SOCKETS; i++){
        fprintf(stderr, "\nSocket %d:\n", i);
		//get_rapl_limit(i, &l1, &l2, &l3);
        get_pkg_rapl_limit(i, &l1, &l2);
        get_dram_rapl_limit(i, &l3);
		dump_rapl_limit(&l1, stdout);
		dump_rapl_limit(&l2, stdout);
        printf("DRAM\n");
        dump_rapl_limit(&l3, stdout);
        printf("done...\nPower Plane\n");
        get_pp_rapl_limit(i, &l4, NULL);
        dump_rapl_limit(&l4, stdout);
        printf("done...\n");
	}
}

void
set_limits()
{
	l1.watts = 100;
	l1.seconds = 2;
	l1.bits = 0;
	l2.watts =  130;
	l2.seconds =  2;
	l2.bits = 0;
    set_pkg_rapl_limit(0, &l1, &l2);
    //set_pkg_rapl_limit(1, &l1, &l2);
    l3.watts = 25; //30; //4;
    l3.seconds = 2; // 0.01; //0.01;
    l3.bits = 0; //0x4480b0;
    set_dram_rapl_limit(0, &l3);
    //set_dram_rapl_limit(1, &l3);
	//set_rapl_limit(0, &l1, &l2, &l3);
	//set_rapl_limit(1, &l1, &l2, &l3);
	get_limits();
}

void thermal_test(){
	dump_thermal_terse_label(stdout);
	fprintf(stdout, "\n");
	dump_thermal_terse(stdout);
	fprintf(stdout, "\n");

	dump_thermal_verbose_label(stdout);
	fprintf(stdout, "\n");
	dump_thermal_verbose(stdout);
	fprintf(stdout, "\n");
}

void perform_rapl_measurement(struct rapl_data* r) {
	//read_rapl_data_old(0, r);

    if (r != NULL)
    {
        fprintf(stdout, "old_pkg_joules=%lf pkg_joules=%lf pkg_delta_joules=%lf elapsed=%lf pkg_watts=%lf\n", 
                r->old_pkg_joules, r->pkg_joules, r->pkg_delta_joules, r->elapsed, r->pkg_watts);
    }
}

void rapl_r_test(struct rapl_data ** rd)
{
	// Initialize two separate state objects and read rapl data into them during overlapping time windows
	struct rapl_data * r1; 
	struct rapl_data r2, r3; 

    //perform_rapl_measurement(NULL);
    // set r1/r2.flags to 1 | 2 -> 3
	//r2.flags = RDF_INIT | RDF_REENTRANT;
    //r1.flags = RDF_REENTRANT | RDF_INIT;


    //fprintf(stdout, "\nOLD\n\n");
	//perform_rapl_measurement(&r2);  // Initialize r1
    //dump_rapl_data(&r2, stderr);
	//r2.flags = RDF_REENTRANT;
	//sleep(1);

	//perform_rapl_measurement(&r2);
    //dump_rapl_data(&r2, stderr);
	//sleep(1);

    fprintf(stdout, "\nNEW\n\n");
    r1 = &((*rd)[0]);
    poll_rapl_data(0, r1);
    //delta_rapl_data(0, rd, r1);
    dump_rapl_data(r1, stderr);
    sleep(1);


    poll_rapl_data(0, r1);
    //delta_rapl_data(0, rd, r1);
    dump_rapl_data(r1, stderr);
    sleep(1);
}


// TODO: check if test for oversized bitfield is in place, change that warning to an error
int main(int argc, char** argv)
{
    struct rapl_data * rd = NULL;
    uint64_t * rapl_flags = NULL;
	#ifdef MPI
	MPI_Init(&argc, &argv);
    printf("mpi init done\n");
	#endif

	if(init_msr())
    {
        return -1;
    }
    printf("msr init done\n");
    if (rapl_init(&rd, &rapl_flags))
    {
        return -1;
    }
    printf("init done\n");
	//set_limits();
	//get_limits();
    set_limits();
    printf("set limits done\n");
	//rapl_test();
	rapl_r_test(&rd);
    printf("rapl_r_test done\n");
    //read_rapl_data(0, NULL);
    //read_rapl_data(0, &rd);
    //dump_rapl_data(&rd, stdout);
    //printf("\nOUTPUT: The DRAM used %lf watts. Throttled %lu\n", rd.dram_watts, MASK_VAL(rd.dram_perf_count, 31, 0) - MASK_VAL(rd.old_dram_perf, 31, 0));
    printf("\n\nPOWER INFO\n");
    dump_rapl_power_info(stdout);
    printf("\nEND POWER INFO\n\n");
	//get_limits();
    rapl_finalize(&rd);
	finalize_msr();
	#ifdef MPI
	MPI_Finalize();
	#endif

	return 0;
}
