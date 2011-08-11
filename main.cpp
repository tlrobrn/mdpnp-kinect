/********************************************************************************
*	MDPnP Patient Kinect Monitor												*
* 	Taylor O'Brien, University of Illinois										*
*																				*
* 	This software uses OpenNI components and modules to monitor hospital		*
* 	patients in bed.  It will send a notification when	a patient appears to	*
* 	be in the process of getting out of bed.									*
* 																				*
* 	This code was created using examples and modified sample code from OpenNI.	*
* 	Used by permission under the GNU Lesser General Public License.				*
* 																				*
* 	Last Modified 2 August 2011 by Taylor O'Brien								*
********************************************************************************/
//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------
#include "includes/monitor.h"

//-------------------------------------------------------------------------------
//	Main
//-------------------------------------------------------------------------------
int main(int argc, char **argv) {
	// Get the desired tilt if there is one
	int *tilt;
	if( argc == 1 )
		tilt = NULL;
	else
		tilt = new int( atoi(argv[1]) );
	
	// Initialize the KinectMonitor
	KinectMonitor *monitor = new KinectMonitor(tilt);
    
  // Run the Kinect Monitor
  monitor->run();
    
  // Free up memory
  delete monitor;
  if(tilt != NULL) delete tilt;
    
  return 0;
}
