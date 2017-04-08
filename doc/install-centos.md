# Centos 7
## Prerequisites
### OpenNebula
OpenNebula must be insalled, following ONE documentation [OpenNebula documentation](http://docs.opennebula.org/5.2/deployment/opennebula_installation/index.html).
After following the front-end installation procedure, move to the node installation to add hypervisors to your cloud. 

### Required packages
Additonal libraries and packages are automatically installed with the ```rpm``` file.

## Installation of FaSS 
### Manually
You can install FaSS using ```git``` as follows (as root user):
```bash
$ cd /tmp/
$ git clone https://github.com/indigo-dc/one-fass
$ cd one-fass
$ cd rpms
$ yum localinstall one-fass-indigo-service-test-1.0.x86_64.rpm
```
FaSS will be installed. 

## Configuration of FaSS

Then you should adjust the configuration file of the OpenNebula scheduler, to allow it to point at the FaSS endpoint instead, in ```/etc/one/sched.conf``` change:

```ONE_XMLRPC = "http://localhost:2633/RPC2"``` to ```ONE_XMLRPC = "http://localhost:2637/RPC2"```.

Now the ONE scheduler will send the RP Calls to FaSS.

Currently, it is needed to have a patch of the OpenNebula scheduler code. The changes are already implemented and it is sufficient to change in ```/usr/lib/systemd/system/opennebula-scheduler.service```:

```ExecStart=/usr/bin/mm_sched``` to ```ExecStart=/usr/bin/mm_sched_fass```

after this, you need to restart OpenNebula:

```bash
$ systemctl daemon-reload
$ systemctl restart opennebula
```

## Logs
The log file for FaSS can be found in `/var/log/fass/fass.log`. 
The log of the OpenNebula daemon is located in `/var/log/one/oned.log`.
