/********************************************************************************
*	MDPnP Patient Kinect Monitor												*
* 	Taylor O'Brien, University of Illinois										*
*																				*
* 	This software uses OpenNI components and modules to monitor hospital		*
* 	patients in bed.  It will send a notification when	a patient appears to	*
* 	be in the process of getting out of bed.									*
* 																				*
* 	This code was created using examples and modified sample code from			*
* 	OpenNI. Used by permission under the GNU Lesser General Public License.		*
* 																				*
* 	Last Modified 21 July 2011 by Taylor O'Brien								*
********************************************************************************/
//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------
#include <XnOpenNI.h>
#include <XnTypes.h>
#include <XnOS.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include "monitor.h"

//-------------------------------------------------------------------------------
//	Macros
//-------------------------------------------------------------------------------
#define CONTEXT_XML			"../config/config.xml"
#define CALIBRATION_FILE	"../config/calibration.bin"
#define LAYING_TOLERANCE	200
#define TURNED_TOLERANCE	150

//-------------------------------------------------------------------------------
//	Implementation
//-------------------------------------------------------------------------------
void CleanupExit() {
	_scriptNode.Release();
	_depthGenerator.Release();
	_userGenerator.Release();
	_patient.Release();
	_context.Release();
	
	exit(1);
}

void LoadCalibration(XnUserID user) {
	// Done if user is already calibrated
	if(_userGenerator.GetSkeletonCap().IsCalibrated(user)) return;
		
	// Load Calibration File
	XnStatus stat = _userGenerator.GetSkeletopCap().
		LoadCalibrationDataFromFile(user, CALIBRATION_FILE);
		
	// Start Tracking
	if(stat == XN_STATUS_OK) {
		_userGenerator.GetSkeletonCap().StartTracking(user);
	}
}

Position getPosition(XnUserID user) {
	XnSkeletonJointPosition headPos, torsoPos, leftPos, rightPos;
	double head, torso, left, right;
	Position position = UNKNOWN;
	xn::SkeletonCapability skeleton = _userGenerator.GetSkeletonCap();
	
	// Get Joint positions
	skeleton.GetJointPosition(user, XN_SKEL_HEAD, headPos);
	skeleton.GetJointPosition(user, XN_SKEL_TORSO, torsoPos);
	skeleton.GetJointPosition(user, XN_SKEL_LEFT_SHOULDER, leftPos);
	skeleton.GetJointPosition(user, XN_SKEL_RIGHT_SHOULDER, rightPos);
	
	head	= headPos.position.Z;
	torso	= tosroPos.posistion.Z;
	left	= leftPos.posistion.Z;
	right	= rightPos.posistion.Z;
	
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

/* Callbacks */
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, 
	XnUserID nID, void* pCookie) {
	// New User has entered the scene
	printf("Person recognized.\tID: %d\n",nId);
	LoadCalibration(nId);
}

void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator,
	XnUserID nId, void* pCookie) {
	// User has exited the scene
	printf("Person lost.\tID: %d\n", nId);
}

//-------------------------------------------------------------------------------
//	Main
//-------------------------------------------------------------------------------
int main(int argc, char **argv) {
	XnStatus status;
	xn::EnumerationErrors errors;
	XnCallbackHandle userCallbacks;
	
	// Setup context
	status = _context.InitFromXmlFile(CONTEXT_XML, _scriptNode, &errors);
	if(status == XN_STATUS_NO_NODE_PRESENT) {
		XnChar strError[1024];
		errors.ToString(strError, 1024);
		printf("%s\n", strError);
		return status;
		
	} else if(status != XN_STATUS_OK) {
		printf("Could not initialize Context: %s\n", xnGetStatusString(status));
		return status;
	}
	
	// Setup depth generator
	status = _context.FindExistingNode(XN_NODE_TYPE_DEPTH, _depthGenerator);
	CHECK_RC(status, "Find depth generator");
	
	// Setup user generator
	status = _context.FindExistingNode(XN_NODE_TYPE_USER, _userGenerator);
	if(status != XN_STATUS_OK) {
		status = _userGenerator.Create(_context);
		CHECK_RC(status, "Find user generator");
	}
	
	// Check for skeleton support
	if(!_userGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON)) {
		printf("Skeletal mapping not supported.\n");
		return 1;
	}
	// Setup Skeleton Profile
	_userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_UPPER);
	
	// Register Callbacks
	status = _userGenerator.
		RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, userCallbacks);
	CHECK_RC(status, "Register to user callbacks");
	
	// Start gathering & generating data
	status = _context.StartGeneratingAll();
	CHECK_RC(status, "StartGenerating");
	
	// GLUT graphics
	glInit(&argc, argv);
	glutMainLoop();
}
//-------------------------------------------------------------------------------
