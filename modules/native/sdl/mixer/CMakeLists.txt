####################################################################
# The Falcon Programming language
#
# CMake configuration file for module SDL_mixer
####################################################################

FALCON_DEFINE_MODULE( SDLMIXER mixer )

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 OLD)
endif(COMMAND cmake_policy)

Find_Package ( SDL_mixer REQUIRED )
INCLUDE_DIRECTORIES( ${SDLMIXER_INCLUDE_DIR} )

# Target
ADD_LIBRARY( ${SDLMIXER} MODULE
   sdlmixer.cpp
   sdlmixer_ext.cpp
   sdlmixer_mod.cpp
)

#Link
TARGET_LINK_LIBRARIES(${SDLMIXER} falcon_engine ${SDLMIXER_LIBRARY} ${SDL_LIBRARY} )

SET_TARGET_PROPERTIES(${SDLMIXER}
   PROPERTIES PREFIX "")
#Install
FALCON_INSTALL_MODULE2( ${SDLMIXER} sdl )
