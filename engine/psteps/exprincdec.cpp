/*
   FALCON - The Falcon Programming Language.
   FILE: exprincdec.cpp

   Standard misc PSteps commonly used in the virtual machine.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 31 Jul 2011 00:39:23 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#undef SRC 
#define SRC "engine/psteps/exprincdec.cpp"

#include <falcon/trace.h>
#include <falcon/vmcontext.h>
#include <falcon/class.h>
#include <falcon/stdsteps.h>
#include <falcon/textwriter.h>

#include <falcon/synclasses.h>
#include <falcon/engine.h>


#include <falcon/psteps/exprincdec.h>
#include <falcon/errors/operanderror.h>

namespace Falcon
{

class ExprPreInc::ops
{
public:
   inline static void preAssign( VMContext* ) {}
   inline static void postAssign( VMContext* ) {}
   inline static int64 operate( int64 a ) { return a + 1; }
   inline static void operate( VMContext* ctx, Class* cls, void* inst ) { cls->op_inc(ctx, inst); }
   inline static numeric operaten( numeric a ) { return a + 1.0; }
   inline static const char* id() { return "++"; }
};


class ExprPreDec::ops
{
public:
   inline static void preAssign( VMContext* ) {}
   inline static void postAssign( VMContext* ) {}
   inline static int64 operate( int64 a ) { return a - 1; }
   inline static void operate( VMContext* ctx, Class* cls, void* inst ) { cls->op_dec(ctx, inst); }
   inline static numeric operaten( numeric a ) { return a - 1.0; }
   inline static const char* id() { return "--"; }
};


class ExprPostInc::ops
{
public:
   inline static void preAssign( VMContext* ctx ) { Item temp = ctx->topData(); ctx->pushData(temp);}
   inline static void postAssign( VMContext* ctx ) {ctx->popData();}
   inline static int64 operate( int64 a ) { return a + 1; }
   inline static void operate( VMContext* ctx, Class* cls, void* inst ) { cls->op_incpost(ctx, inst); }
   inline static numeric operaten( numeric a ) { return a + 1.0; }
   inline static const char* id() { return "<post>++"; }
};


class ExprPostDec::ops
{
public:
   inline static void preAssign( VMContext* ctx ) { Item temp = ctx->topData(); ctx->pushData(temp);}
   inline static void postAssign( VMContext* ctx ) {ctx->popData();}
   inline static int64 operate( int64 a ) { return a - 1; }
   inline static void operate( VMContext* ctx, Class* cls, void* inst ) { cls->op_decpost(ctx, inst); }
   inline static numeric operaten( numeric a ) { return a - 1.0; }
   inline static const char* id() { return "<post>--"; }
};

//==============================================================
// Genetic operands


// Inline class to simplify
template <class _cpr >
bool generic_simplify( Item& value, TreeStep* first )
{
   if( first->category() == TreeStep::e_cat_expression && static_cast<Expression*>(first)->simplify( value ) )
   {
      switch( value.type() )
      {
      case FLC_ITEM_INT: value.setInteger( _cpr::operate(value.asInteger()) ); return true;
      case FLC_ITEM_NUM: value.setNumeric( _cpr::operaten(value.asNumeric()) ); return true;
      }
   }

   return false;
}


// Inline class to apply
template <class _cpr >
void generic_apply_( const PStep* ps, VMContext* ctx )
{ 
   const UnaryExpression* self = static_cast<const UnaryExpression*>(ps);
#ifndef NDEBUG
   TRACE2( "Apply \"%s\"", self->describe().c_ize() );
#endif

   fassert( self->first() != 0 );
   
   CodeFrame& cf = ctx->currentCode();
   switch( cf.m_seqId )
   {
      // Phase 0 -- generate the item.
   case 0:
      cf.m_seqId = 1;
      if( ctx->stepInYield( self->first(), cf ) )
      {
         return;
      }
      /* no break */
   
      // Phase 1 -- operate
   case 1:
      cf.m_seqId = 2;
      {   
         // eventually save the result if necessary
         _cpr::preAssign( ctx );
         register Item *op = &ctx->topData();
         
         switch( op->type() )
         {
            case FLC_ITEM_INT: op->setInteger( _cpr::operate(op->asInteger()) ); break;
            case FLC_ITEM_NUM: op->setNumeric( _cpr::operaten(op->asNumeric()) ); break;
            default:
            {
               Class* cls;
               void* data;
               if( op->asClassInst( cls, data ) )
               {
                  // it's all in the hands of our class.
                  ctx->popCode();
                  _cpr::postAssign( ctx );
                  _cpr::operate( ctx, cls, data );
                  return;
               }
               else {
                  throw
                     new OperandError( ErrorParam(e_invalid_op, __LINE__ ).extra(_cpr::id()) );
               }
            }
         }
      }
      /* no break */
   
      // Phase 2 -- assigning the topmost value back.
   case 2:
      cf.m_seqId = 3;
      // now assign the topmost item in the stack to the lvalue of self.
      PStep* lvalue = self->first()->lvalueStep();
      if( lvalue != 0 )
      {
         if( ctx->stepInYield( lvalue, cf ) )
         {
            return;
         }
      }
      /* no break */
      
   }
   
   // eventually pop the stack -- always the code ... 
   ctx->popCode();
   // ... and possibly the parameter on the stack 
   _cpr::postAssign( ctx );
}


//=========================================================
// Implementation -- preinc
//

bool ExprPreInc::simplify( Item& value ) const
{
   return generic_simplify<ExprPreInc::ops>( value, m_first );
}


void ExprPreInc::apply_( const PStep* ps, VMContext* ctx )
{  
   generic_apply_<ExprPreInc::ops>(ps, ctx);
}


const String& ExprPreInc::exprName() const
{
   static String name("++");
   return name;
}

//=========================================================
// Implementation -- postinc
//

bool ExprPostInc::simplify( Item& value ) const
{
   return generic_simplify<ExprPostInc::ops>( value, m_first );
}


void ExprPostInc::apply_( const PStep* ps, VMContext* ctx )
{  
   generic_apply_<ExprPostInc::ops>(ps, ctx);
}


void ExprPostInc::render( TextWriter* tw, int depth ) const
{
   tw->write(renderPrefix(depth));

   if( m_first == 0 )
   {
      tw->write( "/* Blank ExprPostInc */" );
   }
   else
   {
      tw->write( "( " );
      m_first->render( tw, relativeDepth(depth) );
      tw->write( "++ )");
   }

   if( depth >= 0 )
   {
      tw->write("\n");
   }
}

const String& ExprPostInc::exprName() const
{
   static String name("++");
   return name;
}

//=========================================================
// Implementation -- predec
//

bool ExprPreDec::simplify( Item& value ) const
{
   return generic_simplify<ExprPreDec::ops>( value, m_first );
}


void ExprPreDec::apply_( const PStep* ps, VMContext* ctx )
{  
   generic_apply_<ExprPreDec::ops>(ps, ctx);
}


const String& ExprPreDec::exprName() const
{
   static String name("--");
   return name;
}

//=========================================================
// Implementation -- postdec
//

bool ExprPostDec::simplify( Item& value ) const
{
   return generic_simplify<ExprPostDec::ops>( value, m_first );
}


void ExprPostDec::apply_( const PStep* ps, VMContext* ctx )
{  
   generic_apply_<ExprPostDec::ops>(ps, ctx);
}


void ExprPostDec::render( TextWriter* tw, int depth ) const
{
   tw->write(renderPrefix(depth));

   if( m_first == 0 )
   {
      tw->write( "/* Blank ExprPostDec */" );
   }
   else
   {
      tw->write( "( " );
      m_first->render( tw, relativeDepth(depth) );
      tw->write( " --)");
   }

   if( depth >= 0 )
   {
      tw->write("\n");
   }
}

const String& ExprPostDec::exprName() const
{
   static String name("--");
   return name;
}

}

/* end of exprincdec.cpp */