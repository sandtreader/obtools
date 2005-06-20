#!/bin/sh
# Start/stop the ObTools XMLMesh server.

test -f /usr/sbin/xmlmesh-server || exit 0

case "$1" in
start)	echo -n "Starting ObTools XMLMesh server"
        start-stop-daemon --start --quiet --pidfile /var/run/ot-xmlmesh.pid --name xmlmesh-server --startas /usr/sbin/xmlmesh-server
        echo "." 
	;;
stop)	echo -n "Stopping ObTools XMLMesh server"
        start-stop-daemon --stop --quiet --pidfile /var/run/ot-xmlmesh.pid --name xmlmesh-server 
        echo "."
        ;;
restart|force-reload) echo -n "Restarting ObTools XMLMesh server"
        start-stop-daemon --stop --retry 5 --quiet --pidfile /var/run/ot-xmlmesh.pid --name xmlmesh-server
        start-stop-daemon --start --quiet --pidfile /var/run/ot-xmlmesh.pid --name xmlmesh-server --startas /usr/sbin/xmlmesh-server
        echo "."
        ;;
*)	echo "Usage: /etc/init.d/xmlmesh-server start|stop|restart|force-reload"
        exit 1 
        ;;
esac
exit 0
