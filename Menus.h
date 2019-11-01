#pragma once

#include "skse64/GameMenus.h"
#include "ScaleformUtils.h"

namespace Tralala
{
	// 98
	class HUDMenu : public IMenu
	{
	public:
		BSTEventSink<void>	unk30;			// 30 - UserEventEnabledEvent
		void*				unk38;			// 38 - New in SE
		tArray<HUDObject*>	hudComponents;	// 40
		ActorValueMeter*	unk58;			// 58
		ActorValueMeter*	unk60;			// 60
		ActorValueMeter*	unk68;			// 68
		ShoutMeter*			unk70;			// 70
		GFxValue			hudBaseInstance;// 78
		UInt8				unk90;			// 90
		UInt8				unk91[7];		// 91
	};
	STATIC_ASSERT(sizeof(HUDMenu) == 0x98);

	// 100
	class CraftingSubMenu : public FxDelegateHandler
	{
	public:
		virtual ~CraftingSubMenu();

		// 40
		struct ItemInfo
		{
			GFxValue	object;			// 00
			BSString	effectDesc;		// 18
			BSString	unk28;
			GFxMovieView* parentView;	// 38
		};
		

		BSTEventSink<void>	unk10;		// 010 - TESFurnitureEvent
		GFxMovieView*		view;		// 018
		TESFurniture*		furniture;	// 20
		ItemInfo*			itemInfo;		
		GFxValue*			unk30;
		GFxValue			menuInstance;	// 38
		GFxValue			menuItemList;	// 50
		GFxValue			unk68;
		GFxValue			menuItemInfo;	// 80
		GFxValue			menuBottomBarInfo;	// 98
		GFxValue			menuAdditionalDesc;	// B0
		GFxValue			menuName;	// C8
		GFxValue			buttonText;		// E0
		bool				unkF8;
	};
	STATIC_ASSERT(sizeof(CraftingSubMenu) == 0x100);

	// 1A0
	class AlchemyMenu : public CraftingSubMenu
	{
	public:
		// 10
		struct EntryData
		{
			InventoryEntryData* data;		// 00
			UInt32				filterFlag;	// 08
			UInt8				bEquipped;	// 0C
			UInt8				bEnabled;	// 0D
			UInt16				pad0E;		// 0E		
		};
		STATIC_ASSERT(sizeof(EntryData) == 0x10);

		// 20, seems to be bstsmallarray
		struct SelectedIndex
		{
			UInt32	reserve;			// 00 - init'd to local alloc
			UInt32	pad04;
			UInt32	firstSlot;
			UInt32	secondSlot;
			UInt32	thirdSlot;
			UInt32	pad14;
			UInt32	count;
			UInt32	pad1C;
		};
		STATIC_ASSERT(sizeof(SelectedIndex) == 0x20);

		struct Item // sorted by highest value
		{
			UInt32	index;			// ingredient's index on the list
			UInt32	effectIndex;	// ingredient effect's index
		};

		struct Data138
		{
			tArray<Item*> unk00;
			tArray<EntryData>* items;
		};

		tArray<EntryData> items;			// 100
		SelectedIndex	selectedIdx;	// 118
		Data138			unk138;
		GFxValue		categoryList;		// 158
		AlchemyItem**	unk170;
		AlchemyItem*	defaultCraftedPot;	// 178, potion of unknown effect
		AlchemyItem*	unk180;			// 180
		UInt32			curItemIndexByName;	// 188
		BSFixedString	fontColor;			// 190
		bool			hasPurityPerk;		// 198
	};
	STATIC_ASSERT(sizeof(AlchemyMenu) == 0x1A0);

	// 38
	class CraftingMenu : public IMenu
	{
	public:

		CraftingSubMenu*	subMenu;	// 30
	};


	// 50
	class DialogueMenu : public IMenu,
		public BSTEventSink<MenuOpenCloseEvent>
	{
	public:

		tArray<BSString> topicList;	// 38
	};
	STATIC_ASSERT(sizeof(DialogueMenu) == 0x50);

}

namespace Menus
{
	void GetAddresses();
	bool InstallHook();
}