#pragma once

#include "skse64/GameInput.h"

namespace Tralala
{
	extern uintptr_t g_playerControlsAddr;
	extern uintptr_t g_menuControlsAddr;

	void ControlsGetAddresses();

	// 90
	class MenuControls
	{
	public:
		virtual			~MenuControls();
		virtual UInt32	Unk_01();

		//	void			** _vtbl;		// 00
		BSTEventSink<MenuModeChangeEvent> menuModeEventSink; // 08
		UInt64			unk10;			// 10
		tArray<void*>	arr18;			// 18
		UInt64			unk30[3];		// 30

		MenuEventHandler* clickHandler;	// 48
		MenuEventHandler* directionHandler;	// 50
		MenuEventHandler* consoleOpenHandler;	// 58
		MenuEventHandler* quickSaveLoadHandler;	// 60
		MenuEventHandler* menuOpenHandler;	// 68
		MenuEventHandler* favoritesHandler;	// 70
		MenuEventHandler* screenshotHandler;	// 78

		UInt8			unk80;			// 80
		bool			beastForm;			// 81
		bool			remapMode;		// 82
		UInt8			unk83;			// 83
		UInt8			pad84[0x90 - 0x84];	// 84

		static MenuControls *	GetSingleton();
	};
	STATIC_ASSERT(offsetof(MenuControls, remapMode) == 0x082);
}