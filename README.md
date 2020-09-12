# RelayDESFireEV1

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/IediWHW2NhA/0.jpg)](https://www.youtube.com/watch?v=IediWHW2NhA)

This project is the result of a seminal work. 
It can perform basic authentification relay attack if DESFire EV1 is used.


You need:
* Proxmark3 (i was using proxmark3 easy (40 €))
* Some Arduino board (i was using WeMos D1 mini) & PN532.
* Windows computer to run a simple console application.


For updating code on proxmark, I was using Gator96100 ProxSpace.

https://github.com/Gator96100/ProxSpace/releases

(ProxSpace v3.5 ... ProxSpace-64.7z)

To install C read README.md file.

When ProxSpece environment is set. Copy content of the proxmark3.zip to <ProxSpace environment dir>/pm3
Inside proxmark3.zip file is Iceman Proxmark3 v4.9237 (https://github.com/RfidResearchGroup/proxmark3/releases/tag/v4.9237) + some changes.



To compile and install code on proxmark you need to know 2 commands:


```
make clean && make all 
./pm3-flash-fullimage COM16 
```
(or ```pm3-flash-all <COM PORT>``` first time to upload bootrom)


To run proxmark console:
```
./client/proxmark3.exe COM16
```


What is new:
* ```hf 14a simx``` This cmd is used to simulate DESFire EV1 card.

* ```hf 14a sniffx``` it shows you live packets.

Code was mostly added in to those two files:
* pm3/proxmark3/armsrc/iso14443a.c
* pm3/proxmark3/client/src/cmdhf14a.c

In console application, you will need to change COM ports & path to proxmark3.exe.

In Arduino code you will need to change DESFire application address.

Sorry for ugly code.
