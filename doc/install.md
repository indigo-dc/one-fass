# Install
* [Centos 7 instructions](install-centos.md)

## Ports open needed for FaSS
FaSS needs OpenNebula port open, the default is 2633, plus other two ports:
* port 2637 for the Remote Procedure Call to ONE
* port 8086 for Influx Database

If you wish to change the default ports, edit FaSS config file: ```/tmp/one-fass/etc/fassd.conf```, and don't forget to update the FaSS endpoint in ```/etc/one/sched.conf``` as explained in the specific [installation istructions](install-centos.md).
