#!/bin/bash
#
# Copyright © 2017 INFN Torino - INDIGO-DataCloud
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#-------------------------------------------------------------------------------
# Install program for Fass. It will install it relative to $FASS_LOCATION 
# if defined with the -d option, otherwise it'll be installed under /. 
# In this case you may specify the fassadmin user/group, so you do
# not need run the Fass daemon with root privileges
#-------------------------------------------------------------------------------

# Command-line parsing
usage() {
 echo
 echo "Usage: install.sh [-u install_user] [-g install_group] [-k keep conf]"
 echo "                  [-d FASS_LOCATION] [-r] [-h]"
 echo
 echo "-u: user that will run fass, defaults to user executing install.sh"
 echo "-g: group of the user that will run fass, defaults to user"
 echo "    executing install.sh"
 echo "-k: keep configuration files of existing Fass installation, useful"
 echo "    when upgrading. This flag should not be set when installing"
 echo "    Fass for the first time"
 echo "-d: target installation directory, if not defined it'd be root. Must be"
 echo "    an absolute path."
 echo "-r: remove Fass, only useful if -d was not specified, otherwise"
 echo "    rm -rf \$FASS_LOCATION would do the job"
 echo "-l: creates symlinks instead of copying files, useful for development"
 echo "-h: prints this help"
}

PARAMETERS="hkrlu:g:d:"

if [ $(getopt --version | tr -d " ") = "--" ]; then
    TEMP_OPT=`getopt $PARAMETERS "$@"`
else
    TEMP_OPT=`getopt -o $PARAMETERS -n 'install.sh' -- "$@"`
fi

if [ $? != 0 ] ; then
    usage
    exit 1
fi

eval set -- "$TEMP_OPT"

INSTALL_ETC="yes"
UNINSTALL="no"
LINK="no"
FASSADMIN_USER=`id -u fassadmin`
FASSADMIN_GROUP=`id -g fassadmin`
SRC_DIR=$PWD

while true ; do
    case "$1" in
        -h) usage; exit 0;;
        -k) INSTALL_ETC="no"   ; shift ;;
        -r) UNINSTALL="yes"   ; shift ;;
        -l) LINK="yes" ; shift ;;
        -u) FASSADMIN_USER="$2" ; shift 2;;
        -g) FASSADMIN_GROUP="$2"; shift 2;;
        -d) ROOT="$2" ; shift 2 ;;
        --) shift ; break ;;
        *)  usage; exit 1 ;;
    esac
done

# Definition of locations

CONF_LOCATION="$HOME/.fass"

if [ -z "$ROOT" ] ; then
    BIN_LOCATION="/usr/bin"
    #LIB_LOCATION="/usr/lib/fass"
    ETC_LOCATION="/etc/fass"
    LOG_LOCATION="/var/log/fass"
    VAR_LOCATION="/var/lib/fass"
    RUN_LOCATION="/var/run/fass"
    LOCK_LOCATION="/var/lock/fass"
    #INCLUDE_LOCATION="/usr/include"

    MAKE_DIRS="$BIN_LOCATION $ETC_LOCATION $LOG_LOCATION $RUN_LOCATION $LOCK_LOCATION $VAR_LOCATION"
              # $LIB_LOCATION $INCLUDE_LOCATION"

    DELETE_DIRS="$ETC_LOCATION $LOG_LOCATION $RUN_LOCATION"
                # $LIB_LOCATION $VAR_LOCATION" 
    CHOWN_DIRS="$ETC_LOCATION $LOG_LOCATION $RUN_LOCATION $LOCK_LOCATION"
               # $VAR_LOCATION 

    # systemd
    #cp systemd/fass.service /usr/lib/systemd/system/fass.service
    #cp systemd/fass.logrotate /etc/logrotate.d/fass
else
    BIN_LOCATION="$ROOT/bin"
    #LIB_LOCATION="$ROOT/lib"
    ETC_LOCATION="$ROOT/etc"
    VAR_LOCATION="$ROOT/var"
    #INCLUDE_LOCATION="$ROOT/include"

    MAKE_DIRS="$BIN_LOCATION $ETC_LOCATION $VAR_LOCATION" 
               # $LIB_LOCATION $INCLUDE_LOCATION"

    DELETE_DIRS="$MAKE_DIRS"

    CHOWN_DIRS="$ROOT"

fi

# File definition, what is going to be installed and where

INSTALL_FILES=(
    BIN_FILES:$BIN_LOCATION
)

INSTALL_ETC_FILES=( ETC_FILES:$ETC_LOCATION)

# Binary files, to be installed under $BIN_LOCATION

BIN_FILES="src/fass/fassd" 

# Configuration files for Fass, to be installed under $ETC_LOCATION

ETC_FILES="etc/fassd.conf etc/shares.conf"


# --- MAIN --------------------------------------------------------

# Create Fass directories
 
echo "Creating dirs..."
if [ "$UNINSTALL" = "no" ] ; then
    for d in $MAKE_DIRS; do
        mkdir -p $DESTDIR$d
    done
fi

# Install/uninstall files

do_file() {
    if [ "$UNINSTALL" = "yes" ]; then
        rm $DESTDIR$2/`basename $1`
    else
        if [ "$LINK" = "yes" ]; then
            ln -s $SRC_DIR/$1 $DESTDIR$2
        else
            cp -RL $SRC_DIR/$1 $DESTDIR$2
        fi
    fi
}


INSTALL_SET="${INSTALL_FILES[@]}"
 
echo "Installing files..."
for i in ${INSTALL_SET[@]}; do
    SRC=$`echo $i | cut -d: -f1`
    DST=`echo $i | cut -d: -f2`

    eval SRC_FILES=$SRC

    for f in $SRC_FILES; do
        do_file $f $DST
    done
done

if [ "$INSTALL_ETC" = "yes" ] ; then
   echo "Installing etc files..."
   INSTALL_ETC_SET="${INSTALL_ETC_FILES[@]}"

    for i in ${INSTALL_ETC_SET[@]}; do
        SRC=$`echo $i | cut -d: -f1`
        DST=`echo $i | cut -d: -f2`

        eval SRC_FILES=$SRC

        OLD_LINK=$LINK
        LINK="no"

        for f in $SRC_FILES; do
            do_file $f $DST
        done

        LINK=$OLD_LINK
   done
fi

# Set ownership or remove Fass directories

# config file should not be readable outside the group 
# (it contains the ONE password)
chmod 640 $ETC_LOCATION/fassd.conf

if [ "$UNINSTALL" = "no" ] ; then
    echo "Setting dir ownership..."
    for d in $CHOWN_DIRS; do
        chown -R $FASSADMIN_USER:$FASSADMIN_GROUP $DESTDIR$d
    done
else
    echo "Removing dirs..."
    for d in `echo $DELETE_DIRS | awk '{for (i=NF;i>=1;i--) printf $i" "}'`; do
        rmdir $d
    done
fi
