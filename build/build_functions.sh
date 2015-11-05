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
