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
#include <stdint.h>
#include <XnOpenNI.h>
#include <XnTypes.h>
#include <XnOS.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include <XnFPSCalculator.h>
#include <XnUSB.h>
#include <signal.h>
#include "../includes/monitor.h"

//-------------------------------------------------------------------------------
//	Macros
//-------------------------------------------------------------------------------
#define VID_MICROSOFT	0x45e
#define PID_NUI_MOTOR	0x02b0
#define CHECK_RC(status, what)										\
	if(status != XN_STATUS_OK) {									\
		printf("%s failed: %s\n", what, xnGetStatusString(status));	\
		return;												\
	}

// Use OpenNI's xn namespace
using namespace xn;

//-------------------------------------------------------------------------------
//	Globals
//-------------------------------------------------------------------------------
    Context         context;
    ScriptNode      scriptNode;
    DepthGenerator  depthGenerator;
    UserGenerator   userGenerator;
    XnFPSData				xnFPS;
    bool            quit;

//-------------------------------------------------------------------------------
//	Callbacks/Handlers/Auxiliary
//-------------------------------------------------------------------------------

/*
 *	Function:	loadCalibration
 *
 *	Loads a skeletal calibration from a standard file.
 *	This means that a new user does not need to hold a pose to be tracked.
 *	The CALIBRATION_FILE macro is defined in monitor.h.
 *
 *	Parameters:
 *		XnUserID user	-	The reference to the new user to be calibrated.
 */
void loadCalibration(XnUserID user) {
    if( userGenerator.GetSkeletonCap().IsCalibrated(user) ) return;
    
    // Load file
    XnStatus status = userGenerator.GetSkeletonCap().
        LoadCalibrationDataFromFile(user, CALIBRATION_FILE);
    
    // Start tracking
    if( status == XN_STATUS_OK )
        userGenerator.GetSkeletonCap().StartTracking(user);

}

// Callbacks

/*
 *	Function:	foundUser
 *
 *	Loads a calibration for a new user whenever a new user enters the scene.
 *	This function is called automatically when the Kinect recognizes a new human.
 *
 *	Parameters:
 *		UserGenerator&	generator	-	A reference to the UserGenerator module. Not Used.
 *		XNUserID				nID				-	The new unique id for the recognized user.
 *		void*						pCookie
 */
void XN_CALLBACK_TYPE foundUser(UserGenerator &generator,
    XnUserID nID, void *pCookie) {
    // Load up skeleton calibration for the recognized user.
    loadCalibration(nID);
}

/*
 *	Function:	lostUser
 *
 *	Gets the number of users remaining (including the lost user).
 *	If this was the last user, an alert is fired for "Patient not tracked."
 *
 *	Parameters:
 *		UserGenerator&	generator	-	A reference to the UserGenerator module. Not Used.
 *		XNUserID				nID				-	The id referring to the lost user.
 *		void*						pCookie
 */
void XN_CALLBACK_TYPE lostUser(UserGenerator &generator,
    XnUserID nID, void *pCookie) {
  
	// Get the available users (allowing enough space for up to 15 different users)
  XnUInt16 numUsers = 15;
	XnUserID users[numUsers];
	userGenerator.GetUsers(users, numUsers);
	// Raise an alert if there are no users being tracked (meaning we lost the patient).
	if( numUsers <= 1) {
		printf("Patient not tracked.\n");
	}
}

// Signal Handler

/*
 *	Function:	stop
 *
 *	Ends the line on standard out, and sets the global "quit" boolean to true.
 *	The "quit" boolean will cause the loop in KinectMonitor::run() to exit,
 *	and the program to exit.
 *
 *	Parameters:
 *		int	signal	-	The signal caught by the system.
 */
void stop(int signal) {
	printf("\n");
  quit = true;
}

//-------------------------------------------------------------------------------
//	Class Methods
//-------------------------------------------------------------------------------

// Constructors

/*
 *	Function:	KinectMonitor	(Constructor)
 *
 *	Initializes all the production nodes for the kinect to get and process data.
 *	Sets the camera tilt to the specified (or default) angle.
 *	Registers the OpenNI callbacks for user events.
 *	Registers the system signal handlers (for terminating the program).
 *	Initializes the global and member variables.
 *
 *	Parameters:
 *		int*	tilt	-	A pointer to the desired tilt angle of the camera.
 *									If it is NULL, the default value is used.
 */
KinectMonitor::KinectMonitor(int *tilt) {
    XnStatus status;
    EnumerationErrors errors;
    XnCallbackHandle userCallbacks;
    
    // Setup Context from an XML configuration file (the default supplied by OpenNI)
		// CONTEXT_XML is defined in monitor.h
    status = context.InitFromXmlFile(CONTEXT_XML, scriptNode, &errors);
    // Check to ensure that the context was initialized properly.
		if( status == XN_STATUS_NO_NODE_PRESENT ) {
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        printf("%s\n", strError);
        
        return;
    } else if( status != XN_STATUS_OK ) {
        printf("Could not initialize Context: %s\n", xnGetStatusString(status));
        
        return;
    }
    
    // Setup Depth Generator production node from the context
    status = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depthGenerator);
	
	// Setup User Generator production node from the context
	status = context.FindExistingNode(XN_NODE_TYPE_USER, userGenerator);
	// Check that the user generator is available
	if( status != XN_STATUS_OK ) {
		// If the context did not define a UserGenerator node, then try to create one
		status = userGenerator.Create(context);
		CHECK_RC(status, "Find user generator");
	}
	
	// Set FPS
	status = xnFPSInit(&xnFPS, 180);
	
	// Check for Skeletal Mapping
	if( !userGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON) ) {
		printf("Skeletal mapping not supported.\n");
		
		return;
	}
	// Set the skeletal profile to only include the joints in the upper body.
	// Profile options are XN_SKEL_PROFILE_<option>
	// Where <option> could be: NONE, ALL, UPPER, LOWER, or HEAD_HANDS
	userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_UPPER);
	
	// Tilt camera - This feature requires the program to be run with root privilege,
	// because it requires writing to the usb device.
	XN_USB_DEV_HANDLE dev;
	// Sets the angle to either the DEFAULT_TILT (defined in monitor.h) or the given tilt.
	int angle = (tilt == NULL)? DEFAULT_TILT : *tilt;
	// Open the kinect usb device
	status = xnUSBOpenDevice(VID_MICROSOFT, PID_NUI_MOTOR, NULL, NULL, &dev);
	// Send the proper code to the usb device to set the angle.
	uint8_t empty[0x1];
	status = xnUSBSendControl(
		dev, XN_USB_CONTROL_TYPE_VENDOR, 0x31, (XnUInt16)angle,
		0x0, empty, 0x0, 0
	);
	
	// Register Callbacks
	status = userGenerator.RegisterUserCallbacks(
	    foundUser, lostUser, NULL, userCallbacks
	);
	
	// Register Handlers
	signal(SIGABRT, &stop);
	signal(SIGTERM, &stop);
	signal(SIGINT, &stop);
	// Initialize globals
	quit = false;
	out = true;
}
// Destructors

/*
 *	Function:	~KinectMonitor	(Destructor)
 *
 *	Releases all the production nodes and the context.
 */
KinectMonitor::~KinectMonitor() {
    scriptNode.Release();
    depthGenerator.Release();
    userGenerator.Release();
    context.Release();
}
        
// Main Loop

/*
 *	Function:	run
 *
 *	Starts and continues generating data from the Kinect.
 *	A loop runs and updates data whenever new data is available from one of the Kinect
 *	devices, and then the data is processed to check for patient movement.
 *	The loop is controlled by the "quit" global boolean, which is set to false by the 
 *	signal handler "stop()"
 */
void KinectMonitor::run() {
  XnStatus status;
  SceneMetaData scene;
	DepthMetaData depth;
    
  // Start the device
  status = context.StartGeneratingAll();

  // Running loop
  while( !quit ) {
    // Wait for any new incoming data
    context.WaitOneUpdateAll(depthGenerator);
    // Mark the new frame
		xnFPSMarkFrame(&xnFPS);
    // Get the depth data from the device
		depthGenerator.GetMetaData(depth);
        
    // Get the recognized users
		XnUInt16 numUsers = 15;
		XnUserID users[numUsers];
		userGenerator.GetUsers(users, numUsers);
		// Only track the patient if they are alone
		if( numUsers != 1) continue;
        
		// Get the user data
		userGenerator.GetUserPixels(users[0], scene);
        
    // Update patient position
    previous = current;
    current = getPosition(users[0]);
        
    // Raise alerts based on the patient's state and position
		if( previous != current ) {
        if( current == TURNED && out == false ) {
					// Patient is turned
					printf("Patient getting out of bed.\n");
				} else if( out && bedSet ) {
					printf("Patient is out of bed.\n");
				}
		}
  }
}

/*
 *	Function:	getPosition
 *
 *	Returns the Position (enum defined in monitor.h) of the specified user.
 *	Positions are assigned based on joint locations and defined tolerances.
 *	Sets the location of the hospital bed if it has not yet been set and the patient
 *	is lying down.
 *
 *	Parameters:
 *		XnUserID	user	-	The id for the user whose position to get
 *
 *	Return:
 *		The Position of the user
 */
Position KinectMonitor::getPosition(XnUserID user) {
	XnSkeletonJointPosition headPos, torsoPos, leftPos, rightPos;
	double head, torso, center, left, right;
	Position position = UNKNOWN;
	SkeletonCapability skeleton = userGenerator.GetSkeletonCap();
	
	// Get Joint positions for the specified joint locations
	// Joints are XN_SKEL_<option> where <option> is:
	// HEAD, NECK, TORSO, WAIST, LEFT_COLLAR, LEFT_SHOULDER, LEFT_ELBOW, LEFT_WRIST, LEFT_HAND
	// LEFT_FINGERTIP, RIGHT_COLLAR, RIGHT_SHOULDER, RIGHT_ELBOW, RIGHT_WRIST, RIGHT_HAND,
	// RIGHT_FINGERTIP, LEFT_HIP, LEFT_KNEE, LEFT_ANKLE, LEFT_FOOT, RIGHT_HIP, RIGHT_KNEE,
	// RIGHT_ANKLE, RIGHT_FOOT
	skeleton.GetSkeletonJointPosition(user, XN_SKEL_HEAD, headPos);
	skeleton.GetSkeletonJointPosition(user, XN_SKEL_TORSO, torsoPos);
	skeleton.GetSkeletonJointPosition(user, XN_SKEL_LEFT_SHOULDER, leftPos);
	skeleton.GetSkeletonJointPosition(user, XN_SKEL_RIGHT_SHOULDER, rightPos);

	// Get the relevant coordinate for each joint (X, Y, or Z)
	head	= headPos.position.Z;
	torso	= torsoPos.position.Z;
	center	= torsoPos.position.X;
	left	= leftPos.position.Z;
	right	= rightPos.position.Z;
	
	// Determine defined positions based on tolerances defined in monitor.h
	if(head - LAYING_TOLERANCE > torso) {
		position = LAYING;
		out = false;
		
	} else if(	left < right - TURNED_TOLERANCE || 
				left > right + TURNED_TOLERANCE	) {
		position = TURNED;
		
	} else {
		position = FORWARD;
	}
	
	// Set/Check the patient vs the bed location
	if( (!bedSet) && position == LAYING ) {
		// If the bed location is not yet set, then set the bed at the patients torso X coordinate
		bed = center;
		bedSet = true;
		
	} else if( bedSet ) {
		// The patient is out of bed if they are outside the BED_TOLERANCE defined in monitor.h
		out = ( center < bed - BED_TOLERANCE || center > bed + BED_TOLERANCE );
	}
	
	return position;
}
