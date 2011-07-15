/*
 *  MotionDetector Class
 *  
 *  Modified from the OpenKinectProject
 *  Used under the terms of the Apache License v2.0 and GPL2
 *  See the following URLs for license details:
 *  http://www.apache.org/licenses/LICENSE-2.0
 *  http://www.gnu.org/licenses/gpl-2.0.txt
 *
 *	Taylor O'Brien
 *	15 July 2011
 */

#include "libfreenect.hpp"
#include "mutex.h"

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <cmath>
#include <vector>

using namespace std;

class MotionDetector : public Freenect::FreenectDevice {
private:
	vector<uint8_t>		m_buffer_depth;
	vector<uint8_t>		m_buffer_video;
	vector<uint16_t>	m_gamma;
	Mutex::Mutex		m_rgb_mutex;
	Mutex::Mutex		m_depth_mutex;
	bool				m_new_rgb_frame;
	bool				m_new_depth_frame;
	
public:
	MotionDetector(freenect_context *_ctx, int _index);
	void VideoCallback(void *_rgb, uint32_t timestamp);
	void DepthCallback(void *_depth, uint32_t timestamp);
	bool getRGB(vector<uint8_t> &buffer);
	bool getDepth(vector<uint8_t> &buffer);
};