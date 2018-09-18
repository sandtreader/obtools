# prepare rpm build from .deb file

OUR_DIRECTORIES=
RPM_SERVICE=
DEB_DIR1=`pwd`/debian
RPM_SPEC=`pwd`/debian/spec
RPM_ARCH=`uname -m`

BUILDROOT=`mktemp -d`
fakeroot dpkg -x $PACKAGE $BUILDROOT
cd $BUILDROOT

set_rpm_arch()
{
  local DEBARCH=`egrep "^Architecture: " $DEB_DIR1/control|tail -n 1|cut -d\  -f2`
  if [ "$DEBARCH" = "all" ]; then
    RPM_ARCH=noarch
  fi 
}

fix_init()
{
  # /etc/init.d/ is a symlink in redhat land, so
  if [ -d etc/init.d ]; then
     RPM_RCDIR=etc/rc.d/init.d
     mkdir -p $RPM_RCDIR
     mv etc/init.d/${NAME} $RPM_RCDIR
     rmdir etc/init.d
  fi
}

fix_nobody()
{
  # different group for priv drop
  CFG=${NAME##*-}.cfg.xml
  for f in `find -type f -name $CFG`
  do
    RPM_SERVICE=1
    sed -i 's/user="nobody" group="nogroup"/user="nobody" group="nobody"/' $f
  done
}

get_our_directories()
{
  for d in `find -type d |sed 's/^\.//g'`
  do
    b=$(basename $d)
    if [ "$b" = "obtools" ] || [ "$b" = "packetship" ] || [ "$b" = "$NAME" ]; then
      OUR_DIRECTORIES="$OUR_DIRECTORIES $d"
    fi
  done
}

# Directory permission changes in CentOS 7 cause fatal install errors.
# We shouldn't be creating these directories anyway, so remove them from the
#  spec file
fix_spec_centos7()
{
  CentOS7=`lsb_release -d -s |egrep "CentOS Linux release 7"|wc -l`
  if [ "$CentOS7" = 1 ]; then
    sed -i 's/^%dir "\/\(usr\/\(s\|\)bin\/\|\)"$//g' $RPM_SPEC
  fi
}

fix_perms_centos7()
{
  find . -type d -exec chmod g-w {} \;
}

set_rpm_values()
{
  RPM_NS=`echo ${NAME}|cut -c1-3`
  RPM_SERVERNAME=`echo ${NAME}|cut -c4-`

  case "${RPM_NS}" in
  "ot-")
    RPM_COPYRIGHT="xMill Consulting Ltd"
    RPM_NAMETAG="Obtools"
    RPM_CONFIGDIR=/etc/obtools
    ;;
  *)
    RPM_COPYRIGHT="Packet Ship Technologies Ltd"
    RPM_NAMETAG="Packet Ship"
    RPM_CONFIGDIR=/etc/packetship
    ;;
  esac
}

write_spec_header()
{
  echo "Name: ${NAME}${VARIANT}"
  echo "Version: ${VERSION}"
  echo "Release: ${REVISION}"
  grep "^Description:" $DEB_DIR1/control|sed -e 's/^Description:/Summary:/'
  echo "Group: optional"
  grep "^Maintainer:" $DEB_DIR1/control |sed -e 's/Maintainer:/Packager:/'
  echo "License: ${RPM_COPYRIGHT}"
  echo "Distribution: rpm"
  echo "%define _rpmdir ../"
  echo "%define _rpmfilename %%{NAME}-%%{VERSION}-%%{RELEASE}.${RPM_ARCH}.rpm"
  echo "%define _unpackaged_files_terminate_build 0"
  echo "%description"
  awk '/^Description:/ { founddesc=1;print $0;next }
  { if ( founddesc == 1 ) { if ( substr($0,1,1) == " " ) { print $0 } else { exit } }}'  $DEB_DIR1/control |sed -e 's/^Description://' -e 's/^ \.$//' -e 's/^ //'
}

write_init_file()
{
  if [ -f rpm_init_script.sh ]; then
    cp rpm_init_script.sh ${RPM_RCDIR}/${NAME}
    chown 755 ${RPM_RCDIR}/${NAME}
    return
  fi
  NAMETAG="${RPM_NAMETAG} ${RPM_SERVERNAME} server: ${NAME}"
  cat > ${RPM_RCDIR}/${NAME} <<-EOF
#!/bin/sh
# 
# ${NAME} startup script
# chkconfig: 345 80 10
# description: ${NAMETAG} 

### BEGIN INIT INFO
# Provides: ${NAME}
# Required-Start: \$network \$remote_fs
# Required-Stop: \$network \$remote_fs
# Default-Start: 3 4 5
# Default-Stop: 0 1 2 6
# Description: ${NAMETAG}
### END INIT INFO

# Source function library
. /etc/rc.d/init.d/functions

exec=/usr/sbin/${NAME}
prog=${NAME}
config=${RPM_CONFIGDIR}/${CFG}

pidfile=/var/run/${NAME}.pid
lockfile=/var/lock/subsys/\$prog
slavepid=

setslavepid()
{
  if [ -f \$pidfile ]; then
    masterpid=\`cat \$pidfile\`
    if [ "\$masterpid" != "" ]; then
      slavepid=\`ps --ppid \$masterpid -o pid=|sed 's/ //g'\`
    fi
  fi
}

killslavepid()
{
  if [ "\$slavepid" != "" ]; then
    slaverunning=\`ps --pid \$slavepid -o pid=|sed 's/ //g'\`
    if [ "\$slaverunning" = "\$slavepid" -a -d /proc/\$slavepid ]; then
      kill -9 \$slavepid
    fi
  fi
}

start() {
  [ -x \$exec ] || exit 5
  [ -f \$config ] || exit 6
  echo -n \$"Starting ${NAMETAG}"
  daemon \$exec
  retval=\$?
  echo
  [ \$retval -eq 0 ] && touch \$lockfile
  return \$retval
}

stop() {
  echo -n \$"Stopping ${NAMETAG}"
  setslavepid
  killproc -p \$pidfile  ${NAME}
  killslavepid
  retval=\$?
  echo
  [ \$retval -eq 0 ] && rm -f \$lockfile
  return \$retval
}

restart() {
  stop
  start
}

reload() {
  restart
}

force_reload() {
  restart
}

rh_status() {
  status -p \$pidfile \$prog
}

rh_status_q() {
  rh_status > /dev/null 2>&1
}


case "\$1" in
    start)
        rh_status_q && exit 0
        \$1
        ;;
    stop)
        rh_status_q || exit 0
        \$1
        ;;
    restart)
        \$1
        ;;
    reload)
        rh_status_q || exit 7
        \$1
        ;;
    force-reload)
        force_reload
        ;;
    status)
        rh_status
        ;;
    condrestart|try-restart)
        rh_status_q || exit 0
        restart
        ;;
    *)
        echo \$"Usage: \$0 {start|stop|status|restart|condrestart|try-restart|reload|force-reload}"
        exit 2
esac
exit \$?

EOF

chmod 755 ${RPM_RCDIR}/${NAME}
}

write_file_section()
{
  echo "%files"
  #get length of directory name
  NUMCHAR=2

  # all our dirs first
  if [ "$OUR_DIRECTORIES" ]; then
    for directory in $OUR_DIRECTORIES
    do
      echo "%dir \"${directory}\""
    done
  fi

  #all files
  tar cvf /dev/null . |grep  -v "/$" |grep -v $DEB_VERSION.spec |cut -c ${NUMCHAR}-| grep -v "^/DEBIAN/"|sed -e 's/^/\"/' -e 's/$/\"/'
}

fix_nobody
fix_init
set_rpm_arch
get_our_directories
set_rpm_values
write_spec_header > ${RPM_SPEC}

if [ $RPM_SERVICE ]; then
  write_init_file
fi

write_file_section >> ${RPM_SPEC}

fix_spec_centos7
fix_perms_centos7

fakeroot -- rpmbuild --buildroot $BUILDROOT -bb  $RPM_SPEC

mv -v ../${NAME}-${VERSION}-*.rpm $OLDPWD
cd $OLDPWD
rm -rf $BUILDROOT
