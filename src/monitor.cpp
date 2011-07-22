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
//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------
#include <XnOpenNI.h>
#include <XnTypes.h>
#include <XnOS.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include <GL/glut.h>
#include "../includes/monitor.h"

// Use OpenNI's xn namespace
using namespace xn;

//-------------------------------------------------------------------------------
//	Macros
//-------------------------------------------------------------------------------
#define CONTEXT_XML			"../config/config.xml"
#define CALIBRATION_FILE	"../config/calibration.bin"
#define LAYING_TOLERANCE	200
#define TURNED_TOLERANCE	150
#define WINDOW_X			720
#define WINDOW_Y			480
#define MAX_DEPTH			10000

#define CHECK_RC(status, what)										\
	if(status != XN_STATUS_OK) {									\
		printf("%s failed: %s\n", what, xnGetStatusString(status));	\
		return status;												\
	}

//-------------------------------------------------------------------------------
//	Globals
//-------------------------------------------------------------------------------
Context			_context;
ScriptNode		_scriptNode;
DepthGenerator	_depthGenerator;
UserGenerator	_userGenerator;
Player			_person;

float	_depthHist[MAX_DEPTH];
GLfloat	_texcoords[8];
XnFloat	_colors[][3] = {
	{0,1,1},
	{0,0,1},
	{0,1,0},
	{1,1,0},
	{1,0,0},
	{1,.5,0},
	{.5,1,0},
	{0,.5,1},
	{.5,0,1},
	{1,1,.5},
	{1,1,1}
};
XnUInt32 _numColors = 10;

XnBool	_drawSkeleton	= TRUE;
XnBool	_printID		= TRUE;
XnBool	_printState		= TRUE;
XnBool	_quit			= FALSE;

//-------------------------------------------------------------------------------
//	Implementation
//-------------------------------------------------------------------------------
void CleanupExit() {
	_scriptNode.Release();
	_depthGenerator.Release();
	_userGenerator.Release();
	_person.Release();
	_context.Release();
	
	exit(1);
}

void LoadCalibration(XnUserID user) {
	// Done if user is already calibrated
	if(_userGenerator.GetSkeletonCap().IsCalibrated(user)) return;
		
	// Load Calibration File
	XnStatus stat = _userGenerator.GetSkeletonCap().
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
	SkeletonCapability skeleton = _userGenerator.GetSkeletonCap();
	
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

void getPositionString(XnUserID user, char *posStr) {
	xnOSMemSet(posStr, 0, sizeof(posStr));
	Position position = getPosition(user);
	
	if(position == LAYING) {
		sprintf(posStr, "Laying");
	} else if(position == TURNED) {
		sprintf(posStr, "!!! - Getting Up - !!!");
	} else if(position == FORWARD) {
		sprintf(posStr, "");
	} else {
		sprintf(posStr, "???");
	}
}

/* Callbacks */
void XN_CALLBACK_TYPE User_NewUser(UserGenerator& generator, 
	XnUserID nId, void* pCookie) {
	// New User has entered the scene
	printf("Person recognized.\tID: %d\n",nId);
	LoadCalibration(nId);
}

void XN_CALLBACK_TYPE User_LostUser(UserGenerator& generator,
	XnUserID nId, void* pCookie) {
	// User has exited the scene
	printf("Person lost.\t\tID: %d\n", nId);
}

//--------------------------------------------------------------------------------
//	Drawing Scene (and auxiliary functions)
//-------------------------------------------------------------------------------
unsigned int getClosestPowerOfTwo(unsigned int n) {
	unsigned int m = 2;
	while(m < n) m<<=1;
	
	return m;
}

GLuint initTexture(void** buf, int& width, int& height) {
	GLuint texID = 0;
	glGenTextures(1, &texID);
	
	width = getClosestPowerOfTwo(width);
	height = getClosestPowerOfTwo(height);
	*buf = new unsigned char[width*height*4];
	
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	return texID;
}

void drawRectangle(float left, float top, float right, float bottom) {
	GLfloat verts[8] = {	
		left, top,
		left, bottom,
		right, bottom,
		right, top
	};
	
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	
	glFlush();
}

void drawTexture(float left, float top, float right, float bottom) {
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, _texcoords);
	
	drawRectangle(left, top, right, bottom);
	
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void printString(void* font, char *str) {
	int l = strlen(str);
	for(int i = 0; i < l; i++) {
		glutBitmapCharacter(font, *str++);
	}
}

void drawLimb(XnUserID person, XnSkeletonJoint joint1, XnSkeletonJoint joint2) {
	// Exit if the person's skeleton is not being tracked
	if(!_userGenerator.GetSkeletonCap().IsTracking(person)) return;
	
	SkeletonCapability 	skeleton = _userGenerator.GetSkeletonCap();
	XnSkeletonJointPosition pos1, pos2;
	
	// Get joint positions
	skeleton.GetSkeletonJointPosition(person, joint1, pos1);
	skeleton.GetSkeletonJointPosition(person, joint2, pos2);
	// If position confidence is too low, return
	if(pos1.fConfidence < 0.5 || pos2.fConfidence < 0.5) return;
	
	XnPoint3D pt[2];
	pt[0] = pos1.position;
	pt[1] = pos2.position;
	
	_depthGenerator.ConvertRealWorldToProjective(2, pt, pt);
	glVertex3i(pt[0].X, pt[0].Y, 0);
	glVertex3i(pt[1].X, pt[1].Y, 0);	
}

void DrawDepthMap(const DepthMetaData& dmd, const SceneMetaData& smd) {
	static bool initialized = false;
	static GLuint depthTexID;
	static unsigned char* depthTexBuf;
	static int texWidth, texHeight;
	
	float left, top, right, bottom;
	float texXpos, texYpos;
	
	// Initialization
	if(!initialized) {
		texWidth = getClosestPowerOfTwo(dmd.XRes());
		texHeight = getClosestPowerOfTwo(dmd.YRes());
		depthTexID = initTexture((void**)&depthTexBuf, texWidth, texHeight);
		
		initialized = true;
		
		left = dmd.XRes();
		top = 0;
		bottom = dmd.YRes();
		right = 0;
		
		texXpos = left/texWidth;
		texYpos = bottom/texHeight;
		
		memset(_texcoords, 0, 8*sizeof(float));
		_texcoords[0] = texXpos;
		_texcoords[1] = texYpos;
		_texcoords[2] = texXpos;
		_texcoords[7] = texYpos;
	}
	
	unsigned int value = 0;
	unsigned int histValue = 0;
	unsigned int index = 0;
	unsigned int X = 0;
	unsigned int Y = 0;
	unsigned int numberOfPoints = 0;
	XnUInt16 xRes = dmd.XRes();
	XnUInt16 yRes = dmd.YRes();
	
	unsigned char* destImage = depthTexBuf;
	const XnDepthPixel* depth = dmd.Data();
	const XnLabel* labels = smd.Data();
	
	// Calculate the histogram (accumulative)
	memset(_depthHist, 0, MAX_DEPTH*sizeof(float));
	for(Y = 0; Y < yRes; Y++) {
		for(X = 0; X < xRes; X++) {
			value = *depth;
			if(value != 0) {
				_depthHist[value]++;
				numberOfPoints++;
			}
			depth++;
		}
	}
	
	for(index = 1; index < MAX_DEPTH; index++) {
		_depthHist[index] += _depthHist[index-1];
	}
	
	if(numberOfPoints) {
		for(index = 1; index < MAX_DEPTH; index++) {
			_depthHist[index] = (unsigned int)
				(256 * (1.0f - (_depthHist[index]/numberOfPoints)));
		}
	}
	
	depth = dmd.Data();
	XnUInt32 ind = 0;
	// Prepare the texture map
	for(Y = 0; Y < yRes; Y++) {
		for(X = 0; X < xRes; X++) {
			destImage[0] = 0;
			destImage[1] = 0;
			destImage[2] = 0;
			depth++;
			labels++;
			destImage+=3;
		}
	}
	
	glBindTexture(GL_TEXTURE_2D, depthTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 
		0, GL_RGB, GL_UNSIGNED_BYTE, depthTexBuf);
	
	// Display texture map
	glColor4f(0.75,0.75,0.75,1);
	
	glEnable(GL_TEXTURE_2D);
	drawTexture(dmd.XRes(), dmd.YRes(), 0, 0);
	glDisable(GL_TEXTURE_2D);
	
	// Labeling
	char strLabel[50] = "";
	XnUInt16 numUsers = 15;
	XnUserID users[numUsers];
	_userGenerator.GetUsers(users, numUsers);
	
	for(int i = 0; i < numUsers; i++) {
		// Display labels
		if(_printID) {
			XnPoint3D com;
			_userGenerator.GetCoM(users[i], com);
			_depthGenerator.ConvertRealWorldToProjective(1, &com, &com);
			
			xnOSMemSet(strLabel, 0, sizeof(strLabel));
			if(_printState) {
				char position[25];
				getPositionString(users[i], position); 
				sprintf(strLabel, "%d - %s", users[i], position);
			} else {
				sprintf(strLabel, "%d", users[i]);
			}
			
			glColor4f(
				1 - _colors[i%_numColors][0],
				1 - _colors[i%_numColors][1],
				1 - _colors[i%_numColors][2],
				1
			);
			
			glRasterPos2i(com.X, com.Y);
			printString(GLUT_BITMAP_HELVETICA_18, strLabel);
		}
		// Display Skeleton
		if(_drawSkeleton && _userGenerator.GetSkeletonCap().IsTracking(users[i])) {
			glBegin(GL_LINES);
				glColor4f(
					1 - _colors[i%_numColors][0],
					1 - _colors[i%_numColors][1],
					1 - _colors[i%_numColors][2],
					1
				);
				drawLimb(users[i], XN_SKEL_HEAD, XN_SKEL_NECK);
				
				drawLimb(users[i], XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER);
				drawLimb(users[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW);
				drawLimb(users[i], XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);
				
				drawLimb(users[i], XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER);
				drawLimb(users[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW);
				drawLimb(users[i], XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);
				
				drawLimb(users[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO);
				drawLimb(users[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO);
				
				drawLimb(users[i], XN_SKEL_TORSO, XN_SKEL_LEFT_HIP);
				drawLimb(users[i], XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE);
				drawLimb(users[i], XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT);
				
				drawLimb(users[i], XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP);
				drawLimb(users[i], XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE);
				drawLimb(users[i], XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT);
				
				drawLimb(users[i], XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP);
			glEnd();
		}
	}
}

//-------------------------------------------------------------------------------
//	GLUT
//-------------------------------------------------------------------------------
void glutDisplay(void) {
	SceneMetaData scene;
	DepthMetaData depth;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Setup viewpoint
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	_depthGenerator.GetMetaData(depth);
	glOrtho(0, depth.XRes(), depth.YRes(), 0, -1.0, 1.0);
	
	glDisable(GL_TEXTURE_2D);
	
	// Wait for incoming data
	_context.WaitOneUpdateAll(_depthGenerator);
	_depthGenerator.GetMetaData(depth);
	_userGenerator.GetUserPixels(0, scene);
	
	/*/ Get user position
	_previous = _current;
	_current = getPosition(0);
	
	if(_previous != _current) { // If the position has changed
		if(_current == TURNED)	printf("PATIENT GETTING OUT OF BED");
		// Could put other notifications here as well.
	}*/
	
	// Draw the scene
	DrawDepthMap(depth, scene);
	glutSwapBuffers();
}

void glutIdle(void) {
	if(_quit) {
		CleanupExit();
	}
	glutPostRedisplay();
}

void glutKeyboard(unsigned char key, int x, int y) {
	switch(key) {
		case 27: //esc
			CleanupExit();
			break;
		case 's':
			_drawSkeleton = !_drawSkeleton;
			break;
		case 'i':
			_printID = !_printID;
			break;
		case 'l':
			_printState = !_printState;
			break;
	}
}

void glInit(int* pargc, char** argv) {
	glutInit(pargc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_X, WINDOW_Y);
	glutCreateWindow("MDPnP Patient Monitoring");
	glutSetCursor(GLUT_CURSOR_NONE);
	
	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

//-------------------------------------------------------------------------------
//	Main
//-------------------------------------------------------------------------------
int main(int argc, char **argv) {
	XnStatus status;
	EnumerationErrors errors;
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
