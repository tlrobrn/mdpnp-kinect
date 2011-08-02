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
#ifndef MONITOR_H
#define MONITOR_H
//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------
#include <XnCppWrapper.h>

using namespace xn;

//-------------------------------------------------------------------------------
//	Enums & Structs
//-------------------------------------------------------------------------------
enum Position {LAYING, TURNED, FORWARD, UNKNOWN};

//-------------------------------------------------------------------------------
//	Macros
//-------------------------------------------------------------------------------
#define CONTEXT_XML			"../config/config.xml"
#define CALIBRATION_FILE	"../config/calibration.bin"
#define LAYING_TOLERANCE	200
#define TURNED_TOLERANCE	150

#define CHECK_RC(status, what)										\
	if(status != XN_STATUS_OK) {									\
		printf("%s failed: %s\n", what, xnGetStatusString(status));	\
		return;												\
	}

//-------------------------------------------------------------------------------
//  Class Definition
//-------------------------------------------------------------------------------
class KinectMonitor {
    private:
        // Member variables
        Position        previous, current;
    
    public:
        // Constructors
        KinectMonitor();
        // Destructors
        ~KinectMonitor();
        
        // Get data
        void run();
        Position getPosition(XnUserID user);
};
//-------------------------------------------------------------------------------
#endif
