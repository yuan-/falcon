/*
   FALCON - The Falcon Programming Language.
   FILE: stmtselect.cpp

   Syntactic tree item definitions -- select statement.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Mon, 15 Aug 2011 13:58:12 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#undef SRC
#define SRC "engine/psteps/select.cpp"

#include <falcon/expression.h>
#include <falcon/engine.h>
#include <falcon/vmcontext.h>
#include <falcon/syntree.h>
#include <falcon/symbol.h>
#include <falcon/module.h>
#include <falcon/stdhandlers.h>
#include <falcon/textwriter.h>

#include <falcon/errors/linkerror.h>

#include <falcon/psteps/stmtselect.h>

#include <falcon/engine.h>
#include <falcon/synclasses.h>
#include <falcon/itemarray.h>
#include <falcon/vmcontext.h>

#include <falcon/datareader.h>
#include <falcon/datawriter.h>

#include <map>
#include <deque>
#include <vector>

#include <falcon/psteps/switchlike.h>

namespace Falcon {

StmtSelect::StmtSelect( int32 line, int32 chr ):
   SwitchlikeStatement( line, chr )
{
   FALCON_DECLARE_SYN_CLASS( stmt_select );
   apply = apply_;
}

StmtSelect::StmtSelect( const StmtSelect& other ):
   SwitchlikeStatement( other )
{
}

StmtSelect::~StmtSelect()
{
}


void StmtSelect::renderHeader( TextWriter* tw, int32 depth ) const
{
   tw->write( renderPrefix(depth) );
   tw->write( "select " );
   selector()->render( tw, relativeDepth(depth) );
   tw->write("\n");
}


void StmtSelect::apply_( const PStep* ps, VMContext* ctx )
{
   const StmtSelect* self = static_cast<const StmtSelect*>(ps);

   CodeFrame& cf = ctx->currentCode();
   // first time around? -- call the expression.
   if( cf.m_seqId == 0 )
   {
      cf.m_seqId = 1;
      if( ctx->stepInYield( self->m_expr, cf ) )
      {
         return;
      }
   }

   SynTree* res = self->findBlockForType( ctx->topData(), ctx );

   // we're gone
   ctx->popCode();
   // and so is the topdata.
   ctx->popData();

   // but if the syntree wants to do something...
   if( res != 0 )
   {
      ctx->pushCode( res );
   }
}


}

/* end of stmtselect.cpp */