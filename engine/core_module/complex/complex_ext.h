/*
   FALCON - The Falcon Programming Language.
   FILE: complex_ext.cpp

   Complex class for Falcon
   Interface extension functions
   -------------------------------------------------------------------
   Author: Enrico Lumetti
   Begin: Sat, 05 Sep 2009 21:04:31 +0000

   -------------------------------------------------------------------
   (C) Copyright 2009: The above AUTHOR

         Licensed under the Falcon Programming Language License,
      Version 1.1 (the "License"); you may not use this file
      except in compliance with the License. You may obtain
      a copy of the License at

         http://www.falconpl.org/?page_id=license_1_1

      Unless required by applicable law or agreed to in writing,
      software distributed under the License is distributed on
      an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
      KIND, either express or implied. See the License for the
      specific language governing permissions and limitations
      under the License.

*/

/** \file
   Complex class for Falcon
   Interface extension functions - header file
*/

#ifndef complex_ext_H
#define complex_ext_H

#include <falcon/module.h>
#include "complex_mod.h"

namespace Falcon {
namespace Ext {

FALCON_FUNC Complex_init( ::Falcon::VMachine *vm );
FALCON_FUNC Complex_toString( ::Falcon::VMachine *vm );
FALCON_FUNC Complex_abs( ::Falcon::VMachine *vm );
FALCON_FUNC Complex_add__( ::Falcon::VMachine *vm );
FALCON_FUNC Complex_mul__( ::Falcon::VMachine *vm );

}
}

#endif

/* end of complex_ext.h */