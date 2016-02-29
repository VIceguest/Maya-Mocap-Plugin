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

#ifndef __IDEBUG_H__
#define __IDEBUG_H__

#ifdef __cplusplus

#ifndef _DEBUG


#	define IASSERT(x)
#	define ILOG(x)
#	define ILOG0 ILOG
#	define ILOG1 ILOG
#	define ILOG2 ILOG
#	define ILOG3 ILOG
#	define ILOG4 ILOG


#else	// else of _DEBUG

#include <cassert>
#define IASSERT(x)	{ assert(x); }

#if defined (_WIN32)
// for win32
#	include <sstream>
#	include <windows.h>
#	define ILOG(x)	{\
		std::ostringstream buf;\
		buf << "LOG -> " << __FUNCTION__ << "(" << __FILE__ << ":" << __LINE__ <<") = \t" << x << std::endl;\
		OutputDebugString((buf.str()).c_str());\
	}

#else
// for other os
#	define REQUIRE_IOSTREAM
#	include <maya/MIOStream.h>
#	define ILOG(X) { std::clog << "LOG -> " << __FUNCTION__ << "() = \t" << x << std::endl; }

#endif

#define INLOG(x)

// ILOG
// 0: for a great unimportant messages
// 1: for many useful messages    
// 2: for rare informative message
// 3: for general warnings
// 4: for fatal errors

#ifdef VERBOSITY0
#	define ILOG0 ILOG
#	define ILOG1 ILOG
#	define ILOG2 ILOG
#	define ILOG3 ILOG
#	define ILOG4 ILOG
#endif

#ifdef VERBOSITY1
#	define ILOG0 INLOG
#	define ILOG1 ILOG
#	define ILOG2 ILOG
#	define ILOG3 ILOG
#	define ILOG4 ILOG
#endif

#ifdef VERBOSITY2
#	define ILOG0 INLOG
#	define ILOG1 INLOG
#	define ILOG2 ILOG
#	define ILOG3 ILOG
#	define ILOG4 ILOG
#endif

#ifdef VERBOSITY3
#	define ILOG0 INLOG
#	define ILOG1 INLOG
#	define ILOG2 INLOG
#	define ILOG3 ILOG
#	define ILOG4 ILOG
#endif

#ifdef VERBOSITY4
#	define ILOG0 INLOG
#	define ILOG1 INLOG
#	define ILOG2 INLOG
#	define ILOG3 INLOG
#	define ILOG4 ILOG
#endif

#endif	// end of _DEBUG

#else	// else of __cplusplus

#error 'IDEBUG.H' IS FOR CPLUSPLUS COMPILERS ONLY !!!

#endif	// end of __cplusplus

#endif	// #define __IDEBUG_H__
