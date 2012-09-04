#!/bin/bash

pidfile=/var/run/test2.pid


./test-daemon ../test.cfg & 
sleep 1

p=`pidof ./test-daemon`
if [ "$p" != "" ]; then
  kill $p
else
  exit 2
fi


