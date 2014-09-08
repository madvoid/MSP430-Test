# MSP430 Test #
---
## Introduction ##
This repository is a testbed for code that is to be used with Texas Instruments' [MSP430 Launchpad](http://www.ti.com/tool/msp-exp430g2). Some of the projects use GCC and some of the projects are CCS (Eclipse) projects. 

## Installation and Setup##
Since I develop on a Mac, here's what I did to get everything working properly. Thanks to the [Media Computing Group](http://hci.rwth-aachen.de/msp430) for the excellent tutorial. Note that as of August 17th, 2014, the upload instructions do not work on the FRAM Launchpad with EnergyTrace tech.

1. Install Xcode and Xcode command line tools
2. Install Macports
3. Using Macports, install ```msp430-gcc```, ```msp430-libc```, ```mspdebug```.
4. ```mspdebug``` does not play nice with the OSX HID drivers. As a result, one needs to install a kernel extension (kext). Follow the following steps to do so under OS X Mavericks and Xcode 5.1.
	1. Download the kext from [Colossal Dynamic's Github](https://github.com/colossaldynamics/ez430rf2500).
	2. Open the project in Xcode
	3. Double click the little yellow warning triangle in Xcode so that the recommended changes page pops up. Allow Xcode to make the recommended changes. This includes changing the compiler to the most recent version.
	4. Build the project.
	5. In the Xcode sidebar, in the ```projects``` folder, there should be a file called ```ez430rf2500.kext```. Right click, show in finder, and drag the .kext file to your desktop.
	6. Open the terminal and run the following commands. You will need to use ```sudo``` for most commands. After running the commands, reboot your computer. ```mspdebug``` should now be ready to use.

```
cd /System/Library/Extensions
cp -R ~/Desktop/ez430rf2500.kext .
chown -R root:wheel ez430rf2500.kext
chmod -R 755 ez430rf2500.kext
```

### Alternate Version ###
1. Follow steps 1-3 from above
2. Follow the link to the google code page from PentiumPC's post [here](http://forum.43oh.com/topic/1161-launchpad-osx-usb-drivers-cdc-vcp/) and download the installer.
3. Run installer. Note that if you follow these instructions, you should not do step 4 from above.

## Compiling & Uploading ##
All code can be compiled by running ```make``` in the working directory. The makefile has been designed so that all files inside the working directory will be automatically compiled. However, any ```*.c``` files that live in external folders will need to be added to the ```EXTERN_FILES``` line, all separated by spaces. To summarize, the following steps will need to be completed to modify the makefile for other projects:

1. Change the filename on line 28.
2. Change the device name on line 29.
3. Add any external ```*.c``` files to line 30, separated by spaces.

Programs can be uploaded by running ```make upload```. It should not be necessary to compile before running ```make upload```. Cleaning can be done with ```make clean```.

CCS projects are compiled and uploaded within CCS, and CCS should handle everything.

# Project Descriptions #
* **Blink** - Blinks the LEDs on and off
* **Button** - Press a button to wake from sleep and blink light
* **CCS** - Contains projects from CCS. So far, all projects are configured for the MSPFR5969
	* **Blink** - Blinks light on and off
	* **BMP180_Cals** - Gets calibration values from BMP180. *Currently Not Working!*
	* **LPM4** - Enters LPM4 and toggles LED with button
	* **SHT21** - Has ShtLib, functions for use with SHT21
	* **SHT21_Hum** - Gathers humidity from SHT21 sensor
	* **SHT21_Temp** - Gathers temperature from SHT21 sensor
	* **TimerB_OneShot** - Counts down timer once