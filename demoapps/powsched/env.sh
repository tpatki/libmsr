export EXPOSE_MASTER_PORT=8001
export EXPOSE_MASTER_HOST=`hostname`

echo "$EXPOSE_MASTER_HOST:$EXPOSE_MASTER_PORT"
