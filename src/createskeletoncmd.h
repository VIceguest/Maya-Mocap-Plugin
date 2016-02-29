#include <maya/MIOStream.h>
#include <maya/MPxCommand.h>

class CreateNueronSkeletonCmd: public MPxCommand
{
public:
	CreateNueronSkeletonCmd();
	virtual ~CreateNueronSkeletonCmd();

	virtual MStatus doIt ( const MArgList& args );

	static void* creator();
private:

};
