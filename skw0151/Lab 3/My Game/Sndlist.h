/// \file SndList.h
/// \brief Enumerated type for sounds.

#pragma once

#include "Sound.h"

/// \brief Game sound enumerated type. 
///
/// These are the sounds used in actual gameplay. 
/// The sounds must be listed here in the same order that
/// they are in the sound settings XML file.

enum eSoundType{ 
	BEEP_SOUND, BOOP_SOUND, CRUNCH_SOUND, BUZZ_SOUND, LAUNCH_SOUND
}; //eSoundType