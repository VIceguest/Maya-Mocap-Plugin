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

#include <algorithm>

#include "idebug.h"
#include "iskeleton.h"

using namespace std;
using namespace imath;

//---------------------------------------------------------------------------
namespace Rotation {
	bool isOrderValid(int order)
	{
		if (order < MC_RO_XYZ || order > MC_RO_XZY) return false;
		return true;
	}

//---------------------------------------------------------------------------
	int getReversedOrder(const int order)
	{
		const int ro[] = {MC_RO_ZYX, MC_RO_XZY, MC_RO_YXZ,
			MC_RO_XYZ, MC_RO_ZXY, MC_RO_YZX};
		if (isOrderValid(order)) {
			return ro[order];
		}
		return MC_RO_NONE;
	}

//---------------------------------------------------------------------------
	int getOrderFromString(string order)
	{
		transform(order.begin(), order.end(), order.begin(), (int(*)(int))toupper);	
		if (!order.compare("XYZ")) return MC_RO_XYZ;
		if (!order.compare("YZX")) return MC_RO_YZX;
		if (!order.compare("ZXY")) return MC_RO_ZXY;
		if (!order.compare("ZYX")) return MC_RO_ZYX;
		if (!order.compare("YXZ")) return MC_RO_YXZ;
		if (!order.compare("XZY")) return MC_RO_XZY;
		return MC_RO_NONE;
	}

//---------------------------------------------------------------------------
	string getStringFromOrder(int order)
	{
		const string ostr[] = {"xyz", "yzx", "zxy", "zyx", "yxz", "xzy"};
		if (!isOrderValid(order)) {
			return string("none");
		}
		return ostr[order];
	}
}
//---------------------------------------------------------------------------
void iSkeleton::iJoint::preOrder(TRANVERSE_CALLBACK visit, icallbackData *data)
{
	ILOG1 ("Visit joint " << name << " by preorder @ level " << data->getLevel());
	IASSERT(NULL != data);
	(*visit)(this, data);		// callback function
	vector<iJoint *>::iterator iter;
	data->levelInc();
	//data->offsetAdd(this);
	for (iter = children.begin(); iter != children.end(); ++iter) {
		(*iter)->preOrder(visit, data);
	}
	//data->offsetSub(this);
	data->levelDec();
}

//---------------------------------------------------------------------------
void iSkeleton::iJoint::postOrder(TRANVERSE_CALLBACK visit, icallbackData *data)
{
	vector<iJoint *>::iterator iter;
	IASSERT(NULL != data);
	data->levelInc();
	//data->offsetAdd(this);
	for (iter = children.begin(); iter != children.end(); ++iter) {
		(*iter)->preOrder(visit, data);
	}
	//data->offsetSub(this);
	data->levelDec();
	ILOG1 ("Visit joint " << name << " by postorder @ level " << data->getLevel());
	(*visit)(this, data);		// callback function
}

//---------------------------------------------------------------------------
// add a joint and move current pointer to it
//---------------------------------------------------------------------------
int iSkeleton::addJoint(const string &name, const iVec &of, const iVec &rot, const double &len)
{
	// has it already existed ?
	if (dict.count(name) != 0) {
		ILOG4 ("Error: Joint name has existed");
		return MC_DUP_JOINT_NAME;
	}
	// ok, let's make a baby
	iJoint *baby = new iJoint(name);
	IASSERT(NULL != baby);

	//baby->setName(name);
	baby->setFather(current);
	baby->setOffset(of);
	baby->setRotation(rot);
	baby->setLength(len);
	if (NULL == current) {
		// current is point to root
		// if root is not empty then report a error
		if (NULL != root) {
			delete baby;	// missing it will cause memory leaking !!!
			ILOG4 ("Error: Root is not empty");
			return MC_INVALID_JOINT;
		}
		// otherwise ...
		root = current = baby;
		dict.insert(valType(name, baby));
	} else {
		// current is not point to root
		current->addChild(baby);
		dict.insert(valType(name, baby));
		// go down
		current = baby;
	}
	ILOG0 ("Success");
	return MC_SUCCESS;
}

//---------------------------------------------------------------------------
// get a joint from it's name
//---------------------------------------------------------------------------
iSkeleton::iJoint *iSkeleton::getJoint(string name)
{
	//I have made a big mistake !!!!!
	//if (dict.count(name) != 0) {

	if (dict.count(name) == 0) {
		ILOG4 ("Error: Cannot find the joint named " << name);
		return NULL;
	}
	return dict[name];
}

//---------------------------------------------------------------------------
// get a joint from current pointer
//---------------------------------------------------------------------------
iSkeleton::iJoint *iSkeleton::getJoint()
{
	return current;
}

//---------------------------------------------------------------------------
// move current pointer to the parent
//---------------------------------------------------------------------------
int iSkeleton::goUp()
{
	if (NULL == current) return MC_INVALID_JOINT;
	iJoint *parent = current->getFather();
	if (NULL == parent) return MC_INVALID_JOINT;
	current = parent;
	return MC_SUCCESS;
}

//---------------------------------------------------------------------------
// move current pointer to root
//---------------------------------------------------------------------------
int iSkeleton::goTop()
{
	//if (NULL == current) return MC_INVALID_JOINT;
	//iJoint *parent = current->getFather();
	if (NULL == root) return MC_INVALID_JOINT;
	current = root;
	return MC_SUCCESS;
}

//---------------------------------------------------------------------------
// move current pointer here
//---------------------------------------------------------------------------
int iSkeleton::goHere(iSkeleton::iJoint *joint)
{
	if (!exist(joint)) return MC_INVALID_JOINT;
	current = joint;
	return MC_SUCCESS;
}

//---------------------------------------------------------------------------
// check the joint
//---------------------------------------------------------------------------
bool iSkeleton::exist(iSkeleton::iJoint *joint)
{
	bool result = false;
	map<string, iSkeleton::iJoint *>::iterator iter;
	for (iter = dict.begin(); iter != dict.end(); ++iter) {
		if ((*iter).second == joint) {
			result = true;
			break;
		}
	}

	return result;
}
