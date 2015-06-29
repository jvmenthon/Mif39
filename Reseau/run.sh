# NGUYEN LE Duc Tan
# MIF39 Lyon 1 Informatique

PROGRAM=./p2p_node	

if [ ! -e "$TERMINAL" ] ; then
			export TERMINAL=xterm
fi

IP=127.0.0.1

DIR=./repertoire_node_

start_server() 
{
NUM=$1
i=1
until [ ${NUM} -lt ${i} ] ; do
		${TERMINAL} -geometry 99x24 -title Node_T${i} -e ${PROGRAM} -ui ${i}0001 -tcp ${i}0002 -udp ${i}0003 -name server${i} -dir ${DIR}${i} &
		mkdir -p ${DIR}${i}
		let i=i+1
done
}


start_client()
{
		NUM=$1
		${TERMINAL} -geometry 99x24 -title ControlUI_Node_${NUM} -e telnet ${IP} ${NUM}0001 &
}


usage() 
{
cat << EOF

START SERVER by adding the following command:
	d [n] : start [n] detached node
	c [n] : start a control UI [for node n]
	k     : kill all servers

EOF
}

case "$1" in
	"c")
	   NUM=$2
                make
		start_client ${NUM}
	;;
	"d")
	   NUM=$2
                make
		start_server ${NUM}
	;;
	"k")
		killall -9 p2p_node
	;;
	*)
		usage
	;;
esac
