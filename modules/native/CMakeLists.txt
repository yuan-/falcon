###############################################################################
# Falcon Programming language
# Build and installation suite
#
# Native modules
#

list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

#########################################################
# Utilities used in this section
#
macro( FalconMakeBinding module lib desc )

   if( FALCON_NATMODS_AUTO )
      find_package( ${lib} )
      if ( ${lib}_FOUND )
         option( FALCON_BUILD_${module} "${desc}" ON )
      else()
         option( FALCON_BUILD_${module} "${desc}" OFF )
      endif()
   else()
      option( FALCON_BUILD_${module} "${desc}" OFF )
   endif()

endmacro()

#########################################################

OPTION( FALCON_NATMODS_AUTO "Select automatically all the modules that have support" ON )

# Native modules, usually compiled
OPTION( FALCON_BUILD_DBI "Install DBI module" ON )
OPTION( FALCON_BUILD_DYNLIB "Install DYNLIB module" OFF)
OPTION( FALCON_BUILD_MONGODB "Install MongoDB module" ON )

# Bindings -- native modules requiring external libraries.
FalconMakeBinding( CURL CURL "Install CURL binding" )
FalconMakeBinding( SDL SDL "Install SDL  binding" )


# Other modules
option( FALCON_BUILD_WOPI "Install Web Oriented Programming Interface" OFF  )

# TODO: foreach ftw
if( FALCON_NATMODS_AUTO )

   #### Check DBUS
   find_path( DBUS_INCLUDE_DIR dbus.h HINTS /usr/include/dbus-1.0/dbus )
   if ( DBUS_INCLUDE_DIR )
      set( _dbus ON )
   endif()

   #### Check DataMatrix
   include( FalconFindDataMatrix )

   #### Check HPDF (LibHARU)
   find_path( LibHaru_INCLUE_DIR hpdf.h )
   if ( LibHaru_INCLUE_DIR )
      set( _pdf ON )
   endif()

   #### Check GD2
   find_path( GD_INCLUDE_DIR gd.h )
   if ( GD_INCLUDE_DIR )
      set( _gd2 ON )
   endif()

   #### Check GTK2
   include( FalconFindGTK2 )

else()
  set( _dbus OFF )
  set( _dmtx OFF )
  set( _gd2 OFF )
  set( _gtk OFF )
  set( _pdf OFF )
endif()

# Bindings -- Requiring more sophisticated checks
option( FALCON_BUILD_DBUS "Install DBUS binding"  ${_dbus})
option( FALCON_BUILD_DMTX "Install DataMatrix binding" ${_dmtx} )
option( FALCON_BUILD_GD2 "Install GD library binding" ${_gd2})
option( FALCON_BUILD_GTK "Install GTK2 module" ${_gtk} )
option( FALCON_BUILD_PDF "Install libharu (hpdf)  binding" ${_pdf})

#########################################################
# Inclusion statements
#

if(FALCON_BUILD_CURL)
   add_subdirectory( curl )
endif()

if ( FALCON_BUILD_DBI )
    add_subdirectory( dbi )
endif()

if ( FALCON_BUILD_DBUS )
    add_subdirectory( dbus )
endif()

if ( FALCON_BUILD_DMTX )
    add_subdirectory( dmtx )
endif()

if ( FALCON_BUILD_DYNLIB )
    add_subdirectory( dynlib )
endif()

if ( FALCON_BUILD_GD2 )
    add_subdirectory( gd2 )
endif()

if ( FALCON_BUILD_GTK )
    add_subdirectory( gtk )
endif()

if ( FALCON_BUILD_MONGODB )
    add_subdirectory( mongodb )
endif()

if ( FALCON_BUILD_PDF )
    add_subdirectory( hpdf )
endif()

if ( FALCON_BUILD_SDL )
    add_subdirectory( sdl )
endif()

if ( FALCON_BUILD_WOPI )
    add_subdirectory( wopi )
endif()

if ( FALCON_BUILD_CONIO )
    add_subdirectory( conio )
endif()
