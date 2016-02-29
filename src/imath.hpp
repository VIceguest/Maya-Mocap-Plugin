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

#ifndef __IMATH_HPP__
#define __IMATH_HPP__

#include <iostream>
#include <string>
#include <cmath>

namespace imath {

const double PI = 3.1415926535897932384626433832795;

enum iRotationOrder {
	IRO_XYZ, IRO_YZX, IRO_ZXY,
	IRO_ZYX, IRO_YXZ, IRO_XZY,
	IRO_NONE = 0x80000000
};

inline float toDegrees(float rad) { return static_cast<float>(rad * 180.0 / PI); }
inline double toDegrees(double rad) { return (rad * 180.0 / PI); }
inline float toRadians(float deg) { return static_cast<float>(deg * PI / 180.0); }
inline double toRadians(double deg) { return (deg * PI / 180.0); }

#include "ivector.hpp"

typedef iVector<float> iVecF;
typedef iVector4<float> iVec4F;

typedef iVector<double> iVec;
typedef iVector4<double> iVec4;

} // namespace imath

#endif	// __IMATH_HPP__
