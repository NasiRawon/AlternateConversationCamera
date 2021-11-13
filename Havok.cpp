#include "PatternScanner.h"
#include "Havok.h"


namespace Havok
{
	uintptr_t g_getNiObjectAddr = 0;

	void GetAddresses()
	{
		const std::array<BYTE, 11> pattern = { 0x4A, 0x8B, 0x4C, 0x38, 0x30, 0x8B, 0x79, 0x2C, 0x83, 0xE7, 0x7F };
		g_getNiObjectAddr = (uintptr_t)scan_memory(pattern, 0xB, true, 0x1, 0x5);
	}


	NiAVObject* hkpCollidable::GetNiObject()
	{
		typedef NiAVObject* (*GetNiObject_t)(hkpCollidable*);
		GetNiObject_t GetNiObject = (GetNiObject_t)g_getNiObjectAddr;

		return GetNiObject(this);
	}
}