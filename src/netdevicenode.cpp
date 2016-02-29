#include "netdevicenode.h"

using namespace std;
#include "macros.h"
#include <maya/MStringArray.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MVector.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>

#define RECVBUFSIZE  3072
#define NOSHIFTATTRNUM (JOINT_NUM*3+2)
#define SHIFTNOREFATTRNUM ((JOINT_NUM-1)*6+2)
#define SHIFTREFATTRNUM  (JOINT_NUM*6+2)

MTypeId NetDeviceNode::id( 0x00081053 );

MObject	NetDeviceNode::	outputRotateOrder;
MObject NetDeviceNode::outputTranslate[MAX_ROBOT_NUM][JOINT_NUM];
MObject NetDeviceNode::outputTranslateX[MAX_ROBOT_NUM][JOINT_NUM];
MObject NetDeviceNode::outputTranslateY[MAX_ROBOT_NUM][JOINT_NUM];
MObject NetDeviceNode::outputTranslateZ[MAX_ROBOT_NUM][JOINT_NUM];

MObject NetDeviceNode::outputRotate[MAX_ROBOT_NUM][JOINT_NUM];
MObject NetDeviceNode::outputRotateX[MAX_ROBOT_NUM][JOINT_NUM];
MObject NetDeviceNode::outputRotateY[MAX_ROBOT_NUM][JOINT_NUM];
MObject NetDeviceNode::outputRotateZ[MAX_ROBOT_NUM][JOINT_NUM];

MObject NetDeviceNode::inputIP;
MObject NetDeviceNode::inputPort;

NetDeviceNode::NetDeviceNode():m_sClient(0),
	m_strIP("127.0.0.1"),
	m_strPort("7001")
{}

NetDeviceNode::~NetDeviceNode()
{
	if(m_sClient!=0)
	{
		closesocket(m_sClient);
		WSACleanup();
	}
	destroyMemoryPools();
}

void NetDeviceNode::postConstructor()
{
	MObjectArray attrArray;
	attrArray.append( NetDeviceNode::outputRotateOrder);

	setRefreshOutputAttributes( attrArray );
	createMemoryPools( 1, RECVBUFSIZE, sizeof(char));
}

void NetDeviceNode::threadHandler()
{
	setDone( false );

	MPlug plugIP(thisMObject(),inputIP);
	m_strIP=plugIP.asString();

	MPlug plugPort(thisMObject(),inputPort);
	m_strPort=plugPort.asString();

	MStatus status;
	MCharBuffer buffer;
	char recvBuf[RECVBUFSIZE];
	char recvChar[1];
	int  nBytesRead;

	bool bConnectionCreated=false;

	while ( ! isDone() )
	{
		if ( ! isLive() )
		{
			if(bConnectionCreated)
			{
				closesocket(m_sClient);
				WSACleanup();
			}
			//Sleep(100);
			continue;
		}

		if(!bConnectionCreated)
		{
			unsigned long nonblock = 1;
			int nRet;
			struct sockaddr_in saServer; 
			WSADATA wsa;

			if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
				return;
			m_sClient=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //TCP
			nRet = ioctlsocket(m_sClient, FIONBIO, &nonblock);//non block
			if (nRet == SOCKET_ERROR)
			{
				closesocket(m_sClient);
				//Sleep(100);
				continue;
			}

			saServer.sin_family = AF_INET; 
			saServer.sin_port = htons(m_strPort.asInt()); 
			saServer.sin_addr.S_un.S_addr = inet_addr(m_strIP.asChar());

			/*nRet =*/  connect(m_sClient, (struct sockaddr *)&saServer, sizeof(saServer));
			// 	if (nRet==SOCKET_ERROR)
			// 		return;
			fd_set r_set;  
			FD_ZERO(&r_set);  
			FD_SET(m_sClient,&r_set);     
			timeval timeout;  
			timeout.tv_sec = 3;  
			timeout.tv_usec = 0;  
			if(select(0, &r_set,&r_set,NULL,&timeout)<=0)  
			{  
				closesocket(m_sClient);
				continue;
			}  
			bConnectionCreated=true;
		}

		memset(recvBuf,0,RECVBUFSIZE);
		nBytesRead=-1;
		while (recv(m_sClient, recvChar, 1, 0)==1)
		{
			if(recvChar[0]=='|')
			{
				recv(m_sClient, recvChar, 1, 0);
				break;
			}
			nBytesRead++;
			if(nBytesRead==RECVBUFSIZE)
				break;
			recvBuf[nBytesRead]=recvChar[0];
		}
		if(nBytesRead==-1)
			continue;
		if(nBytesRead==RECVBUFSIZE)
			continue;
		recvBuf[++nBytesRead]='\0';

		status = acquireDataStorage(buffer);
		if ( ! status )
			continue;

		beginThreadLoop();
		{			
			memcpy(buffer.ptr(),recvBuf,++nBytesRead);
			pushThreadData( buffer );
		}
		endThreadLoop();
	}

	closesocket(m_sClient);
	WSACleanup();

	setDone( true );
}

void NetDeviceNode::threadShutdownHandler()
{
	setDone( true );
}

void* NetDeviceNode::creator()
{
	return new NetDeviceNode;
}


MStatus NetDeviceNode::initialize()
{
	MStatus status;
	 
	MFnTypedAttribute typedAttr;
	MFnStringData     fnStringData;
	MObject           defaultString;
	defaultString = fnStringData.create( "127.0.0.1" ); 
	inputIP=typedAttr.create("inputIp", "ip",MFnData::kString,defaultString, &status);
	MCHECKERROR(status, "create inputip");

	defaultString = fnStringData.create( "7001" ); 
	inputPort=typedAttr.create("inputPort", "port", MFnData::kString,defaultString,&status);
	MCHECKERROR(status, "create inputport");

	//typedAttr.setHidden(true);

	addAttribute(inputIP);
// 	attributeAffects( live, inputIP);
// 	attributeAffects( frameRate, inputIP);
	addAttribute(inputPort);
 //	attributeAffects( live, inputPort);
// 	attributeAffects( frameRate, inputPort);
	
	MFnNumericAttribute numAttr;
	
	for(int j=0;j<MAX_ROBOT_NUM;j++)
	for(int i=0;i<JOINT_NUM;i++)
	{
		MString strTx="outputTranslateX";
		strTx+=j;
		strTx+=i;
		MString strBriefTx="otx";
		strBriefTx+=j;
		strBriefTx+=i;
		outputTranslateX[j][i] = numAttr.create(strTx, strBriefTx, MFnNumericData::kDouble, 0.0, &status);
		MCHECKERROR(status, "create outputTranslateX");

		MString strTy="outputTranslateY";
		strTy+=j;
		strTy+=i;
		MString strBriefTy="oty";
		strBriefTy+=j;
		strBriefTy+=i;
		outputTranslateY[j][i] = numAttr.create(strTy, strBriefTy, MFnNumericData::kDouble, 0.0, &status);
		MCHECKERROR(status, "create outputTranslateY");

		MString strTz="outputTranslateZ";
		strTz+=j;
		strTz+=i;
		MString strBriefTz="otz";
		strBriefTz+=j;
		strBriefTz+=i;
		outputTranslateZ[j][i] = numAttr.create(strTz, strBriefTz, MFnNumericData::kDouble, 0.0, &status);
		MCHECKERROR(status, "create outputTranslateZ");

		MString strT="outputTranslate";
		strT+=j;
		strT+=i;
		MString strBriefT="ot";
		strBriefT+=j;
		strBriefT+=i;
		outputTranslate[j][i] = numAttr.create(strT, strBriefT, outputTranslateX[j][i], outputTranslateY[j][i], 
			outputTranslateZ[j][i], &status);
		MCHECKERROR(status, "create outputTranslate");
		
		addAttribute(outputTranslate[j][i]);
		attributeAffects( live, outputTranslate[j][i]);
		attributeAffects( frameRate,  outputTranslate[j][i]);

		MString strRx="outputRotateX";
		strRx+=j;
		strRx+=i;
		MString strBriefRx="orx";
		strBriefRx+=j;
		strBriefRx+=i;
		outputRotateX[j][i] = numAttr.create(strRx, strBriefRx, MFnNumericData::kDouble, 0.0, &status);
		MCHECKERROR(status, "create outputRotateX");

		MString strRy="outputRotateY";
		strRy+=j;
		strRy+=i;
		MString strBriefRy="ory";
		strBriefRy+=j;
		strBriefRy+=i;
		outputRotateY[j][i] = numAttr.create(strRy, strBriefRy, MFnNumericData::kDouble, 0.0, &status);
		MCHECKERROR(status, "create outputRotateY");

		MString strRz="outputRotateZ";
		strRz+=j;
		strRz+=i;
		MString strBriefRz="orz";
		strBriefRz+=j;
		strBriefRz+=i;
		outputRotateZ[j][i] = numAttr.create(strRz, strBriefRz, MFnNumericData::kDouble, 0.0, &status);
		MCHECKERROR(status, "create outputRotateZ");

		MString strR="outputRotate";
		strR+=j;
		strR+=i;
		MString strBriefR="or";
		strBriefR+=j;
		strBriefR+=i;
		outputRotate[j][i] = numAttr.create(strR, strBriefR, outputRotateX[j][i], outputRotateY[j][i], 
			outputRotateZ[j][i], &status);
		MCHECKERROR(status, "create outputRotate");

		addAttribute(outputRotate[j][i]);
		attributeAffects( live, outputRotate[j][i]);
		attributeAffects( frameRate, outputRotate[j][i]);
	}

	defaultString = fnStringData.create( "yxz" ); 
	outputRotateOrder=typedAttr.create("outputRotateOrder", "oro", MFnData::kString,defaultString,&status);
	addAttribute(outputRotateOrder); 
	attributeAffects( live, outputRotateOrder);
	attributeAffects( frameRate, outputRotateOrder);

	for(int j=0;j<MAX_ROBOT_NUM;j++)
	for(int i=0;i<JOINT_NUM;i++){
		attributeAffects (outputRotateOrder, outputTranslate[j][i]) ; // Workaround to setRefreshOutputAttributes() limitation
		attributeAffects (outputRotateOrder, outputRotate[j][i]) ;

		attributeAffects (inputIP, outputTranslate[j][i]) ;
		attributeAffects (inputIP, outputRotate[j][i]) ;
		
		attributeAffects (inputPort, outputTranslate[j][i]) ;
		attributeAffects (inputPort, outputRotate[j][i]) ;
	}

	return MS::kSuccess;
}

MStatus NetDeviceNode::compute( const MPlug& plug, MDataBlock& block )
{
	MStatus status;
	BOOL bRotateTranslate=FALSE;
	for(int j=0;j<MAX_ROBOT_NUM;j++)
	for(int i=0;i<JOINT_NUM;i++)
	{   
		if( plug == outputRotate[j][i] || plug == outputRotateX[j][i] ||
			plug == outputRotateY[j][i] || plug == outputRotateZ[j][i]||
			plug == outputTranslate[j][i] || plug == outputTranslateX[j][i] ||
			plug == outputTranslateY[j][i] || plug == outputTranslateZ[j][i] )
		{
			bRotateTranslate=TRUE;break;
		}
	}

	if( plug == outputRotateOrder ||plug == inputIP ||plug == inputPort ||bRotateTranslate)
	{
		MCharBuffer buffer;
		if ( popThreadData(buffer) )
		{	
			MString s(buffer.ptr());
			MStringArray sa;
			s.split( ' ', sa ) ;

			int nIndex=sa[0].asInt();

			if(nIndex>MAX_ROBOT_NUM)
				return ( MS::kFailure );

			if(sa.length()==NOSHIFTATTRNUM)
			{
				MDataHandle outputTranslateHandle = block.outputValue( outputTranslate[nIndex][1], &status );
				MCHECKERROR(status, "Error in block.outputValue for outputTranslate");

				double3& outputTranslate = outputTranslateHandle.asDouble3();
				outputTranslate[0] = sa[2].asDouble();
				outputTranslate[1] = sa[3].asDouble();
				outputTranslate[2] = sa[4].asDouble();

				outputTranslateHandle.setClean();

				for(int i=1;i<JOINT_NUM;i++)
				{   
					MDataHandle outputRotateHandle = block.outputValue( outputRotate[nIndex][i], &status );
					MCHECKERROR(status, "Error in block.outputValue for outputRotate");
						
					double3& outputRotate = outputRotateHandle.asDouble3();

					MDataHandle outputRotateOrderHandle = block.inputValue( outputRotateOrder, &status );
					MString strOrder=outputRotateOrderHandle.asString();
					if(strOrder=="yxz"){
						outputRotate[0] = sa[6+(i-1)*3].asDouble();
						outputRotate[1] = sa[5+(i-1)*3].asDouble();
						outputRotate[2] = sa[7+(i-1)*3].asDouble();
					}else if(strOrder=="xyz"){
						outputRotate[0] = sa[5+(i-1)*3].asDouble();
						outputRotate[1] = sa[6+(i-1)*3].asDouble();
						outputRotate[2] = sa[7+(i-1)*3].asDouble();
					}else if(strOrder=="yzx"){
						outputRotate[0] = sa[7+(i-1)*3].asDouble();
						outputRotate[1] = sa[5+(i-1)*3].asDouble();
						outputRotate[2] = sa[6+(i-1)*3].asDouble();
					}else if(strOrder=="zxy"){
						outputRotate[0] = sa[6+(i-1)*3].asDouble();
						outputRotate[1] = sa[7+(i-1)*3].asDouble();
						outputRotate[2] = sa[5+(i-1)*3].asDouble();
					}else if(strOrder=="xzy"){
						outputRotate[0] = sa[5+(i-1)*3].asDouble();
						outputRotate[1] = sa[7+(i-1)*3].asDouble();
						outputRotate[2] = sa[6+(i-1)*3].asDouble();
					}else if(strOrder=="zyx"){
						outputRotate[0] = sa[7+(i-1)*3].asDouble();
						outputRotate[1] = sa[6+(i-1)*3].asDouble();
						outputRotate[2] = sa[5+(i-1)*3].asDouble();
					}
					outputRotateHandle.setClean();
				}
			}
			else if(sa.length()==SHIFTNOREFATTRNUM)
			{
				for(int i=1;i<JOINT_NUM;i++)
				{
					MDataHandle outputTranslateHandle = block.outputValue( outputTranslate[nIndex][i], &status );
					MCHECKERROR(status, "Error in block.outputValue for outputTranslate");

					double3& outputTranslate = outputTranslateHandle.asDouble3();
					outputTranslate[0] = sa[2+(i-1)*6].asDouble();
					outputTranslate[1] = sa[3+(i-1)*6].asDouble();
					outputTranslate[2] = sa[4+(i-1)*6].asDouble();

					outputTranslateHandle.setClean();

					MDataHandle outputRotateHandle = block.outputValue( outputRotate[nIndex][i], &status );
					MCHECKERROR(status, "Error in block.outputValue for outputRotate");

					double3& outputRotate = outputRotateHandle.asDouble3();

					MDataHandle outputRotateOrderHandle = block.inputValue( outputRotateOrder, &status );
					MString strOrder=outputRotateOrderHandle.asString();
					if(strOrder=="yxz"){
						outputRotate[0] = sa[6+(i-1)*6].asDouble();
						outputRotate[1] = sa[5+(i-1)*6].asDouble();
						outputRotate[2] = sa[7+(i-1)*6].asDouble();
					}else if(strOrder=="xyz"){
						outputRotate[0] = sa[5+(i-1)*6].asDouble();
						outputRotate[1] = sa[6+(i-1)*6].asDouble();
						outputRotate[2] = sa[7+(i-1)*6].asDouble();
					}else if(strOrder=="yzx"){
						outputRotate[0] = sa[7+(i-1)*6].asDouble();
						outputRotate[1] = sa[5+(i-1)*6].asDouble();
						outputRotate[2] = sa[6+(i-1)*6].asDouble();
					}else if(strOrder=="zxy"){
						outputRotate[0] = sa[6+(i-1)*6].asDouble();
						outputRotate[1] = sa[7+(i-1)*6].asDouble();
						outputRotate[2] = sa[5+(i-1)*6].asDouble();
					}else if(strOrder=="xzy"){
						outputRotate[0] = sa[5+(i-1)*6].asDouble();
						outputRotate[1] = sa[7+(i-1)*6].asDouble();
						outputRotate[2] = sa[6+(i-1)*6].asDouble();
					}else if(strOrder=="zyx"){
						outputRotate[0] = sa[7+(i-1)*6].asDouble();
						outputRotate[1] = sa[6+(i-1)*6].asDouble();
						outputRotate[2] = sa[5+(i-1)*6].asDouble();
					}
					outputRotateHandle.setClean();
				}
			}
			else if(sa.length()==SHIFTREFATTRNUM)
			{
				for(int i=0;i<JOINT_NUM;i++)
				{
					MDataHandle outputTranslateHandle = block.outputValue( outputTranslate[nIndex][i], &status );
					MCHECKERROR(status, "Error in block.outputValue for outputTranslate");

					double3& outputTranslate = outputTranslateHandle.asDouble3();
					outputTranslate[0] = sa[2+i*6].asDouble();
					outputTranslate[1] = sa[3+i*6].asDouble();
					outputTranslate[2] = sa[4+i*6].asDouble();

					outputTranslateHandle.setClean();

					MDataHandle outputRotateHandle = block.outputValue( outputRotate[nIndex][i], &status );
					MCHECKERROR(status, "Error in block.outputValue for outputRotate");

					double3& outputRotate = outputRotateHandle.asDouble3();

					MDataHandle outputRotateOrderHandle = block.inputValue( outputRotateOrder, &status );
					MString strOrder=outputRotateOrderHandle.asString();
					if(strOrder=="yxz"){
						outputRotate[0] = sa[6+i*6].asDouble();
						outputRotate[1] = sa[5+i*6].asDouble();
						outputRotate[2] = sa[7+i*6].asDouble();
					}else if(strOrder=="xyz"){
						outputRotate[0] = sa[5+i*6].asDouble();
						outputRotate[1] = sa[6+i*6].asDouble();
						outputRotate[2] = sa[7+i*6].asDouble();
					}else if(strOrder=="yzx"){
						outputRotate[0] = sa[7+i*6].asDouble();
						outputRotate[1] = sa[5+i*6].asDouble();
						outputRotate[2] = sa[6+i*6].asDouble();
					}else if(strOrder=="zxy"){
						outputRotate[0] = sa[6+i*6].asDouble();
						outputRotate[1] = sa[7+i*6].asDouble();
						outputRotate[2] = sa[5+i*6].asDouble();
					}else if(strOrder=="xzy"){
						outputRotate[0] = sa[5+i*6].asDouble();
						outputRotate[1] = sa[7+i*6].asDouble();
						outputRotate[2] = sa[6+i*6].asDouble();
					}else if(strOrder=="zyx"){
						outputRotate[0] = sa[7+i*6].asDouble();
						outputRotate[1] = sa[6+i*6].asDouble();
						outputRotate[2] = sa[5+i*6].asDouble();
					}
					outputRotateHandle.setClean();
				}
			}

			block.setClean( plug );
			releaseDataStorage(buffer);
			return ( MS::kSuccess );
		}
		else
		{
			return ( MS::kFailure );
		}
	}
	return ( MS::kFailure );
}
