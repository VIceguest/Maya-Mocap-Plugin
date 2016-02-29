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

#ifndef __IMOCAPIMPORT_H__
#define __IMOCAPIMPORT_H__

#define REQUIRE_IOSTREAM

#include <maya/MPxFileTranslator.h>
#include <maya/MEulerRotation.h>
#include <maya/MTime.h>
#include <maya/MDagPath.h>
#include <maya/MStringArray.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MPlug.h>
#include "iskeleton.h"
#include <string>

#define IM_INT_DEFAULT		0x80000000L	// 2147483648L
#define IM_DOUBLE_DEFAULT	0.0

inline MTransformationMatrix::RotationOrder iorderToTrOrder(int rotationOrder) {
	MTransformationMatrix::RotationOrder order;
	switch (rotationOrder) {
	case Rotation::MC_RO_XYZ: order = MTransformationMatrix::kXYZ; break;
	case Rotation::MC_RO_YZX: order = MTransformationMatrix::kYZX; break;
	case Rotation::MC_RO_ZXY: order = MTransformationMatrix::kZXY; break;
	case Rotation::MC_RO_XZY: order = MTransformationMatrix::kXZY; break;
	case Rotation::MC_RO_YXZ: order = MTransformationMatrix::kYXZ; break;
	case Rotation::MC_RO_ZYX: order = MTransformationMatrix::kZYX; break;
	default: order = MTransformationMatrix::kLast; break;
	};
	return order;
}

inline MEulerRotation::RotationOrder iorderToEuOrder(int rotationOrder) {
	MEulerRotation::RotationOrder order;
	switch (rotationOrder) {
	case Rotation::MC_RO_XYZ: order = MEulerRotation::kXYZ; break;
	case Rotation::MC_RO_YZX: order = MEulerRotation::kYZX; break;
	case Rotation::MC_RO_ZXY: order = MEulerRotation::kZXY; break;
	case Rotation::MC_RO_XZY: order = MEulerRotation::kXZY; break;
	case Rotation::MC_RO_YXZ: order = MEulerRotation::kYXZ; break;
	case Rotation::MC_RO_ZYX: order = MEulerRotation::kZYX; break;
	};
	return order;
}

inline MString getNeatName(MString dirtyName) {
	MStringArray array;
	dirtyName.split(':', array);
	return array[array.length() - 1];
}

inline bool validate(const MFnBase &mfnObj) {
	return (mfnObj.object() != MObject::kNullObj);
}

class imocapImport : public MPxFileTranslator {
public:
	imocapImport();
	virtual ~imocapImport();

	static void *creator();

	virtual MStatus reader(
		const MFileObject& file,
		const MString& options,
		FileAccessMode mode
	);

	virtual MFileKind identifyFile(
		const MFileObject& fileName,
		const char* buffer,
		short size
	) const;

	virtual bool haveReadMethod() const;
	virtual bool haveWriteMethod() const;
	virtual bool haveNamespaceSupport () const;
	virtual bool canBeOpened() const;
//	virtual MString defaultExtension() const;
	virtual MString filter() const;

private:
	// Parameter block
	class imocapParam {
	public:
		imocapParam() {
			// Set parameters to default value
			bonesOnly = true;
			merge = false;
			scale = 1.0;
			rotationOrder = Rotation::MC_RO_NONE;
			startFrame = IM_INT_DEFAULT;
			endFrame = IM_INT_DEFAULT;
			frameTime = IM_DOUBLE_DEFAULT;
		}
		bool	bonesOnly;		// Extract skeleton from mocap file only
		bool	merge;			// Apply motion data on existing skeleton
		float	scale;			// Determine the actual size of skeletons
		unsigned int		rotationOrder;	// The roation order of every bones
		unsigned int		startFrame;		// The start frame of motion section (0...n)
		unsigned int		endFrame;		// The end frame of motion section (0...n)
		double	frameTime;		// The interval between frames (second)
	} paramBlock;

public:
	enum MC_FILE_TYPE { MC_FT_UNKNOWN, MC_FT_BVH, MC_FT_HTR };
	enum FRAME_DATA_TYPE{FRAME_DT_FIRST,FRAME_DT_NORMAL};
	// Callback data block
	//
	class imyCallbackData : public iSkeleton::iJoint::icallbackData {
	public:
		imyCallbackData() : icallbackData() {}
		virtual ~imyCallbackData() {}
		// Parameters
		bool onlyBones;
		bool injection;
		float proportion;
		unsigned int order;
		unsigned int frameBegin;
		unsigned int frameEnd;
		double interval;
		MTime currentTime;
		MDagPath dagPath;
		MString myNamespace;		// Namespace
		bool haveTranslation;
		// Result
		MStatus result;
	};

	// Accessory for joints
	//
	class imyAccessory : public iSkeleton::iJoint::iaccessory {
	public:
		MObject obj;			// Store the object of joints
		imyAccessory() : obj(MObject::kNullObj), iaccessory() {}
		virtual ~imyAccessory() {}
	};
public:
	MStatus importMocapFile(const MString filename, const bool isOpen,const std::string& strJointNameSuffix="");
	MString myNamespace;		// Namespace
	MStatus importBvhString(MString strBvh);//add by jiangbin
private:
	MC_FILE_TYPE recognition(MString filename) const;
	//MStatus importBvhFile(MString filename, iSkeleton &skobj);
	MStatus rebuildSkeleton(iSkeleton &skobj, const bool isOpen);
	friend void skeletonCallback(iSkeleton::iJoint *item, iSkeleton::iJoint::icallbackData *data);
};

MStatus createJoint(iSkeleton::iJoint *item, imocapImport::imyCallbackData *mdata,  MObject &joint);
MStatus seekJoint(iSkeleton::iJoint *item, imocapImport::imyCallbackData *mdata, MObject &joint);
MStatus animateJoint(iSkeleton::iJoint *item, imocapImport::imyCallbackData *mdata, const MObject &joint);
MStatus getAnimCurve(const MObject &joint, const MString attr, MFnAnimCurve &curve);
MStatus removeAnimCurveKeys(MFnAnimCurve &curve, const MTime &startTime, const MTime &endTime);

#endif	// #define __IMOCAPIMPORT_H__
