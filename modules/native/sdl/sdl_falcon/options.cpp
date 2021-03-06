/*
   FALCON - The Falcon Programming Language.
   FILE: options.cpp

   Falcon compiler and interpreter - options file
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Fri, 10 Sept 2004 13:15:23 +0100

   -------------------------------------------------------------------
   (C) Copyright 2004: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

/** \file
   Falcon compiler and interpreter - options file
*/

#include "options.h"
#include <iostream>

using namespace std;
using namespace Falcon;


FalconOptions::FalconOptions():
   m_modal( false ),
   m_justinfo( false ),
   input( "" ),
   output( "" ),
   load_path( "" ),
   io_encoding( "" ),
   source_encoding( "" ),
   module_language( "" ),

   compile_only( false ),
   run_only( false ),
   tree_out( false ),
   assemble_out( false ),
   search_path( false ),
   force_recomp( false ),
   check_memory( false ),

   comp_memory( true ),
   recompile_on_load( true ),
   save_modules( true ),
   wait_after( false ),
   parse_ftd( false ),

   compile_tltable( false ),
   interactive( false ),
   ignore_syspath( false )
{}


void FalconOptions::usage( bool deep )
{
   cout
      << "Usage:\n" << endl
      << "       falcon (-c|-S|-t) [c_opts] [-o<output>] module" << endl
      << "       falcon -y [-o<output>] module" << endl
      << "       falcon -x [c_options] module" << endl
      << "       falcon [c_opts] [r_opts] module [script options]" << endl
      << "       falcon -i [-p module] ...[-p module]" << endl
      << endl;

   if( deep )
   {
      cout
      << "Modal options:" << endl
      << "   -c          compile the given source in a .fam module" << endl
      << "   -i          interactive mode" << endl
      << "   -S          produce an assembly output" << endl
      << "   -t          generate a syntactic tree (for logic debug)" << endl
      << "   -y          write string translation table for the module" << endl
      << "   -x          execute a binary '.fam' module" << endl
      << endl
      << "Compilation options (c_opts):" << endl
      << "   -d          Set directive (as <directive>=<value>)" << endl
      << "   -D          Set constant (as <constant>=<value>)" << endl
      << "   -E <enc>    Source files are in <enc> encoding (overrides -e)" << endl
      << "   -f          force recompilation of modules even when .fam are found" << endl
      << "   -m          do NOT compile in memory (use temporary files)" << endl
      << "   -T          consider given [module] as .ftd (template document)" << endl
      << endl
      << "Run options (r_opts):" << endl
      << "   -C          check for memory allocation correctness." << endl
      << "   -e <enc>    set given encoding as default for VM I/O." << endl
      << "   -l <lang>   Set preferential language of loaded modules" << endl
      << "   -L <path>   Add path for 'load' directive (start with ';' remove std paths)" << endl
      << "   -M          do NOT save the compiled modules in '.fam' files" << endl
      << "   -p <module> preload (pump in) given module" << endl
      << "   -P          ignore system PATH (and FALCON_LOAD_PATH envvar)" << endl
      << "   -r          do NOT recompile sources (ignore sources)" << endl
      << endl
      << "General options:" << endl
      << "   -h/-?       display usage" << endl
      << "   -H          this help" << endl
      << "   -o <fn>     output to <fn> instead of [module.xxx]" << endl
      << "   -v          print copyright notice and version and exit" << endl
      << "   -w          add an extra console wait after program exit" << endl
      << "" << endl
      << "Paths must be in falcon file name format: directory separators must be slashes [/] and" << endl
      << "multiple entries must be entered separed by a semicolon (';')" << endl
      << "File names may be set to '-' meaning standard input or output (depending on the option)" << endl
      << endl;
   }
   else
   {
      cout
      << "Use '-H' option to get more help." << endl;
   }
}


void FalconOptions::modalGiven()
{
   if (m_modal)
   {
      throw String("multiple modal options selected");
   }

   m_modal = true;
}


void FalconOptions::parse( int argc, char **argv, int &script_pos )
{
   // option decoding
   for ( int i = 1; i < argc; i++ )
   {
      char *op = argv[i];

      if ( op[0] == '-' )
      {
         switch ( op[1] )
         {
            case 'c': modalGiven(); compile_only = true; break;
            case 'C': check_memory = true; break;
            case 'd':
               if ( op[2] == 0 && i + 1< argc )
                  directives.pushBack ( new String ( argv[++i] ) );
               else
                  directives.pushBack ( new String ( op + 2 ) );
               break;

            case 'D':
               if ( op[2] == 0 && i + 1< argc )
                  defines.pushBack ( new String ( argv[++i] ) );
               else
                  defines.pushBack ( new String ( op + 2 ) );
               break;

            case 'e':
               if ( op[2] == 0 && i + 1 < argc )
               {
                  io_encoding = argv[++i];
               }
               else
               {
                  io_encoding = op + 2;
               }
               break;

            case 'E':
               if ( op[2] == 0 && i + 1< argc )
               {
                  source_encoding = argv[++i];
               }
               else
               {
                  source_encoding = op + 2;
               }
               break;

            case 'f': force_recomp = true; break;
            case 'h': case '?': usage(false); m_justinfo = true; break;
            case 'H': usage(true); m_justinfo = true; break;
            case 'i': modalGiven(); interactive = true; break;

            case 'L':
               if ( op[2] == 0 && i + 1 < argc )
                  load_path = argv[++i];
               else
                  load_path = op + 2;
               break;

            case 'l':
               if ( op[2] == 0 && i + 1 < argc )
                  module_language = argv[++i];
               else
                  module_language = op + 2;
               break;

            case 'm': comp_memory = false; break;
            case 'M': save_modules = false; break;

            case 'o':
               if ( op[2] == 0 && i + 1< argc )
                  output = argv[++i];
               else
                  output = op + 2;
               break;

            case 'p':
               if ( op[2] == 0 && i + 1< argc )
                  preloaded.pushBack ( new String ( argv[++i] ) );
               else
                  preloaded.pushBack ( new String ( op + 2 ) );
               break;

            case 'P': ignore_syspath = true; break;
            case 'r': recompile_on_load = false; break;

            case 'S': modalGiven(); assemble_out = true; break;
            case 't': modalGiven(); tree_out = true; break;
            case 'T': parse_ftd = true; break;
            case 'x': run_only = true; break;
            case 'v': version(); m_justinfo = true; break;
            case 'w': wait_after = true; break;
            case 'y': modalGiven(); compile_tltable = true; break;

            default:
               cerr << "falcon: unrecognized option '" << op << endl << endl;
               usage(false);
               m_justinfo = true;
         }
      }
      else
      {
         input = op;
         script_pos = i+1;
         // the other m_options are for the script.
         break;
      }
   }

}

void FalconOptions::version()
{
   cout << "Falcon compiler and interpreter." << endl;
   cout << "Version "  << FALCON_VERSION " (" FALCON_VERSION_NAME ")" << endl << endl;
}


/* options.cpp */
