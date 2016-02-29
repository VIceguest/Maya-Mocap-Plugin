#include "imocapimport.h"
#include "idebug.h"
#include "netdevicenode.h"
#include "createskeletoncmd.h"
#include <maya/MFnPlugin.h>

const char *IM_IMPORTER_NAME = "BVH";
const char *IM_VENDOR_NAME = "TEST";

//const char *const imocapImportOptionScript = "imocapImportOptions";
const char *const imocapImportDefaultOptions = 
	"bonesOnly=false;merge=false;scale=1;rotationOrder=none;startFrame=0;endFrame=2147483648";

//-----------------------------------------------------------------------------
// Initialize Plug-in
//-----------------------------------------------------------------------------
MStatus initializePlugin(MObject obj)
{
	MStatus stat = MS::kFailure;

	MFnPlugin mocapPlugIn(obj, IM_VENDOR_NAME, "0.1.0");
	stat = mocapPlugIn.registerFileTranslator(IM_IMPORTER_NAME, "none",
		imocapImport::creator,
		(char*)0,
		(char *)imocapImportDefaultOptions, 
		false);

	stat = mocapPlugIn.registerCommand("createNeuronSkeleton",
		CreateNueronSkeletonCmd::creator);

	stat = mocapPlugIn.registerNode( "netDevice", 
		NetDeviceNode::id,
		NetDeviceNode::creator,
		NetDeviceNode::initialize,
		MPxNode::kThreadedDeviceNode );

	stat = mocapPlugIn.registerUI("createmocapmenu", "deletemocapmenu");

	ILOG2("Plug-in was loaded.");

	return stat;
}

//-----------------------------------------------------------------------------
// Uninitialize Plug-in
//-----------------------------------------------------------------------------
MStatus uninitializePlugin(MObject obj)
{
	MStatus stat = MS::kFailure;

	MFnPlugin mocapPlugIn(obj);
	stat = mocapPlugIn.deregisterNode( NetDeviceNode::id );

	stat = mocapPlugIn.deregisterCommand( "createNueronSkeleton" );

	stat = mocapPlugIn.deregisterFileTranslator(IM_IMPORTER_NAME);

	ILOG2("Plug-in was unloaded.");

	return stat;
}
