////////////////////////////////////////////////////////////////////////////
//
//  MoCap File Importer
//  Copyright(c) Shun Cox (shuncox@gmail.com)
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IMOCAPDATA_H__
#define __IMOCAPDATA_H__

#define REQUIRE_IOSTREAM
#include <maya/MIOStream.h>
//#include <MStreamUtils.h>
#include <string>
#include <vector>

#include "iskeleton.h"
#include "iconverter.h"

using namespace std;

// postfix for effectors
const string effectorPostfix("_Effector");
// horizontal line
const string horizontalLine(40, '=');
// replacement of space character
const string replacementOfSpace("_");

// max words in joint name
const unsigned int maxWordsJointName = 10;
// max number of joints
const unsigned int maxNumJoints = 1000;
// max frame rate
const unsigned int maxFrameRate = 1000;
// max number of frames
const unsigned int maxNumFrames = 100000;
// motion translation threshold
const double motionThreshold = 0.001;

///////////////////////////////////////////////////////////////////////////////
// class for frames
//
//class iMotionFrames {
//public:
//
//	iMotionFrames() {};
//	
//	// frames are accessible through operator[]
//	iFrame &operator[](unsigned int idx) {
//		if (idx >= frames.size()) throw badIndex("Frame index out of bounds");
//		return frames[idx];
//	}
//	iFrame operator[](unsigned int idx) const {
//		if (idx >= frames.size()) throw badIndex("const Frame index out of bounds");
//		return frames[idx];
//	}
//private:
//	vector<iFrame> frames;
//};

///////////////////////////////////////////////////////////////////////////////
// base class for mocap data
//
class iMocapData {
protected:
	istream *input;
	iSkeleton *skeleton;
public:
	// constructor
	iMocapData() : input(NULL), skeleton(NULL) {}
	iMocapData(istream *in, iSkeleton *sk) : input(in), skeleton(sk) {}
	// attach input stream and skeleton
	int attach(istream *in, iSkeleton *sk);
	// load mocap data
	int load(const std::string& strJointNameSuffix="");
protected:
	virtual int parsing(const std::string&) { return 0; };
};

#endif	// #ifndef __IMOCAPDATA_H__
