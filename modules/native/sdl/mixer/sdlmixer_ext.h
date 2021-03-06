/*
   FALCON - The Falcon Programming Language.
   FILE: sdlmixer_ext.h

   The SDL Mixer binding support module.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 04 Oct 2008 19:11:27 +0200

   -------------------------------------------------------------------
   (C) Copyright 2008: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

/** \file
   The SDL Mixer binding support module.
*/

#ifndef flc_sdlmix_ext_H
#define flc_sdlmix_ext_H

#include <falcon/setup.h>

namespace Falcon {
namespace Ext {

// Generic mixer
FALCON_FUNC mix_Compiled_Version( VMachine *vm );
FALCON_FUNC mix_Linked_Version( VMachine *vm );
FALCON_FUNC mix_OpenAudio( VMachine *vm );
FALCON_FUNC mix_CloseAudio( VMachine *vm );
FALCON_FUNC mix_QuerySpec( VMachine *vm );

// waves
FALCON_FUNC mix_LoadWAV( VMachine *vm );

// channels
FALCON_FUNC mix_AllocateChannels( VMachine *vm );
FALCON_FUNC mix_Volume( VMachine *vm );
FALCON_FUNC mix_Pause( VMachine *vm );
FALCON_FUNC mix_Resume( VMachine *vm );

FALCON_FUNC mix_HaltChannel( VMachine *vm );
FALCON_FUNC mix_ExpireChannel( VMachine *vm );
FALCON_FUNC mix_FadeOutChannel( VMachine *vm );
FALCON_FUNC mix_ChannelFinished( VMachine *vm );

FALCON_FUNC mix_Playing( VMachine *vm );
FALCON_FUNC mix_Paused( VMachine *vm );
FALCON_FUNC mix_FadingChannel( VMachine *vm );

// music
FALCON_FUNC mix_LoadMUS( VMachine *vm );
FALCON_FUNC mix_VolumeMusic( VMachine *vm );
FALCON_FUNC mix_HaltMusic( VMachine *vm );
FALCON_FUNC mix_FadeOutMusic( VMachine *vm );
FALCON_FUNC mix_PauseMusic( VMachine *vm );
FALCON_FUNC mix_ResumeMusic( VMachine *vm );
FALCON_FUNC mix_RewindMusic( VMachine *vm );
FALCON_FUNC mix_PausedMusic( VMachine *vm );
FALCON_FUNC mix_SetMusicPosition( VMachine *vm );
FALCON_FUNC mix_PlayingMusic( VMachine *vm );
FALCON_FUNC mix_SetMusicCMD( VMachine *vm );
FALCON_FUNC mix_SetSynchroValue( VMachine *vm );
FALCON_FUNC mix_GetSynchroValue( VMachine *vm );
FALCON_FUNC mix_HookMusicFinished( VMachine *vm );

//==========================================
// Mix Chunks
FALCON_FUNC MixChunk_init( VMachine *vm );
FALCON_FUNC MixChunk_Volume( VMachine *vm );
FALCON_FUNC MixChunk_Play( VMachine *vm );

//==========================================
// Mix Music
FALCON_FUNC MixMusic_init( VMachine *vm );
FALCON_FUNC MixMusic_GetType( VMachine *vm );
FALCON_FUNC MixMusic_Play( VMachine *vm );

}
}

#endif

/* end of sdlmixer_ext.h */
