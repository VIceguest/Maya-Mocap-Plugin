#include "createskeletoncmd.h"
#include "imocapimport.h"
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <stdio.h>
#include <io.h>
#include <shlobj.h> 
#include <sstream>
#include <iostream>
#include <string>

void* CreateNueronSkeletonCmd::creator()
{
	return new CreateNueronSkeletonCmd();
}

MStatus CreateNueronSkeletonCmd::doIt( const MArgList& args )
{
	imocapImport mcImport;
	
	char docDir[_MAX_PATH];  
	SHGetSpecialFolderPath(NULL,docDir,CSIDL_PERSONAL,0);
	
	MString filename(docDir);
	filename+="\\maya\\BVH_Headers\\BVH_Header_";
	filename+=args.asString(1);
	filename+=args.asString(2);
	filename+=args.asString(3);
	filename+=".bvh";
	mcImport.myNamespace =filename;
	if (_access(filename.asChar(),0) )
	{
		MString errinfo("File ");
		errinfo+=filename;
		errinfo+=" not exist!";
		MGlobal::displayError(errinfo);
		return MS::kFailure;
	}
	int pos = mcImport.myNamespace.rindex('.');
	if (pos == -1) pos = mcImport.myNamespace.length();
        --pos;
	mcImport.myNamespace = mcImport.myNamespace.substring(0, pos);

	int nCount = args.asInt(0);
	for ( int i=0;i < nCount;i++)
	{
		std::ostringstream oss;
		oss<<i;
		std::string strJointNameSuffix=oss.str();
		mcImport.importMocapFile(filename,false,strJointNameSuffix);
	}
	return MS::kSuccess;
}

CreateNueronSkeletonCmd::CreateNueronSkeletonCmd()
{

}

CreateNueronSkeletonCmd::~CreateNueronSkeletonCmd()
{

}
