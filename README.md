# MDPnP Kinect Monitoring #

Taylor O'Brien
University of Illinois

## System ##

Ubuntu 11.04 Linux-x86 (32-bit)

## Dependencies ##

+ mono-complete
+ libusb-1.0-0-dev

## Libraries ##

+ OpenNI [http://www.openni.org](http://www.openni.org)
+ NITE (OpenNI Middleware)
+ Sensor   (OpenNI Hardware)
+ SensorKinect [https://github.com/avin2/SensorKinect](https://github.com/avin2/SensorKinect)

## Setup Instructions ##

These instructions are for Ubuntu, but they should work verbatim for any Linux platform that uses apt (untested).

1. Create a base directory for the libraries e.g. kinect

        $ mkdir kinect

2. Download the latest unstable binaries from OpenNI, NITE (OpenNI Middleware), Sensor (OpenNI Hardware), and SensorKinect

    [OpenNI Module](http://www.openni.org/downloadfiles/opennimodules/openni-binaries/latest-unstable/160-openni-unstable-build-for-ubuntu-10-10-x86-32-bit-v1-3-2/download)

    [NITE OpenNI Middleware Module](http://www.openni.org/downloadfiles/opennimodules/openni-compliant-middleware-binaries/latest-unstable/174-primesense-nite-unstable-build-for-ubuntu-10-10-x86-32-bit-v1-4-1/download)

    [Sensor OpenNI Hardware Module](http://www.openni.org/downloadfiles/opennimodules/openni-compliant-hardware-binaries/latest-unstable/167-primesensor-module-unstable-build-for-ubuntu-10-10-x86-32-bit-v5-0-3/download)

    [SensorKinect](https://github.com/avin2/SensorKinect/tarball/unstable)

3. Unpack the archives into OpenNI, Nite, Sensor, and SensorKinect folders within the base kinect directory

4. Update and install dependencies

        $ sudo apt-get update
        $ sudo apt-get install mono-complete
        $ sudo apt-get install libusb-1.0-0-dev

5. Create the directory /var/lib/ni

        $ sudo mkdir /var/lib/ni

6. In the OpenNI, Sensor, and Nite directories, run the install script

        $ cd OpenNI
        $ sudo ./install.sh

        $ cd ../Sensor
        $ sudo ./install.sh

        $ cd ../Nite
        $ sudo ./install.sh

    During the NITE install use the PrimeSense Key: 0KOIk2JeIBYClPWVnMoRKn5cdY4=

7. Install SensorKinect

        $ cd ../SensorKinect/Platform/Linux-x86/CreateRedist/
        $ sudo ./RedistMaker
        $ cd ../Redist
        $ sudo ./install.sh

## Build and Run ##

To build the project, go to the base directory of the MDPnP-KinectMonitor (it will contain the Makefile) and run make.

	$ make

This will compile the project.  There should now be an executable called MDPnP-KinectMonitor in the base directory (as well as object files located in the bin/ directory).

To run the project, it must be run with sudo priviledges to adjust the camera.  There is also an optional angle argument (just an int) that specifies the tilt of the camera.

	$ sudo ./MDPnP-KinectMonitor <angle>

You can also run it without an angle and it will be set to the default tilt.

	$ sudo ./MDPnP-KinectMonitor
