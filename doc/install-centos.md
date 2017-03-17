# Centos 7
## Prerequisites
### OpenNebula
OpenNebula must be insalled, following ONE documentation [OpenNebula documentation](http://docs.opennebula.org/5.0/deployment/opennebula_installation/index.html).

### Required packages
TODO

## Installation of FaSS 

### Manually
Once OpenNebula and the required packages have been installed, you can install FaSS as follows (as root user):

```bash
$ cd /tmp/
$ git clone https://github.com/indigo-dc/one-fass
$ cd one-fass
$ ./install.sh
```

FaSS will be installed. 
Then you should adjust the OpenNebula configuration file ```/etc/one/oned.conf``` adding FaSS endpoint instead of the OpenNebula one:
```PORT = 2633``` -> ```PORT = 2637```

## Logs
The log file for FaSS can be found in `/var/log/fass/fass.log`.
The log of the OpenNebula daemon is located in `/var/log/one/oned.log`.
