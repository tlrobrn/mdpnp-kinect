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
#include <signal.h>
#include "../includes/monitor.h"

// Use OpenNI's xn namespace
using namespace xn;

// Constructors
KinectMonitor::KinectMonitor() {
    XnStatus status;
    EnumerationErrors errors;
    XnCallbackHandle userCallbacks;
    
    // Setup Context
    status = context.InitFromXmlFile(CONTEXT_XML, scriptNode, &errors);
    if( status == XN_STATUS_NO_NODE_PRESENT ) {
        XnChar strError[1024];
        errors.ToString(stdError, 1024);
        printf("%s\n", strError);
        
        exit(status);
    } else if( status != XN_STATUS_OK ) {
        printf("Could not initialize Context: %s\n", xnGetStatusString(status));
        
        exit(status);
    }
    
    // Setup Depth Generator
    status = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depthGenerator);
	CHECK_RC(status, "Find depth generator");
	
	// Setup User Generator
	status = context.FindExistingNode(XN_NODE_TYPE_USER, userGenerator);
	if( status != XN_STATUS_OK ) {
		status = userGenerator.Create(context);
		CHECK_RC(status, "Find user generator");
	}
	
	// Setup Skeletal Mapping
	if( !userGenerator.IsCapibilitySupported(XN_CAPABILITY_SKELETON) ) {
		printf("Skeletal mapping not supported.\n");
		
		exit(1);
	}
	userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_UPPER);
	
	// Register Callbacks
	status = userGenerator.RegisterUserCallbacks(
	    this.foundUser, this.lostUser, NULL, userCallbacks
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
    person.Release();
    context.Release();
}
        
// Setup
void KinectMonitor::loadCalibration(XnUserID user) {
    if( userGenerator.GetSkeletonCap().IsCalibrated(user) ) return;
    
    // Load file
    XnStatus status = userGenerator.GetSkeletonCap().
        LoadCalibrationDataFromFile(user, CALIBRATION_FILE);
    
    // Start tracking
    if( status == XN_STATUS_OK )
        userGenerator.GetSkeletonCap().StartTracking(user);
}
        
// Get data
void KinectMonitor::run() {
    XnStatus status;
    
    // Start the device
    status = context.StartGeneratingAll();
    CHECK_RC(status, "StartGenerating");
    
    // Running loop
    while( !quit ) {
        // Wait for incoming data
        context.WaitOneUpdateAll(depthGenerator);
        
        // Update patient position
        previous = current;
        current = getPosition(0);
        
        if( previous != current ) {
            if( current == TURNED ) {
                // Patient is turned
                printf("Patient getting out of bed.");
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
        
// Callbacks
void XN_CALLBACK_TYPE KinectMonitor::foundUser(UserGenerator &generator,
    XnUserID nID, void *pCookie) {
    
    printf("NEW PERSON\n");
    loadCalibration(nId);
}
void XN_CALLBACK_TYPE KinectMonitor::lostUser(UserGenerator &generator,
    XnUserID nID, void *pCookie) {
    
    printf("PERSON LEFT");
}

// Signal Handler
void KinectMonitor::stop(int signal) {
    quit = true;
}