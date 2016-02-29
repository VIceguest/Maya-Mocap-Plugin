#include <maya/MStatus.h>
#include "idebug.h"

#define MCHECKERROR(STAT,MSG)       \
    if ( MS::kSuccess != STAT ) {   \
        cerr << MSG << endl;        \
            return MS::kFailure;    \
    }

#define MCHECKERRORNORET(STAT,MSG)  \
    if ( MS::kSuccess != STAT ) {   \
        cerr << MSG << endl;        \
    }

#define MAKE_TYPED_ATTR( NAME, LONGNAME, SHORTNAME, TYPE, DEFAULT )         \
										                                    \
	MStatus NAME##_stat;                                                    \
	MFnTypedAttribute NAME##_fn;                                            \
	NAME = NAME##_fn.create( LONGNAME, SHORTNAME, TYPE, DEFAULT );          \
	NAME##_fn.setHidden( true );											\
	NAME##_stat = addAttribute( NAME );                                     \
	MCHECKERROR(NAME##_stat, "addAttribute error");

#define MAKE_NUMERIC_ATTR( NAME, LONGNAME, SHORTNAME, TYPE, DEFAULT,        \
							ARRAY, BUILDER, KEYABLE )                       \
										                                    \
	MStatus NAME##_stat;                                                    \
	MFnNumericAttribute NAME##_fn;                                          \
	NAME = NAME##_fn.create( LONGNAME, SHORTNAME, TYPE, DEFAULT );          \
	MCHECKERROR(NAME##_stat, "numeric attr create error");		            \
    NAME##_fn.setArray( ARRAY );                                            \
    NAME##_fn.setUsesArrayDataBuilder( BUILDER );                           \
	NAME##_fn.setHidden( ARRAY );											\
	NAME##_fn.setKeyable( KEYABLE );										\
	NAME##_stat = addAttribute( NAME );                                     \
	MCHECKERROR(NAME##_stat, "addAttribute error");

#define ADD_ATTRIBUTE( ATTR )                                               \
	MStatus ATTR##_stat;                                                    \
	ATTR##_stat = addAttribute( ATTR );                                     \
    MCHECKERROR( ATTR##_stat, "addAttribute: ATTR" )

#define ATTRIBUTE_AFFECTS( IN, OUT )                                        \
	MStatus IN##OUT##_stat;                                                 \
	IN##OUT##_stat = attributeAffects( IN, OUT );                           \
	MCHECKERROR(IN##OUT##_stat,"attributeAffects:" #IN "->" #OUT);

#define MS_STATUS		_mstatus

#define MS_ENTRANCE		MStatus _mstatus = MStatus::kSuccess; do {

#define MS_CHECK(x)		{\
	_mstatus = (x);\
	if (_mstatus.error()) {\
	ILOG4 ("MAYA_API ERROR '"#x << "' : " << _mstatus);\
	break;\
	}\
						}

#define MS_CHECK_RELAY	if (_mstatus.error()) break;

#define MS_EXIT			} while(false);

#define MS_STORE(x)		x = _mstatus;

#define MS_RETURN		return _mstatus;
