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

#include "idebug.h"
#include "imocapdata.h"
//-----------------------------------------------------------------------------
// attach input stream and skeleton
//-----------------------------------------------------------------------------
int iMocapData::attach(istream *in, iSkeleton *sk)
{
	// check input stream and skeleton
	if (NULL == in) {
		ILOG4 ("Error: Invalid stream");
		return MC_INVALID_STREAM;
	}
	if (NULL == sk) {
		ILOG4 ("Error: Invalid skeleton");
		return MC_INVALID_SKELETON;
	}
	input = in;
	skeleton = sk;
	ILOG0 ("Success");
	return MC_SUCCESS;
}

//-----------------------------------------------------------------------------
// load file
//-----------------------------------------------------------------------------
int iMocapData::load(const std::string& strJointNameSuffix)
{
	return parsing(strJointNameSuffix);
}
