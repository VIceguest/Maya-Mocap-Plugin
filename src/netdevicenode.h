#pragma once

#include <maya/MIOStream.h>
#include <maya/MTypeId.h>
#include <maya/MPxThreadedDeviceNode.h>
#include <winsock2.h>
#include <maya/MString.h>

#define  MAX_ROBOT_NUM 5
#define  JOINT_NUM   60

class NetDeviceNode : public MPxThreadedDeviceNode
{

public:
	NetDeviceNode();
	virtual 			~NetDeviceNode();

	virtual void		postConstructor();
	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );
	virtual void		threadHandler();
	virtual void		threadShutdownHandler();

	static void*		creator();
	static MStatus		initialize();

public:
	static MTypeId		id;
	
	//当前maya sdk只能输出一个属性,so...
	static MObject		outputRotateOrder;

	//每个节点位移
	static MObject		outputTranslate[MAX_ROBOT_NUM][JOINT_NUM];
	
	static MObject 		outputTranslateX[MAX_ROBOT_NUM][JOINT_NUM];
	static MObject		outputTranslateY[MAX_ROBOT_NUM][JOINT_NUM];
	static MObject 		outputTranslateZ[MAX_ROBOT_NUM][JOINT_NUM];

	//每个节点的旋转
	static MObject		outputRotate[MAX_ROBOT_NUM][JOINT_NUM];

	static MObject 		outputRotateX[MAX_ROBOT_NUM][JOINT_NUM];
	static MObject		outputRotateY[MAX_ROBOT_NUM][JOINT_NUM];
	static MObject 		outputRotateZ[MAX_ROBOT_NUM][JOINT_NUM];

	//IP PORT
	static MObject		inputIP;
	static MObject 		inputPort;
	
private:
	SOCKET				m_sClient;
	MString				m_strIP;
	MString				m_strPort;
};
