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

#ifndef __IMOCAPDATABVH_H__
#define __IMOCAPDATABVH_H__

#include "imocapdata.h"

#define MC_GET_WORD if ((result = tokenBvh.getWord(word)) != MC_SUCCESS) break;
#define MC_TO_UPPER toUppercase(word);

///////////////////////////////////////////////////////////////////////////////
// class for BVH mocap files
//
class iMocapDataBvh : public iMocapData {
public:
	iMocapDataBvh(istream *in, iSkeleton *sk) : iMocapData(in, sk) {}
private:
	//////////////////////////////////////
	// inner class for file parsing
	//
	class iTokenizerBvh {
		istream *input;
		vector<string> words;
		vector<string>::iterator iter;
	public:
		iTokenizerBvh() : input(NULL), iter(words.begin()) {}
		iTokenizerBvh(istream *in) : input(in), iter(words.begin()) {}
		int attach(istream *in);
		int getWord(string &oneword);
	};
	//
	//////////////////////////////////////

	//////////////////////////////////////
	// inner struct for linkage between joints and channels
	//
	struct iChannelLink {
		string jointName;
		string typeOrder;
	};
	//
	//////////////////////////////////////
	// parse mocap data
	int parsing(const string&  strJointNameSuffix="");
};

#endif	// #ifndef __IMOCAPDATABVH_H__
