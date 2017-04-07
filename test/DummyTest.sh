#!/bin/bash

template=$1
echo Initiating 5 VMs of tty type:
onetemplate instantiate $template --name vm1
onetemplate instantiate $template --name vm2
onetemplate instantiate $template --name vm3
onetemplate instantiate $template --name vm4
onetemplate instantiate $template --name vm5
sleep 1

echo Listing the VMs
onevm list

echo Starting FaSS
echo
rm -rf /var/lock/fass/fass
sleep 1
systemctl start fass
sleep 5

while [[ "$(onevm show "vm5" | grep STATE)" =~ "PENDING" ]] ; do
	if [[ "$(onevm show "vm4" | grep STATE)" =~ "ACTIVE" ]] ; then
		echo FaSS failed to implement Dummy Algorithm. FIFO scheduling. Exiting.
		exit 1
	fi
	echo Waiting for FaSS to start scheduling according to the Dummy Algorithm...
	sleep 10
done

echo Done scheduling VM 5:
onevm list

while [[ "$(onevm show "vm4" | grep STATE)" =~ "PENDING" ]] ; do
        if [[ "$(onevm show "vm3" | grep STATE)" =~ "ACTIVE" ]] ; then
                echo FaSS failed to implement Dummy Algorithm. FIFO scheduling. Exiting.
                exit 1
        fi
	echo Waiting for FaSS to schedule...
        sleep 10
done
 
echo Done scheduling VM 4:
onevm list

while [[ "$(onevm show "vm3" | grep STATE)" =~ "PENDING" ]] ; do
        if [[ "$(onevm show "vm2" | grep STATE)" =~ "ACTIVE" ]] ; then
                echo FaSS failed to implement Dummy Algorithm. FIFO scheduling. Exiting.
                exit 1
        fi
	echo Waiting for FaSS to schedule...
        sleep 10
done

echo Done scheduling VM 3:
onevm list

while [[ "$(onevm show "vm2" | grep STATE)" =~ "PENDING" ]] ; do
        if [[ "$(onevm show "vm1" | grep STATE)" =~ "ACTIVE" ]] ; then
                echo FaSS failed to implement Dummy Algorithm. FIFO scheduling. Exiting.
                exit 1
        fi
	echo Waiting for FaSS to schedule...
        sleep 10
done

echo Done scheduling VM 2:
onevm list

while [[ "$(onevm show "vm2" | grep STATE)" =~ "PENDING" ]] ; do
        echo Waiting for FaSS to schedule the first VM...
        sleep 10
done

echo Done scheduling VM 1:
onevm list

echo FaSS Dummy Algorithm successfully tested

echo Cleaning...
onevm recover --delete "vm5"
onevm recover --delete "vm4"
onevm recover --delete "vm3"
onevm recover --delete "vm2"
onevm recover --delete "vm1"

echo Stopping FaSS
systemctl stop fass
