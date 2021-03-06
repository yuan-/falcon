
/**
 * \file
 *
 * ZLib module main file - extension definitions
 */

#ifndef FLC_ZLIB_EXT_H
#define FLC_ZLIB_EXT_H

#include <falcon/setup.h>
#include <falcon/module.h>
#include <falcon/error.h>
#include <falcon/error_base.h>

#ifndef FALCON_ZLIB_ERROR_BASE
   #define FALCON_ZLIB_ERROR_BASE        1190
#endif


namespace Falcon {
namespace Ext {

FALCON_FUNC ZLib_getVersion( ::Falcon::VMachine *vm );
FALCON_FUNC ZLib_compress( ::Falcon::VMachine *vm );
FALCON_FUNC ZLib_uncompress( ::Falcon::VMachine *vm );
FALCON_FUNC ZLib_compressText( ::Falcon::VMachine *vm );
FALCON_FUNC ZLib_uncompressText( ::Falcon::VMachine *vm );

class ZLibError: public ::Falcon::Error
{
public:
   ZLibError():
      Error( "ZLibError" )
   {}

   ZLibError( const ErrorParam &params  ):
      Error( "ZLibError", params )
      {}
};

FALCON_FUNC  ZLibError_init ( ::Falcon::VMachine *vm );

}
}

#endif /* FLC_ZLIB_EXT_H */
