#==========================================================================
# Package-building helper functions
#
# Copyright (c) 2013 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

# figure out libssl version, add dependency to control file
set_libssl_depends()
{
  if [ -f $NAME ]; then
    SSLVER=`ldd $NAME|grep libcrypto|awk '{print $1}'|sed 's/libcrypto.so.//'`
  else
    SSLVER=`ldd /usr/sbin/sshd|grep libcrypto|awk '{print $1}'|sed 's/libcrypto.so.//'`
  fi
  if [ $SSLVER ]; then
    sed  -i "s/\(^Depends:.*$\)/\1, libssl$SSLVER/" $DEB_DIR/DEBIAN/control
  fi
}
