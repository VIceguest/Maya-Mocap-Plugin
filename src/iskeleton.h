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

#ifndef __ISKELETON_H__
#define __ISKELETON_H__

#include <vector>
#include <string>
#include <map>

#include "imath.hpp"

///////////////////////////////////////////////////////////////////////////////
// result values
//
enum MCDATA_RESULT {
	MC_SUCCESS = 0,
	MC_EOF,
	MC_INVALID_STREAM,
	MC_INVALID_SKELETON,
	MC_INVALID_JOINT,
	MC_DUP_JOINT_NAME,
	MC_ILLEAGAL_DATA,
	MC_FATAL_ERROR
};

///////////////////////////////////////////////////////////////////////////////
namespace Rotation {
	enum MC_ROT_ORDER {
		MC_RO_XYZ, MC_RO_YZX, MC_RO_ZXY,
		MC_RO_ZYX, MC_RO_YXZ, MC_RO_XZY,
		MC_RO_NONE = 0x80000000
	};

	bool isOrderValid(const int order);
	int getReversedOrder(const int order);
	int getOrderFromString(std::string order);
	std::string getStringFromOrder(int order);
};

///////////////////////////////////////////////////////////////////////////////
// class for skeletons
//
class iSkeleton {
public:

	//////////////////////////////////////
	// inner struct for motion data
	//
	struct iFrame {
		imath::iVec offset;
		imath::iVec rotation;
		double scale;
	};
	//
	//////////////////////////////////////

	//////////////////////////////////////
	// inner class for joints
	//
	class iJoint {
	public:
		class iaccessory {
		public:
			iaccessory() {}
			virtual ~iaccessory() {}
		};
	private:
		iaccessory *accessory;	// reserved for additional data
		// joint's characteristic
		std::string name;
		iJoint *father;
		std::vector<iJoint *> children;
		// joint's properties
		imath::iVec offset;
		imath::iVec rotation;
		double length;		// reserved for .htr/.htr2 format
	public:
		// define a callback type
		class icallbackData {
			unsigned int level;
		public:
			icallbackData() : level(0) {}
			virtual ~icallbackData() {}
			unsigned int getLevel() { return level; }
			void levelInc() { ++level; }
			void levelDec() { --level; }
		};
		typedef void (* TRANVERSE_CALLBACK)(iJoint *item, icallbackData *data);

		// motion data
		std::vector<iFrame> motion;

		// constructor
        iJoint() : father(NULL), length(0.0), accessory(NULL) {}
		iJoint(const std::string &nick) : father(NULL), name(nick), length(0.0), accessory(NULL) {}
		// destructor
		~iJoint() {
			// remove additional data
			if (NULL != accessory) {
				delete accessory;
				accessory = NULL;
			}
		}
		// get/set name
		std::string getName() { return name; }
		void setName(const std::string &nick) { name = nick; }
		// get/set father
		iJoint *getFather() { return father; }
		void setFather(iJoint *parent) { father = parent; }
		// get/set offset
		void getOffset(imath::iVec &of) { of = offset; }
		void setOffset(const imath::iVec &of) { offset = of; }
		// get/set rotation
		void getRotation(imath::iVec &rot) { rot = rotation; }
		void setRotation(const imath::iVec &rot) { rotation = rot; }
		// get/set length
		double getLength() { return length; }
		void setLength(double len) { length = len; }
		// get/set accessory
		iaccessory *getAccessory() { return accessory; }
		void setAccessory(iaccessory *data) { accessory = data; }

		// add child
		int addChild(iJoint *child) {
			if (NULL == child) return MC_INVALID_JOINT;
			children.push_back(child);
			return MC_SUCCESS;
		}
		// get child
		iJoint *getChild(unsigned int idx) {
			if (idx > static_cast<unsigned int>(children.size())) {
				throw imath::badIndex("child index out of bounds");
			}
			return children[static_cast<std::vector <iJoint *>::size_type>(idx)];
		}
		/*
		// kill all children and their offspring
		int removeChildren() {
			std::vector<iJoint *>::iterator iter;
			for (iter = children.begin(); iter != children.end(); ++iter) {
				(*iter)->removeChildren();
				delete *iter;
				*iter = NULL;
			}
			// don't forget to clear the vector
			children.clear();
			return MC_SUCCESS;
		}
		// kill a child and its offspring
		int removeChild(iJoint *child) {
			if (NULL == child) return MC_INVALID_JOINT;
			std::vector<iJoint *>::iterator iter;
			for (iter = children.begin(); iter != children.end(); ++iter) {
				if (*iter == child) {
					// kill it's children first !!!
					child->removeChildren();
					// then itself
					children.erase(iter);
					delete child;
					return MC_SUCCESS;
				}
			}
			return MC_INVALID_JOINT;
		}
		*/
		// count children
		unsigned int countChildren() {
			return static_cast<unsigned int>(children.size());		// should be vector<iJoint*>::sizetype ??
		}
		// preorder scan
		void preOrder(TRANVERSE_CALLBACK visit, icallbackData *data);
		// postorder scan
		void postOrder(TRANVERSE_CALLBACK visit, icallbackData *data);
		// print itself
		friend std::ostream& operator<<(std::ostream &out, iJoint &temp) {
			temp.output(out);
			return out;
		}
	private:
		// overriding standard output operator
		void output(std::ostream &out) {
			out << name << "[ " << offset << ", " << rotation << ", " << length << " ]";
		}

	};
	//
	//////////////////////////////////////

	typedef std::map<std::string, iJoint *>::value_type valType;

	// constructor
	iSkeleton() : root(NULL), current(NULL), rotationOrder(Rotation::MC_RO_ZXY),
		frames(0), frameTime(0.4), scaleOrientation(0), haveTranslation(false) {}
	// destructor
	//virtual ~iSkeleton() {}
	// empty
	bool empty() { return (NULL == root); }
	// exist
	bool exist(iJoint *joint);
	// root
	bool isRoot(iJoint *joint) { return ((NULL != joint) && (joint == root)); }
	// add a joint and move current pointer to it
	int addJoint(const std::string &name, const imath::iVec &of,
		const imath::iVec &rot, const double &len = 0.0);
	// get a joint from it's name
	iJoint *getJoint(std::string name);
	// get a joint from current pointer
	iJoint *getJoint();
	// set/get parameters
	void setFrames(unsigned int count) { frames = count; }
	unsigned int getFrames() { return frames; }
	void setFrameTime(double time) { frameTime = time; }
	double getFrameTime() { return frameTime; }
	void setRotOrder(int ord) { rotationOrder = ord; }
	int getRotOrder() { return rotationOrder; }
	void setScaleOrientation(unsigned int o) { scaleOrientation = o; }
	unsigned int getScaleOrientation() { return scaleOrientation; }
	void setHaveTranslation(bool o) { haveTranslation = o; }
	bool getHaveTranslation() { return haveTranslation; }
	/*
	// remove a joint
	int removeJoint() {
		if (NULL == current) return MC_INVALID_JOINT;
		int result;
		iJoint *parent = current->getFather();
		if (NULL == parent) {
			result = current->removeChildren();
			if (MC_SUCCESS == result) {
				delete current;
				root = current = NULL;
			}
		} else {
			result = parent->removeChild(current);
			if (MC_SUCCESS == result) {
				current = parent;
			}
		}
		return result;
	}
	// remove all joints
	void clear() {
		if (NULL != root) {
			// mass killing !!!
			root->removeChildren();
			delete root;
			root = current = NULL;
		}
		// clear map of joints
		dict.clear();
	}
	*/
	// move current pointer to the parent
	int goUp();
	// move current pointer to root
	int goTop();
	// move current pointer here
	int goHere(iJoint *joint);

	// print itself
	friend std::ostream& operator<<(std::ostream &out, iSkeleton &temp) {
		temp.output(out);
		return out;
	}
private:
	std::string name;
	iJoint *root;
	iJoint *current;
	// add this map to speed up progress of motion data loading 
	std::map<std::string, iJoint *> dict;

	// motion parameters
	int rotationOrder;
	unsigned int frames;
	double frameTime;

	// extra properties for HTR files
	unsigned int scaleOrientation;
	bool haveTranslation;

	// overriding standard output operator
	void output(std::ostream &out) {
		if (NULL != root) outputFamily(out, root);
	}

	void outputFamily(std::ostream &out, iJoint *joint, unsigned int level = 0) {
		const std::string leading(level, ' ');
		out << leading << *joint << std::endl;
		for (unsigned int i = 0; i < joint->countChildren(); ++i) {
			outputFamily(out, joint->getChild(i), level + 1);
		}
	}
};

#endif	// #define __ISKELETON_H__
