/*
   FALCON - The Falcon Programming Language.
   FILE: len.cpp

   Falcon core module -- len function/method
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 04 Jun 2011 20:52:06 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#include <falcon/cm/len.h>
#include <falcon/vm.h>
#include <falcon/vmcontext.h>
#include <falcon/itemid.h>
#include <falcon/error.h>

namespace Falcon {
namespace Ext {

Len::Len():
   PseudoFunction( "len", &m_invoke )
{
   setDeterm(true);
   signature("X");
   addParam("item");
}

Len::~Len()
{
}

void Len::invoke( VMContext* ctx, int32 nParams )
{
   Item *elem;
   register length_t len;
   if ( ctx->isMethodic() )
   {
      elem = &ctx->self();
      len = elem->len();
   }
   else
   {
      if( nParams <= 0 )
      {
         throw paramError();
      }

      elem = ctx->params();
      len = elem->len();
   }

   ctx->returnFrame((int64) len);
}

void Len::Invoke::apply_( const PStep*, VMContext* ctx )
{
   register Item& top = ctx->topData();
   top = top.len();
}

}
}

/* end of len.cpp */