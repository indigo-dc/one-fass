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
The ```shares.conf``` file, therefore, contains the basic working setup for only the administrative accounts
```bash
[users]
user=0:0:50
user=1:0:50
```
to which you need to add all the other users and the relative shares in this format: ```userID:groupID:share%```.

## Start FaSS
To start FaSS, simply type 
```bash
systemctl start fass
```

## Using FaSS
Version 1.1 of FaSS does not implement an algorithm by default, but contains all the setup to reset the priorities of the VM of your users. 
To add an algorithm edit the ```BasicPlugin``` class, in ```/tmp/one-fass/src/pm```.
