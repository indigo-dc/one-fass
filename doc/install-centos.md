# Centos 7
## Prerequisites
### OpenNebula
OpenNebula must be insalled, following [OpenNebula documentation](http://docs.opennebula.org/5.2/deployment/opennebula_installation/index.html).
After following the front-end installation procedure, move to the node installation to add hypervisors to your cloud. 
Note that FaSS v1.2 and higher run with ONE 5.4 and higher. If you are running an older ONE version, checkout FaSS v1.1 tag with ONE patch.

### Required packages
Additonal libraries and packages are automatically installed with the rpm file:
* scons 
* boost
* boost-devel 
* xmlrpc-c-c++
* xmlrpc-c-client++ 
* xmlrpc-c-devel 
* shadow-utils
* coreutils
* glibc-common

### InfluDB installation
InfluxDB must be dowloaded and installed separately, following [InfluxDB documentation](https://docs.influxdata.com/influxdb/v1.3/introduction/). Once installed follow the [getting started section](https://docs.influxdata.com/influxdb/v1.3/introduction/getting_started/) to create and use ```fassdb```.

## Installation of FaSS 
### Manually
You can install FaSS using ```git``` as follows (as root user):
```bash
$ cd /tmp/
$ git clone https://github.com/indigo-dc/one-fass
$ cd one-fass
$ cd rpms
$ yum localinstall one-fass-indigo-service-test-1.1.x86_64.rpm
```
or
```
$ scons 
$ ./install -u fassadmin -g fassadmin
```
Check ```./install --help``` for additional options, e.g. ```-k``` to keep the configuration files.

## Configuration of FaSS

Then you should adjust the configuration file of the OpenNebula scheduler, to allow it to point at the FaSS endpoint instead, in ```/etc/one/sched.conf``` change:

```ONE_XMLRPC = "http://localhost:2633/RPC2"``` to ```ONE_XMLRPC = "http://localhost:2637/RPC2"```.

Now the ONE scheduler will send the RP Calls to FaSS.

## Logs
The log file for FaSS can be found in `/var/log/fass/fass.log`. 
The log of the OpenNebula daemon is located in `/var/log/one/oned.log`.
