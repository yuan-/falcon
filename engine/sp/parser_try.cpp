/*
   FALCON - The Falcon Programming Language.
   FILE: parser_try.cpp

   Parser for Falcon source files -- try statement handler
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 03 Jul 2011 18:13:22 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#undef SRC
#define SRC "engine/sp/parser_try.cpp"

#include <falcon/setup.h>
#include <falcon/trace.h>

#include <falcon/error.h>
#include <falcon/statement.h>

#include <falcon/parser/rule.h>
#include <falcon/parser/parser.h>

#include <falcon/sp/sourceparser.h>
#include <falcon/sp/parsercontext.h>
#include <falcon/sp/parser_deletor.h>
#include <falcon/sp/parser_try.h>

#include <falcon/psteps/stmttry.h>
#include <falcon/psteps/stmtraise.h>
#include <falcon/psteps/exprvalue.h>

#include <falcon/synclasses_id.h>
#include <falcon/symbol.h>

#include "private_caselist.h"

namespace Falcon {

using namespace Parsing;

bool try_errhand(const NonTerminal&, Parser& p)
{
   TokenInstance* ti = p.getNextToken();
   TokenInstance* ti2 = p.getLastToken();

   if( p.lastErrorLine() < ti->line() )
   {
      // generate another error only if we didn't notice it already.
      p.addError( e_syn_try, p.currentSource(), ti2->line(), ti2->chr() );
   }

   if( ! p.interactive() )
   {
      // put in a fake  statement (else, subsequent catch would also cause errors).
      StmtTry* fakeTry = new StmtTry( ti->line(), ti->chr() );
      ParserContext* st = static_cast<ParserContext*>(p.context());
      st->openBlock( fakeTry, fakeTry->body()  );
   }
   else
   {
      // on interactive parsers, let the whole instruction to be destroyed.
      MESSAGE2( "errhand_try -- Ignoring TRY in interactive mode." );
   }
   p.consumeUpTo( p.T_EOL );
   p.clearFrames();
   return true;
}


bool catch_errhand(const NonTerminal&, Parser& p)
{
   TokenInstance* ti = p.getNextToken();
   //TokenInstance* ti2 = p.getLastToken();

   if( p.lastErrorLine() < ti->line() )
   {
      // generate another error only if we didn't notice it already.
      p.addError( e_syn_catch, p.currentSource(), ti->line(), ti->chr() );
   }
   else {
      // we can ignore failing catches.
      MESSAGE2( "catch_errhand -- Ignoring CATCH." );
   }
   
   p.consumeUpTo( p.T_EOL );
   p.clearFrames();
   return true;
}


bool finally_errhand(const NonTerminal&, Parser& p)
{
   TokenInstance* ti = p.getNextToken();
   TokenInstance* ti2 = p.getLastToken();

   if( p.lastErrorLine() < ti->line() )
   {
      // generate another error only if we didn't notice it already.
      p.addError( e_syn_finally, p.currentSource(), ti2->line(), ti2->chr() );
   }
   else {
      // we can ignore failing catches.
      MESSAGE2( "finally_errhand -- Ignoring FINALLY." );
   }
   
   p.consumeUpTo( p.T_EOL );
   p.clearFrames();
   return true;
}

bool raise_errhand(const NonTerminal&, Parser& p)
{
   TokenInstance* ti = p.getNextToken();
   TokenInstance* ti2 = p.getLastToken();

   if( p.lastErrorLine() < ti->line() )
   {
      // generate another error only if we didn't notice it already.
      p.addError( e_syn_raise, p.currentSource(), ti2->line(), ti2->chr() );
   }
   else {
      // we can ignore failing catches.
      MESSAGE2( "raise_errhand -- Ignoring RAISE." );
   }
   
   p.consumeUpTo( p.T_EOL );
   p.clearFrames();
   return true;
}


void apply_try( const Rule&, Parser& p )
{
   // T_try << T_EOL   
   TokenInstance* ti = p.getNextToken();   
   StmtTry* stmt = new StmtTry( ti->line(), ti->chr() );
   ParserContext* ctx = static_cast<ParserContext*>(p.context());
   ctx->openBlock( stmt, stmt->body() );
   p.simplify( 2 );
}


void apply_catch( const Rule&, Parser& p )
{
   // T_catch << CatchSpec
   // all the work is down below.
   p.simplify(2);
}

void apply_finally( const Rule&, Parser& p )
{
   // T_finally << T_EOL
   TokenInstance* ti = p.getNextToken();
   
   ParserContext* ctx = static_cast<ParserContext*>(p.context());
   TreeStep* stmt = ctx->currentStmt();
   if( stmt == 0 || stmt->handler()->userFlags() != FALCON_SYNCLASS_ID_CATCHHOST )
   {
      p.addError( e_finally_outside, p.currentSource(), ti->line(), ti->chr() );
   }
   else
   {
      StmtTry* stmttry = static_cast<StmtTry*>(stmt);
      if( stmttry->fbody() != 0 )
      {
         p.addError( e_finally_adef, p.currentSource(), ti->line(), ti->chr() );
      }
      else
      {
         stmttry->fbody( ctx->changeBranch() );
      }      
   }
   
   p.simplify(2);
}

void apply_raise( const Rule&, Parser& p )
{
   //<< T_raise << Expr << T_EOL );
   ParserContext* ctx = static_cast<ParserContext*>(p.context());
   
   TokenInstance* ti = p.getNextToken();
   Expression* expr = static_cast<Expression*>(p.getNextToken()->detachValue());
   StmtRaise* raise = new StmtRaise( expr, ti->line(), ti->chr() );
   ctx->addStatement( raise );
   
   p.simplify(3);
}

//=======================================================
// Catch clauses 
//   


static void internal_apply_catch( int toks, Parser& p, int line, int chr,
      int64 tid, const String* errName, String* tgt, bool genTrace = false, bool bNew = true )
{   
   ParserContext* ctx = static_cast<ParserContext*>(p.context());
   TreeStep* stmt = ctx->currentStmt();
   if( stmt == 0 || stmt->handler()->userFlags() != FALCON_SYNCLASS_ID_CATCHHOST )
   {
      p.addError( e_catch_outside, p.currentSource(), line, 0 );
   }
   else
   {      
      StmtTry* stmttry = static_cast<StmtTry*>(stmt);      
      SynTree* newBranch = bNew ?
               ctx->changeBranch()
               : ctx->currentTree();

      newBranch->decl(line,chr);
      if(genTrace)
      {
         newBranch->setTracedCatch();
      }
      
      // prepare the head symbol if needed
      if( tgt != 0 )
      {
         ctx->defineSymbol( *tgt );
         newBranch->target( Engine::getSymbol(*tgt ) );
      }
      
      if( errName != 0 )
      {
         // catch by name.         
         Requirement* req = stmttry->catchSelect()
                           .addSelectName( *errName, newBranch );  
         if( req == 0 )
         {
            // name clash.
            p.addError( e_catch_adef, p.currentSource(), line, chr );
         }
         else {
            ctx->onRequirement( req );
         }
      }
      else
      { 
         if( tid == -1 )
         {
            // the default catch
            if ( ! stmttry->catchSelect().setDefault( newBranch ) )
            {
               p.addError( e_catch_adef, p.currentSource(), line, chr );
               delete newBranch;
            }
         }
         // an integer catch
         else if( ! stmttry->catchSelect().addSelectType( tid, newBranch ) )
         {
            // tid duplicated.
            p.addError( e_catch_clash, p.currentSource(), line, chr );
            delete newBranch;
         }
      }
   }
   
   if( toks != 0 )
   {
      // simplify but leave the last EOL as success marker
      SourceParser& sp = *static_cast<SourceParser*>(&p);
      p.getLastToken()->token(sp.CatchSpec);
      p.simplify(toks-1);
   }
}

static void internal_apply_catch_case( int toks, Parser& p, int line, int chr,
      CaseList* cli, String* tgt, bool genTrace = false )
{
   CaseList::iterator iter = cli->begin();

   while( iter != cli->end() )
   {
      CaseItem* itm = *iter;

      switch( itm->m_type ) {
         case CaseItem::e_int:
            internal_apply_catch( 0, p, line, chr, itm->m_iLow, 0, tgt, genTrace, iter == cli->begin() );
            break;

         case CaseItem::e_sym:
            internal_apply_catch( 0, p, line, chr, -1, &itm->m_sym->name(), tgt, genTrace, iter == cli->begin()  );
            break;

         case CaseItem::e_string:
            internal_apply_catch( 0, p, line, chr, -1, itm->m_sLow, tgt, genTrace, iter == cli->begin() );
            break;

         case CaseItem::e_nil:
         case CaseItem::e_true:
         case CaseItem::e_false:
         case CaseItem::e_rngInt:
         case CaseItem::e_rngString:
            p.addError(e_syn_catch, p.currentSource(), line, chr );
            break;
      }

      ++iter;
   }

   // simplify but leave the last EOL as success marker
   SourceParser& sp = *static_cast<SourceParser*>(&p);
   p.getLastToken()->token(sp.CatchSpec);
   p.simplify(toks-1);
}

void apply_catch_all( const Rule&, Parser& p )
{
   // << T_EOL
   TokenInstance* ti = p.getNextToken();
   
   ParserContext* ctx = static_cast<ParserContext*>(p.context());
   TreeStep* stmt = ctx->currentStmt();
   if( stmt == 0 || stmt->handler()->userFlags() != FALCON_SYNCLASS_ID_CATCHHOST )
   {
      p.addError( e_catch_outside, p.currentSource(), ti->line(), ti->chr() );
   }
   else
   {
      StmtTry* stmttry = static_cast<StmtTry*>(stmt);
      if( stmttry->catchSelect().getDefault() != 0 )
      {
         p.addError( e_catch_adef, p.currentSource(), ti->line(), ti->chr() );
      }
      else
      {
         stmttry->catchSelect().setDefault( ctx->changeBranch() );
      }
   }
   
   // turn the eol in the catch clause
   SourceParser& sp = * static_cast<SourceParser*>(&p);
   ti->token( sp.CatchSpec );
}


void apply_catch_in_var( const Rule&, Parser& p )
{
   // << T_in << T_Name << T_EOL
   p.getNextToken();
   TokenInstance* ti = p.getNextToken();
   internal_apply_catch( 3, p, ti->line(), ti->chr(), -1, 0, ti->asString() );
}

void apply_catch_as_var( const Rule&, Parser& p )
{
   // << T_as << T_Name << T_EOL
   p.getNextToken();
   TokenInstance* ti = p.getNextToken();
   internal_apply_catch( 3, p, ti->line(), ti->chr(), -1, 0, ti->asString(), true );
}



void apply_catch_thing( const Rule&, Parser& p )
{
   //<<  CaseList << T_EOL
   TokenInstance* tname = p.getNextToken();   
   internal_apply_catch_case( 2, p, tname->line(), tname->chr(), static_cast<CaseList*>(tname->asData()), 0);
}


void apply_catch_thing_in_var( const Rule&, Parser& p )
{
   // << CaseList << T_in << T_Name << T_EOL
   TokenInstance* tname = p.getNextToken();
   p.getNextToken();
   TokenInstance* tgt = p.getNextToken();   
   internal_apply_catch_case( 4, p, tname->line(), tname->chr(), static_cast<CaseList*>(tname->asData()), tgt->asString() );
}

void apply_catch_thing_as_var( const Rule&, Parser& p )
{
   // << CaseList << T_as << T_Name << T_EOL
   TokenInstance* tname = p.getNextToken();
   p.getNextToken();
   TokenInstance* tgt = p.getNextToken();
   internal_apply_catch_case( 4, p, tname->line(), tname->chr(), static_cast<CaseList*>(tname->asData()), tgt->asString() );
}

}

/* end of parser_try.cpp */
