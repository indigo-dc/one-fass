Name: one-fass
Version: systemd_devel
Summary: A FairShare Scheduling Service for Opennebula 
Release: 0
License: Apache
Group: System
URL: https://github.com/indigo-dc/one-fass

#Source0: fass-%{version}.zip
Source0: systemd_devel.zip 

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

################################################################################
# Build Requires
################################################################################

# TODO: in realtà devo provare su un fresh install di Centos7
BuildRequires: scons 
BuildRequires: boost
BuildRequires: xmlrpc-c-c++
BuildRequires: xmlrpc-c-client++ 
BuildRequires: xmlrpc-c-devel 
BuildRequires: boost-devel 

################################################################################
# Requires
################################################################################

# TODO: questo forse non è necessario
#Requires: %{name}-common = %{version}

################################################################################
# Main Package
################################################################################

Packager: INDIGO DataCloud

%description
A FairShare Scheduling Service for OpenNebula.

################################################################################
# Package fass service
################################################################################

%package service
Summary: Provides the fass service
Group: System
#Requires: %{name} = %{version}
#Requires: xmlrpc-c
Requires: boost
Requires: xmlrpc-c-c++
Requires: xmlrpc-c-client++ 
Requires: shadow-utils
Requires: coreutils
Requires: glibc-common

%description service
This package provides the Fass daemon (fassd).

################################################################################
# Package common
################################################################################

#%package common
#Summary: Provides the Fass user
#Group: System

#%description common
#This package creates the fassadmin user and group.

################################################################################
# Build and install
################################################################################

%prep
#%setup -q -n one-fass-systemd_devel 
%setup -q  

%build
scons 

%install
export DESTDIR=%{buildroot}
./install.sh

# init scripts
install -p -D -m 755 systemd/fass.service %{buildroot}/lib/systemd/system/fass.service

# logrotate
%{__mkdir} -p %{buildroot}%{_sysconfdir}/logrotate.d
install -p -D -m 440 systemd/fass.logrotate %{buildroot}%{_sysconfdir}/logrotate.d/fass

%clean
%{__rm} -rf %{buildroot}

################################################################################
# common - scripts
################################################################################

#%pre common
#getent group fassadmin >/dev/null || groupadd -r fassadmin
#if getent passwd fassadmin >/dev/null; then
#    /usr/sbin/usermod -a -G fassadmin fassadmin > /dev/null
#else
#    /usr/sbin/useradd -r -M -g fassadmin -s /sbin/nologin fassadmin 2> /dev/null
#fi

################################################################################
# service - scripts
################################################################################

%pre service
# create the fassadmin user
getent group fassadmin >/dev/null || groupadd -r fassadmin
if getent passwd fassadmin >/dev/null; then
    /usr/sbin/usermod -a -G fassadmin fassadmin > /dev/null
else
    /usr/sbin/useradd -r -M -g fassadmin -s /sbin/nologin fassadmin 2> /dev/null
fi

# Upgrade - Stop the service
if [ $1 = 2 ]; then
    /usr/bin/systemctl stop fass >/dev/null || :
fi

%post service
if [ $1 = 1 ]; then
    /usr/bin/systemctl enable fass >/dev/null
fi

%preun service
if [ $1 = 0 ]; then
    /usr/bin/systemctl stop fass >/dev/null || :
    /usr/bin/systemctl disable fass >/dev/null || : 
fi

################################################################################
# service - files
################################################################################

%files service
%defattr(0640, root, fassadmin, 0750)
%dir %{_sysconfdir}/fass
%config %{_sysconfdir}/fass/fassd.conf
%config %{_sysconfdir}/logrotate.d/fass

%defattr(-, root, root, 0755)
/lib/systemd/system/fass.service

%{_bindir}/fassd
