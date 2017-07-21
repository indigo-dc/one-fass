# Using FaSS

## Edit initial shares
After having installed FaSS following this [guidelines](https://github.com/indigo-dc/one-fass/blob/indigo-test/doc/install.md), you need to set the initial shares for every user.

```bash
$ cd /tmp/one-fass/etc
```
and edit the file ```shares.conf```.

Check your OpenNebula users:
```bash 
oneuser list
```
and the groups
```bash 
onegroup list
```

By default, after the installation of Opennebula, you will have two administrative accounts, ```oneadmin``` (userID=0) and ```serveradmin``` (userID=1). And two groups, ```oneadmin``` (groupID=0) and ```users``` (groupID=1).
The ```shares.conf``` file, therefore, contains the basic working setup for only the administrative accounts and an example of how to add a user called ```goofy```
```bash
[oneadmin]
uid=0
gid=0
share=50
[serveradmin]
uid=1
gid=0
share=0
[goofy]
uid=2
gid=1
share=50
```
you need to change the users accordingly to your list.

## Start FaSS
To start FaSS, simply type 
```bash
systemctl start fass
```

## Using FaSS
Starting fro FaSS v1.1, the scheduler implements a simple version of the SLURM MultiFactor algorithm, setting the parameter ```plugin_debug``` to 1 in the FaSS configuration file.
In order to implement your own fair-share algorithm, edit the ```BasicPlugin``` class, in ```/tmp/one-fass/src/pm```.
A dummy algorithm, which inverts the priorities recevied from OpenNebula, is also available setting ```plugin_debug``` to 0.  

## Set Virtual Machines to be static
By default the VMs are terminated after they have exceeded their time to live ```ttl``` or maximum waiting time ```max_wait```, both set in the FaSS configuration file ```/one-fass/etc/fassd.conf```. Anyway, it is possible to instantiate a VM as static. This means that it will not be terminated. To do so, add a raw attribute to the onevm instantiate command:
```bash
$ onetemplate instantiate <yourtemplateid> --raw static_vm=1
```
The defalt value for ```static_vm``` is ```0```, i.e. the VM is dynamic. 

