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

// Location of the file for loading the Context
#define CONTEXT_XML			"config/config.xml"
// Location of the file for loading skeletal calibrations
#define CALIBRATION_FILE	"config/calibration.bin"
// The default tilt value (good for approximately 6ft off the floor)
#define DEFAULT_TILT			-30
// The difference between the head and torso Z coordinate that is allowed before
// the patient would be considered as "laying down"
#define LAYING_TOLERANCE	100
// The difference between the left and right shoulder joint Z coordinates before
// the patient would be considered turning (enough to get out of bed)
#define TURNED_TOLERANCE	150
// The distance the patient can move in the X direction before he is considered
// to be out of the hospital bed
#define BED_TOLERANCE		400

//-------------------------------------------------------------------------------
//  Class Definition
//-------------------------------------------------------------------------------
class KinectMonitor {
    private:
        // Member variables
        Position	previous, current;
        double		bed;
        bool		out, bedSet;
    
    public:
        // Constructors
        KinectMonitor(int *tilt);
        KinectMonitor(){KinectMonitor(NULL);}
        
        // Destructors
        ~KinectMonitor();
        
        // Get data
        void run();
        Position getPosition(XnUserID user);
};
//-------------------------------------------------------------------------------
#endif
