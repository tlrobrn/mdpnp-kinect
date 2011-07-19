# MDPnP Kinect Monitoring #
---------------------------

Taylor O'Brien
University of Illinois

---------------------------
## Dependencies ##

+ mono-complete
+ libusb-1.0-0-dev
+ freeglut3-dev

## Libraries ##

+ OpenNI [http://www.openni.org](http://www.openni.org)
+ NITE (OpenNI Middleware)
+ Sensor   (OpenNI Hardware)
+ SensorKinect [https://github.com/avin2/SensorKinect](https://github.com/avin2/SensorKinect)

## Setup Instructions ##

1. Create a base directory for the libraries e.g. kinect

    $ mkdir kinect

2. Download the latest unstable binaries from OpenNI, NITE (OpenNI Middleware), and Sensor (OpenNI Hardware)

[OpenNI Module](http://www.openni.org/downloadfiles/opennimodules/openni-binaries/latest-unstable/160-openni-unstable-build-for-ubuntu-10-10-x86-32-bit-v1-3-2/download)

[NITE OpenNI Middleware Module](http://www.openni.org/downloadfiles/opennimodules/openni-compliant-middleware-binaries/latest-unstable/174-primesense-nite-unstable-build-for-ubuntu-10-10-x86-32-bit-v1-4-1/download)

[Sensor OpenNI Hardware Module](http://www.openni.org/downloadfiles/opennimodules/openni-compliant-hardware-binaries/latest-unstable/167-primesensor-module-unstable-build-for-ubuntu-10-10-x86-32-bit-v5-0-3/download)

3. Unpack the archives into OpenNI, Nite, and Sensor folders within the base kinect directory

4. Create the directory /var/lib/ni

    $ sudo mkdir /var/lib/ni

4. 

PrimeSense Key: 0KOIk2JeIBYClPWVnMoRKn5cdY4=

