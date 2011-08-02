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
* 	Last Modified 21 July 2011 by Taylor O'Brien								*
********************************************************************************/
#ifndef MONITOR_H
#define MONITOR_H
//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------
#include <XnCppWrapper.h>

//-------------------------------------------------------------------------------
//	Enums & Structs
//-------------------------------------------------------------------------------
enum Position {LAYING, TURNED, FORWARD, UNKNOWN};

//-------------------------------------------------------------------------------
//	Functions
//-------------------------------------------------------------------------------
void CleanupExit();
void LoadCalibration(XnUserID user);
Position getPosition(XnUserID user);
void getPositionString(XnUserID user, char *posStr);


/* Callbacks */
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, 
	XnUserID nID, void* pCookie);
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator,
	XnUserID nId, void* pCookie);

//-------------------------------------------------------------------------------
#endif
