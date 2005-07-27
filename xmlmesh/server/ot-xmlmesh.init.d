#!/bin/sh
# Start/stop the ObTools XMLMesh server.

test -f /usr/sbin/ot-xmlmesh || exit 0

case "$1" in
start)	echo -n "Starting ObTools XMLMesh server: ot-xmlmesh"
        start-stop-daemon --start --quiet --pidfile /var/run/ot-xmlmesh.pid --name ot-xmlmesh --startas /usr/sbin/ot-xmlmesh
        echo "." 
	;;
stop)	echo -n "Stopping ObTools XMLMesh server: ot-xmlmesh"
        start-stop-daemon --stop --quiet --pidfile /var/run/ot-xmlmesh.pid --name ot-xmlmesh 
        echo "."
        ;;
restart|force-reload) echo -n "Restarting ObTools XMLMesh server"
        start-stop-daemon --stop --retry 5 --quiet --pidfile /var/run/ot-xmlmesh.pid --name ot-xmlmesh
        start-stop-daemon --start --quiet --pidfile /var/run/ot-xmlmesh.pid --name ot-xmlmesh --startas /usr/sbin/ot-xmlmesh
        echo "."
        ;;
*)	echo "Usage: /etc/init.d/ot-xmlmesh start|stop|restart|force-reload"
        exit 1 
        ;;
esac
exit 0
