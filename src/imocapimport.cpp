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

// To keep compatibility with maya 5.0
#include <fstream>
#include <ctime>

#define REQUIRE_IOSTREAM

#include <maya/MFileObject.h>
#include <maya/MAngle.h>
#include <maya/MVector.h>
#include <maya/MQuaternion.h>
#include <maya/MStringArray.h>
#include <maya/MGlobal.h>
#include <maya/MFnIkJoint.h>
#include <maya/MTime.h>
#include <maya/MAnimControl.h>
#include <maya/MMatrix.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>

#include "idebug.h"
#include "macros.h"
#include "imocapdatabvh.h"
#include "imocapimport.h"

using namespace std;
using namespace imath;

///////////////////////////////////////////////////////////////////////////////
// Description for the options of imocapImport 
///////////////////////////////////////////////////////////////////////////////
// bool		bonesOnly		: Extract skeleton from mocap file only
//							  or with motion data
// bool		merge			: Apply motion data on existing skeleton
//							: or on the new created skeleton
// float	scale			: Determine the actual size of skeletons
// uint		rotationOrder	: The roation order of every bones
// uint		startFrame		: The start frame of motion section
// uint		endFrame		: The end frame of motion section
//							  ( value 0x80000000 for the whole section )
///////////////////////////////////////////////////////////////////////////////

// To keep compatibility with Mac OSX
#if defined (OSMac_CFM_)
#	define USING_MAC_CORE_LIB 1
#else
#	define USING_MAC_CORE_LIB 0
#endif

#if defined (OSMac_)
#pragma message(Mac OSX was detected!)
#	include <sys/param.h>
#	if USING_MAC_CORE_LIB
#		include <Files.h>
#		include <CFURL.h>
extern "C" Boolean createMacFile (const char *fileName, FSRef *fsRef, long creator, long type);
extern "C" Boolean convertFileRepresentation (char *fileName, short inStyle, short outStyle);
#	endif
extern "C" int strcasecmp (const char *, const char *);
#endif

//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
imocapImport::imocapImport() : MPxFileTranslator()
{
}

//-----------------------------------------------------------------------------
// unconstructor
//-----------------------------------------------------------------------------
imocapImport::~imocapImport()
{
}

//-----------------------------------------------------------------------------
// creator
//-----------------------------------------------------------------------------
void *imocapImport::creator() { return new imocapImport(); }

//-----------------------------------------------------------------------------
// status functions
//-----------------------------------------------------------------------------
bool imocapImport::haveReadMethod() const { return true; }
bool imocapImport::haveWriteMethod() const { return false; }
bool imocapImport::haveNamespaceSupport () const { return false; }
bool imocapImport::canBeOpened() const { return true; }
//MString imocapImport::defaultExtension() const { return MString("bvh"); }
MString imocapImport::filter() const { return MString("*.bvh"); }//;*.htr;*.htr2

//-----------------------------------------------------------------------------
// reader
//-----------------------------------------------------------------------------
MStatus imocapImport::reader(
	const MFileObject& file,
	const MString& options,
	FileAccessMode mode
	)
{
	MStatus stat = MS::kFailure;

	ILOG2 (horizontalLine);
	ILOG2 (" Game started");
	ILOG2 (horizontalLine);

	// Get the filename from MFileObject
	MString filename = file.fullName();
	ILOG2 ("Filename = " << filename);

	// To keep compatibility with Mac OSX
#	if defined (OSMac_)
	char osxFilename[MAXPATHLEN];
	strcpy (osxFilename, filename.asChar());
#		if USING_MAC_CORE_LIB
	convertFileRepresentation (osxFilename, kCFURLPOSIXPathStyle, kCFURLHFSPathStyle);
#		endif
#	endif

	// 	Parse the options. The options syntax is in the form of
	//	"flag=val;flag1=val;flag2=val"
	//
	if (options.length() > 0) {

		// Start parsing
		ILOG2 ("Options = " << options);
		MStringArray optionList, theOption;
		options.split(';', optionList);

		unsigned int nOptions = optionList.length();
		for (unsigned int i = 0; i < nOptions; i ++) {
			theOption.clear();
			// Missing parameter
			optionList[i].split('=', theOption);
			if (theOption.length() < 2) continue;
			// Get parameters
			if (theOption[0] == "bonesOnly") {
				paramBlock.bonesOnly = (theOption[1].toLowerCase() == "true");
				ILOG2("Gotta param 'bonesOnly' = " << paramBlock.bonesOnly);
			} else if (theOption[0] == "merge") {
				paramBlock.merge = (theOption[1].toLowerCase() == "true");
				ILOG2("Gotta param 'merge' = " << paramBlock.merge);
			} else if (theOption[0] == "scale") {
				paramBlock.scale = theOption[1].asFloat();
				ILOG2("Gotta param 'scale' = " << paramBlock.scale);
			} else if (theOption[0] == "rotationOrder") {
				string order(theOption[1].asChar());
				paramBlock.rotationOrder = Rotation::getOrderFromString(order);
				ILOG2("Gotta param 'rotationOrder' = " << paramBlock.rotationOrder);
			} else if (theOption[0] == "startFrame") {
				paramBlock.startFrame = theOption[1].asUnsigned();
				ILOG2("Gotta param 'startFrame' = " << paramBlock.startFrame);
			} else if (theOption[0] == "endFrame") {
				paramBlock.endFrame = theOption[1].asUnsigned();
				ILOG2("Gotta param 'endFrame' = " << paramBlock.endFrame);
			}
		}

		// Who can tell me how to obtain the default namespace??
		{
			//myNamespace = "Reference";
			myNamespace = file.name();
			int pos = myNamespace.rindex('.');
			if (pos == -1) pos = myNamespace.length();
			--pos;
			myNamespace = myNamespace.substring(0, pos);
			ILOG2 ("Namespace = " << myNamespace);
		}


		//MGlobal::displayInfo("Don't disturb me. I'm working...");
		clock_t time = clock();

		// To keep compatibility with Mac OSX
#		if defined (OSMac_)
		stat = importMocapFile(MString(osxFilename), (mode == kOpenAccessMode));
#		else
		stat = importMocapFile(filename, (mode == kOpenAccessMode));
#		endif

		if (!stat.error()) {
			//MGlobal::displayInfo(MString("Elapsed time : ") + ((static_cast<float>(clock()) - time) / CLOCKS_PER_SEC) + "s");
			//MGlobal::displayInfo("Mocap file has been imported. Enjoy it pls.");
			ILOG2 (horizontalLine);
			ILOG2 (" Mission accomplished");
			ILOG2 (horizontalLine);
		} else {
			MGlobal::displayError(stat.errorString());
			ILOG2 (horizontalLine);
			ILOG2 (" Mission failed");
			ILOG2 (horizontalLine);
		}
	}

	return stat;
}

//-----------------------------------------------------------------------------
// identifyFile
//-----------------------------------------------------------------------------
MPxFileTranslator::MFileKind imocapImport::identifyFile(
	const MFileObject& fileName,
	const char* buffer,
	short size
	) const
{
	MString filename(fileName.name());

	if (recognition(filename) != imocapImport::MC_FT_UNKNOWN) {
		return kIsMyFileType;
	}

	// The right file magic
	//
	//if (strncmp(buffer, "HIERARCHY", 9) == 0) {
	//	ILOG1("The file magic is 'HIERACHY'. Right!");
	//	return kIsMyFileType;
	//}

	ILOG1("The file is NOT mine. sorry!");
	return kNotMyFileType;
}

//-----------------------------------------------------------------------------
// Recognition
//-----------------------------------------------------------------------------
imocapImport::MC_FILE_TYPE imocapImport::recognition(MString filename) const
{
	const int len = filename.length() - 1;
	const int pos = filename.rindex('.');
	MString extName;

	if (pos != -1) {
		extName = (filename.toLowerCase()).substring(pos + 1, len);

		// The right name extension
		//
		if (extName == "bvh") {
			ILOG1("This is a BVH file. Right!");
			return imocapImport::MC_FT_BVH;
		}

// 		if (extName == "htr" || extName == "htr2") {
// 			ILOG1("This is a HTR file. Right!");
// 			return imocapImport::MC_FT_HTR;
//		}
	}

	return imocapImport::MC_FT_UNKNOWN;
}

//importBvhString
MStatus imocapImport::importBvhString( MString strBvh )
{
	MS_ENTRANCE	// Entry for critical zone
	fstream fBvh;
	fBvh<<strBvh.asChar();
	// perparing for importion
	iSkeleton skel;
	iMocapData *dataMocap = NULL;


	ILOG1 ("Importing  BVH string...");
	dataMocap = new iMocapDataBvh(&fBvh, &skel);


	if (dataMocap->load() != MC_SUCCESS) {
		ILOG4 ("Error: load failed!");
		// delete imocapData object
		if (!dataMocap) {
			delete dataMocap;
			dataMocap = NULL;
		}
		MS_CHECK(MStatus::kFailure);
	} else {
		ILOG2 ("load ok!");
	}
	// delete imocapData object
	if (!dataMocap) {
		delete dataMocap;
		dataMocap = NULL;
	}

	MS_CHECK(rebuildSkeleton(skel, false));

	MS_EXIT		// Exit for emergency
		MS_RETURN	// Return for emergency
}

//-----------------------------------------------------------------------------
// importMocapFile
//-----------------------------------------------------------------------------
MStatus imocapImport::importMocapFile(const MString filename, const bool isOpen,const std::string& strJointNameSuffix)
{
	MS_ENTRANCE	// Entry for critical zone

		const MC_FILE_TYPE type = recognition(filename);

	if (type != MC_FT_UNKNOWN) {

		ifstream fileMocap(filename.asChar());
		if (fileMocap) {
			// perparing for importion
			iSkeleton skel;
			iMocapData *dataMocap = NULL;

			switch (type) {
			case MC_FT_BVH:
				ILOG1 ("Importing a BVH file...");
				dataMocap = new iMocapDataBvh(&fileMocap, &skel);
				break;
// 			case MC_FT_HTR:
// 				ILOG1 ("Importing a HTR file...");
// 				dataMocap = new iMocapDataHtr(&fileMocap, &skel);
// 				break;
			default:
				ILOG4 ("Error: More file type will be supported in the future...");
				fileMocap.close();
				MS_CHECK(MStatus::kFailure);
				break;
			}
			
			if (dataMocap->load(strJointNameSuffix) != MC_SUCCESS) {
				ILOG4 ("Error: load failed!");
				// delete imocapData object
				if (!dataMocap) {
					delete dataMocap;
					dataMocap = NULL;
				}
				fileMocap.close();
				MS_CHECK(MStatus::kFailure);
			} else {
				ILOG2 ("load ok!");
			}
			// delete imocapData object
			if (!dataMocap) {
				delete dataMocap;
				dataMocap = NULL;
			}
			fileMocap.close();

			MS_CHECK(rebuildSkeleton(skel, isOpen));
		}

	} else {
		ILOG4 ("Error: Unknown file type...");
		MS_CHECK(MStatus::kFailure);
	}

	MS_EXIT		// Exit for emergency
		MS_RETURN	// Return for emergency
}

//-----------------------------------------------------------------------------
// rebuildSkeleton
//-----------------------------------------------------------------------------
MStatus imocapImport::rebuildSkeleton(iSkeleton &skel, const bool isOpen)
{
	MS_ENTRANCE	// Entry for critical zone

		if (skel.empty()) {
			ILOG4 ("Error: Skeleton is empty");
			MS_CHECK(MStatus::kInvalidParameter);
		}

		ILOG1 (skel);

		ILOG2 (horizontalLine);
		ILOG2 (" Skeleton & Animation Rebuilding...");
		ILOG2 (horizontalLine);

		imyCallbackData data;
		iSkeleton::iJoint *jot;

		skel.goTop();
		if ((jot = skel.getJoint()) == NULL) {
			ILOG4 ("Error: Internal error");
			MS_CHECK(MStatus::kInvalidParameter);
		}

		// Prepare for callback data
		//
		data.myNamespace = myNamespace;
		data.haveTranslation = skel.getHaveTranslation();

		data.onlyBones	= paramBlock.bonesOnly;
		data.injection	= paramBlock.merge;
		data.proportion	= paramBlock.scale;

		if (Rotation::MC_RO_NONE == paramBlock.rotationOrder) {
			data.order = skel.getRotOrder();
		} else {
			data.order = paramBlock.rotationOrder;
		}
		ILOG2("Rotation order = " << data.order);

		// Assign variable 'frameBegin'
		if (IM_INT_DEFAULT == paramBlock.startFrame) {
			data.frameBegin = 0;
		} else {
			data.frameBegin = paramBlock.startFrame;
		}
		ILOG2("Start frame = " << data.frameBegin);

		// Assign variable 'frames'
		unsigned int n = skel.getFrames();
		if (IM_INT_DEFAULT == paramBlock.endFrame) {
			data.frameEnd = n;
		} else {
			if (paramBlock.endFrame > n) {
				data.frameEnd = n;
			} else {
				data.frameEnd = paramBlock.endFrame;
			}
		}
		ILOG2("End frame = " << data.frameEnd);

		// Assign variable 'interval'
		if (IM_DOUBLE_DEFAULT == paramBlock.frameTime) {
			data.interval = skel.getFrameTime();
		} else {
			data.interval = paramBlock.frameTime;
		}
		ILOG2("Frame time = " << data.interval);

		// Get current time or...
		//
		if (!isOpen) {
			data.currentTime = MAnimControl::currentTime();
		} else {
			data.injection = false;	// It's impossible!
		}

		// Import Bones Only Mode is prefered !!!
		if (data.onlyBones) {
			ILOG1(">>> Into Bones Only Mode...");
			data.injection = false;
		} else {
			ILOG1(">>> Into Normal Mode...");
		}

		// Get selection
		if (data.injection) {
			ILOG1(">>> Into Merge Mode...");

			data.onlyBones = false;	// It's impossible!

			MSelectionList selection;
			MS_CHECK(MGlobal::getActiveSelectionList(selection));
			MItSelectionList iter(selection, MFn::kJoint);
			//for ( ; !iter.isDone(); iter.next()) {
			//}
			// Only one selection is needed
			if (!iter.isDone()) {
				MS_CHECK(iter.getDagPath(data.dagPath));
				ILOG2("We gotta the selection!");
			} else {
				ILOG4("Error: No joint was selected");
				MGlobal::displayError("You should select the root joint firstly!");
				MS_CHECK(MStatus::kInvalidParameter);
			}
		}

		data.result = MStatus::kSuccess;
		// Start rebuilding...
		jot->preOrder(skeletonCallback, &data);
		MS_CHECK(data.result)

			MS_EXIT		// Exit for emergency
			MS_RETURN	// Return for emergency
}

//-----------------------------------------------------------------------------
// callBack
//-----------------------------------------------------------------------------
void skeletonCallback(iSkeleton::iJoint *item, iSkeleton::iJoint::icallbackData *data)
{
	MStatus stat;
	imocapImport::imyCallbackData *mdata = dynamic_cast<imocapImport::imyCallbackData*>(data);

	if (NULL == mdata) {
		ILOG4 ("Error: dynamic_cast failed");
		return;
	}

	//bool injection;

	MS_ENTRANCE	// Entry for critical zone
		MS_CHECK(mdata->result)	// Something wrong?


		// To create or seek the joint
		MObject joint;
	if (mdata->injection) {
		ILOG1("Seek the joint...");
		if (!mdata->onlyBones && seekJoint(item, mdata, joint) == MStatus::kSuccess) {
			// To animate the joint
			MS_CHECK(animateJoint(item, mdata, joint));
		}
	} else {
		ILOG1("Create the joint...");
		MS_CHECK(createJoint(item, mdata, joint));
		if (!mdata->onlyBones) {
			// To animate the joint
			MS_CHECK(animateJoint(item, mdata, joint));
		}
	}

	MS_EXIT	// Exit for emergency
		MS_STORE(mdata->result)
}

//-----------------------------------------------------------------------------
// createJoint
//-----------------------------------------------------------------------------
MStatus createJoint(iSkeleton::iJoint *item, imocapImport::imyCallbackData *mdata, MObject &joint)
{
	MStatus stat;
	MS_ENTRANCE	// Entry for critical zone

		float scale = mdata->proportion;
	unsigned int rotationOrder = Rotation::getReversedOrder(mdata->order);
	// Get information of the joint
	iVec off;
	item->getOffset(off);
	if (1.0 != scale) off *= scale;

	MObject parent;
	if (mdata->getLevel() == 0) {
		//parent = MObject::kNullObj;
		// Create a Transform node as root
		//
		MFnTransform mfnTrans;
		parent = mfnTrans.create(MObject::kNullObj, &stat); MS_CHECK(stat);
		mfnTrans.setName(mdata->myNamespace, &stat); MS_CHECK(stat);
	} else {
		// Get the object of parent joint
		imocapImport::imyAccessory *pacc = dynamic_cast<imocapImport::imyAccessory*>
			((item->getFather())->getAccessory());
		IASSERT(NULL != pacc);
		parent = pacc->obj;
		IASSERT(MObject::kNullObj != parent);
	}

	// Create a new joint
	MFnIkJoint mfnJoint;
	joint = mfnJoint.create(parent, &stat); MS_CHECK(stat);

	// Attach an accessory to it
	//
	imocapImport::imyAccessory *acc = new imocapImport::imyAccessory;
	acc->obj = joint;
	item->setAccessory(acc);

	// STILL missing duplicated name checking
	//
	MString jointName((item->getName()).c_str());
	mfnJoint.setName(jointName, &stat);	MS_CHECK(stat);
	ILOG1("Create Joint '" << jointName << "' @ " << off);

	// Set translation
	//
	MVector loc(off.x, off.y, off.z);
	//mfnJoint.setTranslation(loc, MSpace::kObject);
	MS_CHECK(mfnJoint.setTranslation(loc, MSpace::kTransform));

	// Setup the joint rotation order
	//
	MS_CHECK(mfnJoint.setRotationOrder(iorderToTrOrder(rotationOrder), false));

	// Set base rotation (joint orientation)
	// Needed by HTR format
	//
	iVec baseRotation;
	item->getRotation(baseRotation);
	if (baseRotation.x != 0 || baseRotation.y != 0 || baseRotation.z != 0) {
		const MEulerRotation baseEuler(
			toRadians(baseRotation.x),
			toRadians(baseRotation.y),
			toRadians(baseRotation.z),
			iorderToEuOrder(rotationOrder));
		MS_CHECK(mfnJoint.setOrientation(baseEuler));
	}

	MS_EXIT		// Exit for emergency
		MS_RETURN	// Return for emergency
}

//-----------------------------------------------------------------------------
// seekJoint
//-----------------------------------------------------------------------------
MStatus seekJoint(iSkeleton::iJoint *item, imocapImport::imyCallbackData *mdata, MObject &joint)
{
	MStatus stat;
	MS_ENTRANCE

		MObject parent;
	if (mdata->getLevel() == 0) {
		if (mdata->dagPath.hasFn(MFn::kJoint)) {
			joint = mdata->dagPath.node(&stat); MS_CHECK(stat);
			ILOG1("Root joint is selected");
		} else {
			ILOG4("ERROR: No root joint was selected!");
			MS_CHECK(MStatus::kInvalidParameter);
		}
	} else {
		// Get the object of parent joint
		imocapImport::imyAccessory *pacc = dynamic_cast<imocapImport::imyAccessory*>
			((item->getFather())->getAccessory());
		IASSERT(NULL != pacc);
		parent = pacc->obj;
		IASSERT(MObject::kNullObj != parent);

		// Seek the joint
		MFnIkJoint mfnParent, mfnChild;
		MObject lastJoint;
		mfnParent.setObject(parent);
		unsigned int count = 0;
		unsigned int tcount= mfnParent.childCount(&stat); MS_CHECK(stat);
		MString name = item->getName().c_str();
		unsigned int i;
		for (i = 0; i < tcount; ++i) {
			MObject child = mfnParent.child(i, &stat); MS_CHECK(stat);
			// make sure they are joints
			if (child.hasFn(MFn::kJoint)) {
				count ++;
				lastJoint = child;
				mfnChild.setObject(child);

				// TODO : check accessory duplication
				// ...

				MString childName = mfnChild.name(&stat); MS_CHECK(stat);
				// Non-casesensetive comparation
				ILOG0(" Wanted = " << name.toLowerCase() << " Got = " << getNeatName(childName).toLowerCase());
				if (getNeatName(childName).toLowerCase() == name.toLowerCase()) {
					joint = child;
					ILOG1("Found the child!");
					break;
				}
			}
		}

		ILOG0("Found " << count << " joint(s) under " << mfnParent.name());

		if (count == 0) {
			// No child
			ILOG1("No more leaf!");
			MS_CHECK(MStatus::kNotFound);
		} else if (count == 1) {
			// Only one child
			joint = lastJoint;
		}
	}

	// Attach an accessory to it
	//
	imocapImport::imyAccessory *acc = new imocapImport::imyAccessory;
	acc->obj = joint;
	item->setAccessory(acc);

	MS_EXIT
		MS_RETURN
}

//-----------------------------------------------------------------------------
// animateJoint
//-----------------------------------------------------------------------------
MStatus animateJoint(iSkeleton::iJoint *item, imocapImport::imyCallbackData *mdata, const MObject &joint)
{
	MStatus stat;
	MS_ENTRANCE

		// Get parameters from callback data block
		float scale = mdata->proportion;
	bool motionOffset = mdata->haveTranslation;
	unsigned int startFrame = mdata->frameBegin;
	unsigned int endFrame = mdata->frameEnd;
	MTime time(mdata->currentTime);
	MTime frameTime(mdata->interval, MTime::kSeconds);
	MTime endTime(time + frameTime * (endFrame - startFrame));

	// animate skeleton
	//
	MFnIkJoint mfnJoint;
	stat = mfnJoint.setObject(joint); MS_CHECK(stat);

	MFnAnimCurve acRx, acRy, acRz;
	MS_CHECK(getAnimCurve(joint, MString("rotateX"), acRx));
	if (validate(acRx)) MS_CHECK(removeAnimCurveKeys(acRx, time, endTime));
	MS_CHECK(getAnimCurve(joint, MString("rotateY"), acRy));
	if (validate(acRy)) MS_CHECK(removeAnimCurveKeys(acRy, time, endTime));
	MS_CHECK(getAnimCurve(joint, MString("rotateZ"), acRz));
	if (validate(acRz)) MS_CHECK(removeAnimCurveKeys(acRz, time, endTime));

	MFnAnimCurve acTx, acTy, acTz;
	// not only root has translations in HTR files
	//
	if (mdata->getLevel() == 0 || motionOffset) {
		// Translation curve for joints
		//
		MS_CHECK(getAnimCurve(joint, MString("translateX"), acTx));
		if (validate(acTx)) MS_CHECK(removeAnimCurveKeys(acTx, time, endTime));
		MS_CHECK(getAnimCurve(joint, MString("translateY"), acTy));
		if (validate(acTy)) MS_CHECK(removeAnimCurveKeys(acTy, time, endTime));
		MS_CHECK(getAnimCurve(joint, MString("translateZ"), acTz));
		if (validate(acTz)) MS_CHECK(removeAnimCurveKeys(acTz, time, endTime));

	}

	// Loading keyframes
	//
	// If endFrame exceed the capacity of the motion clip, cut the rest
	//
	if (endFrame > item->motion.capacity()) {
		endFrame = static_cast<unsigned int>(item->motion.capacity());
	}
	ILOG1("Retrieving keys from " << startFrame << " to " << endFrame);

	iVec baseOffset;
	item->getOffset(baseOffset);

	for (unsigned int i = startFrame; i < endFrame; ++i) {
		iSkeleton::iFrame &fm = item->motion[i];
		const iVec rot = fm.rotation;
		if (validate(acRx)) MS_CHECK(acRx.addKeyframe(time, rot.x));
		if (validate(acRy)) MS_CHECK(acRy.addKeyframe(time, rot.y));
		if (validate(acRz)) MS_CHECK(acRz.addKeyframe(time, rot.z));
		//		ILOG0("Time: " << time << "\tAdd keyframe (rotation): " << rot);
		if ((mdata->getLevel() == 0) || motionOffset) {
			// Basic offset is useless in BVH file ?!
			//
			const iVec ofs = (baseOffset + fm.offset) * scale;
			//iVec ofs = fm.offset * scale;
			if (validate(acTx)) MS_CHECK(acTx.addKeyframe(time, ofs.x));
			if (validate(acTy)) MS_CHECK(acTy.addKeyframe(time, ofs.y));
			if (validate(acTz)) MS_CHECK(acTz.addKeyframe(time, ofs.z));
			//			ILOG0("\tAdd keyframe (Translation): " << ofs);
		}

		time += frameTime;
	}

	MS_CHECK_RELAY	// Relay the emergency

		ILOG1("Keyframes loaded");
	MS_EXIT		// Exit for emergency
		MS_RETURN
}

//-----------------------------------------------------------------------------
// getAnimCurve
//-----------------------------------------------------------------------------
MStatus getAnimCurve(const MObject &joint, const MString attr, MFnAnimCurve &curve)
{
	MStatus stat;
	MS_ENTRANCE

		curve.setObject(MObject::kNullObj);
	MPlug plug = MFnDependencyNode(joint).findPlug(attr, &stat); MS_CHECK(stat);

	//if (!plug.isKeyable() || plug.isLocked()) {
	//	ILOG4("attribute " << attr << " is locked or not keyable");
	//	MS_CHECK(MStatus::kNotFound);
	//}

	if (!plug.isKeyable()) {
		ILOG4("attribute " << attr << " is not keyable");
		MS_CHECK(plug.setKeyable(true));
	}
	if (plug.isLocked()) {
		ILOG4("attribute " << attr << " is locked");
		MS_CHECK(plug.setLocked(false));
	}

	if (!plug.isConnected()) {

		// There are eight different types of Anim Curve nodes:
		//
		//	* timeToAngular (animCurveTA)
		//	* timeToLinear (animCurveTL)
		//	* timeToTime (animCurveTT)
		//	* timeToUnitless (animCurveTU)
		//	* unitlessToAngular (animCurveUA)
		//	* unitlessToLinear (animCurveUL)
		//	* unitlessToTime (animCurveUT)
		//	* unitlessToUnitless (animCurveUU)

		curve.create(joint, plug, MFnAnimCurve::kAnimCurveTL, NULL, &stat);
		if (stat != MStatus::kSuccess) {
			ILOG1("Creating Animation Curve failed");
			MS_CHECK(MStatus::kNotFound);
		}
	} else {
		// Plug is connected, find out the AnimCurve node
		MFnAnimCurve animCurve(plug, &stat);
		if (stat == MStatus::kNotImplemented) {
			ILOG1("This plug has more than one Animation Curves, pick up one");
		} else if (stat != MStatus::kSuccess) {
			ILOG1("No Animation Curves found");
			MS_CHECK(stat);
		}
		curve.setObject(animCurve.object(&stat)); MS_CHECK(stat);
	}

	MS_EXIT
		MS_RETURN
}

//-----------------------------------------------------------------------------
// removeAnimCurveKeys
//-----------------------------------------------------------------------------
MStatus removeAnimCurveKeys(MFnAnimCurve &curve, const MTime &startTime, const MTime &endTime)
{
	MStatus stat;
	MS_ENTRANCE

		if (!validate(curve)) MS_CHECK(MStatus::kInvalidParameter);

	unsigned int startIndex, endIndex;
	startIndex = curve.findClosest(startTime, &stat);
	if (stat == MStatus::kSuccess) {
		endIndex = curve.findClosest(endTime, &stat);
		if (stat == MStatus::kSuccess) {
			ILOG1("Remove keys between index " << startIndex << " to " << endIndex);
			// Number of key is one at least
			for (unsigned int i = startIndex; i < endIndex; ++i) {
				// The index will be different while a key is removed
				MS_CHECK(curve.remove(startIndex));
			}
		}
	}

	MS_EXIT
		MS_RETURN
}
