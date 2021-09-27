#pragma once

#include "Defines.h"
#include <Abort.h>

/// <summary>
/// Helper Class to make large-scale cleanup an absolute breeze.
/// </summary>

//MAKER(sean)
class StagedBuffer {
private:
	u8* m_Pointer;
	usize m_Capacity;
	usize m_Length;

public:
	// I dont see much reason defining these in a separate cpp file when they're so short

	StagedBuffer(usize capacity);
	~StagedBuffer();
	void Reset(usize requested_reset);

	u8* Alloc(usize requested_size);
};
