# Centos 7
## Prerequisites
### OpenNebula
OpenNebula must be insalled, following ONE documentation [OpenNebula documentation](http://docs.opennebula.org/5.0/deployment/opennebula_installation/index.html).

### Required packages
Additonal libraries and packages are automatically installed with the ```rpm``` file.

## Installation of FaSS 

### Manually
You can install FaSS as follows (as root user):
```bash
$ cd /tmp/
$ git clone https://github.com/indigo-dc/one-fass
$ cd one-fass
$ rpm -Uvh one-fass-service-systemd_devel-0.x86_64.rpm
```
FaSS will be installed. 

Then you should adjust the configuration file of the OpenNebula scheduler, to allow it to point at the FaSS endpoint instead, in ```/etc/one/sched.conf``` change:

```ONE_XMLRPC = "http://localhost:2633/RPC2"``` -> ```ONE_XMLRPC = "http://localhost:2637/RPC2"```.

Now the ONE scheduler will send the RP Calls to FaSS.

## Logs
The log file for FaSS can be found in `/var/log/fass/fass.log`. 
The log of the OpenNebula daemon is located in `/var/log/one/oned.log`.
