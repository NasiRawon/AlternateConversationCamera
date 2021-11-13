#include "Controls.h"
#include "PatternScanner.h"

namespace Tralala
{
	uintptr_t g_playerControlsAddr = 0;
	uintptr_t g_menuControlsAddr = 0;

	void ControlsGetAddresses()
	{
		const std::array<BYTE, 8> playerpattern = { 0x8B, 0x40, 0x10, 0xC1, 0xE8, 0x17, 0xA8, 0x01 };
		g_playerControlsAddr = (uintptr_t)scan_memory(playerpattern, 0x6E, true, 0x3, 0x7);

		const std::array<BYTE, 7> menupattern = { 0x48, 0x8B, 0x91, 0xF0, 0x03, 0x03, 0x00 };
		g_menuControlsAddr = (uintptr_t)scan_memory(menupattern, 0x7, true, 0x3, 0x7);
	}

	MenuControls* MenuControls::GetSingleton()
	{
		return *(MenuControls**)g_menuControlsAddr;
	}

	PlayerControls* PlayerControls::GetSingleton()
	{
		return *(PlayerControls**)g_playerControlsAddr;
	}
}