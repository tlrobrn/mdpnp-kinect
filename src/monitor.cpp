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
#include <XnOpenNI.h>
#include <XnTypes.h>
#include <XnOS.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include <XnFPSCalculator.h>
#include <signal.h>
#include "../includes/monitor.h"

// Use OpenNI's xn namespace
using namespace xn;

//-------------------------------------------------------------------------------
//	Globals
//-------------------------------------------------------------------------------
    Context         context;
    ScriptNode      scriptNode;
    DepthGenerator  depthGenerator;
    UserGenerator   userGenerator;
    XnFPSData		xnFPS;
    bool            quit;

//-------------------------------------------------------------------------------
//	Callbacks/Handlers/Auxiliary
//-------------------------------------------------------------------------------

void loadCalibration(XnUserID user) {
    if( userGenerator.GetSkeletonCap().IsCalibrated(user) ) return;
    
    // Load file
    XnStatus status = userGenerator.GetSkeletonCap().
        LoadCalibrationDataFromFile(user, CALIBRATION_FILE);
    
    // Start tracking
    if( status == XN_STATUS_OK )
        userGenerator.GetSkeletonCap().StartTracking(user);
    
    printf("CALIBRATED\n");
}

// Callbacks
void XN_CALLBACK_TYPE foundUser(UserGenerator &generator,
    XnUserID nID, void *pCookie) {
    
    printf("NEW PERSON\n");
    loadCalibration(nID);
}
void XN_CALLBACK_TYPE lostUser(UserGenerator &generator,
    XnUserID nID, void *pCookie) {
    
    printf("PERSON LEFT\n");
}

// Signal Handler
void stop(int signal) {
	printf("\n");
    quit = true;
}

//-------------------------------------------------------------------------------
//	Class Methods
//-------------------------------------------------------------------------------

// Constructors
KinectMonitor::KinectMonitor() {
    XnStatus status;
    EnumerationErrors errors;
    XnCallbackHandle userCallbacks;
    
    // Setup Context
    status = context.InitFromXmlFile(CONTEXT_XML, scriptNode, &errors);
    if( status == XN_STATUS_NO_NODE_PRESENT ) {
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        printf("%s\n", strError);
        
        return;
    } else if( status != XN_STATUS_OK ) {
        printf("Could not initialize Context: %s\n", xnGetStatusString(status));
        
        return;
    }
    
    // Setup Depth Generator
    status = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depthGenerator);
	//CHECK_RC(status, "Find depth generator");
	
	// Setup User Generator
	status = context.FindExistingNode(XN_NODE_TYPE_USER, userGenerator);
	if( status != XN_STATUS_OK ) {
		status = userGenerator.Create(context);
		CHECK_RC(status, "Find user generator");
	}
	
	// Set FPS
	status = xnFPSInit(&xnFPS, 180);
	//CHECK_RC(status, "FPS Init");
	
	// Setup Skeletal Mapping
	if( !userGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON) ) {
		printf("Skeletal mapping not supported.\n");
		
		return;
	}
	userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_UPPER);
	
	// Register Callbacks
	status = userGenerator.RegisterUserCallbacks(
	    foundUser, lostUser, NULL, userCallbacks
	);
	
	// Register Handlers
	signal(SIGABRT, &stop);
	signal(SIGTERM, &stop);
	signal(SIGINT, &stop);
	quit = false;
}
// Destructors
KinectMonitor::~KinectMonitor() {
    scriptNode.Release();
    depthGenerator.Release();
    userGenerator.Release();
    context.Release();
}
        
// Get data
void KinectMonitor::run() {
    XnStatus status;
    SceneMetaData scene;
	DepthMetaData depth;
    
    // Start the device
    status = context.StartGeneratingAll();
    //CHECK_RC(status, "StartGenerating");
    
    // Running loop
    while( !quit ) {
        // Wait for incoming data
        context.WaitOneUpdateAll(depthGenerator);
        xnFPSMarkFrame(&xnFPS);
        depthGenerator.GetMetaData(depth);
        
        XnUInt16 numUsers = 15;
		XnUserID users[numUsers];
		userGenerator.GetUsers(users, numUsers);
		if( numUsers != 1) continue;
        
		userGenerator.GetUserPixels(users[0], scene);
        
        // Update patient position
        previous = current;
        current = getPosition(users[0]);
        
        if( previous != current ) {
            if( current == TURNED ) {
                // Patient is turned
                printf("Patient getting out of bed.\n");
            } else if( current == LAYING ) {
				printf("Patient is laying down.\n");
			}
        }
    }
}

Position KinectMonitor::getPosition(XnUserID user) {
    XnSkeletonJointPosition headPos, torsoPos, leftPos, rightPos;
	double head, torso, left, right;
	Position position = UNKNOWN;
	SkeletonCapability skeleton = userGenerator.GetSkeletonCap();
	
	// Get Joint positions
	skeleton.GetSkeletonJointPosition(user, XN_SKEL_HEAD, headPos);
	skeleton.GetSkeletonJointPosition(user, XN_SKEL_TORSO, torsoPos);
	skeleton.GetSkeletonJointPosition(user, XN_SKEL_LEFT_SHOULDER, leftPos);
	skeleton.GetSkeletonJointPosition(user, XN_SKEL_RIGHT_SHOULDER, rightPos);
	
	head	= headPos.position.Z;
	torso	= torsoPos.position.Z;
	left	= leftPos.position.Z;
	right	= rightPos.position.Z;
	
	// Determine defined position
	if(head - LAYING_TOLERANCE > torso) {
		position = LAYING;
		
	} else if(	left < right - TURNED_TOLERANCE || 
				left > right + TURNED_TOLERANCE	) {
		position = TURNED;
		
	} else {
		position = FORWARD;
	}
	
	return position;
}
