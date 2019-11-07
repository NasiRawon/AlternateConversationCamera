#pragma once

#include "skse64/GameForms.h"
#include "skse64/NiExtraData.h"
#include "skse64/GameEvents.h"

#define NOP8 0x90
#define NOP16 0x9090
#define NOP32 0x90909090

namespace Tralala
{
	extern uintptr_t g_MenuTopicManagerAddr;
	extern uintptr_t g_iSizeWDisplayAddr;
	extern uintptr_t g_iSizeHDisplayAddr;
	extern uintptr_t g_deltaTimeAddr;
	extern uintptr_t g_sneakHeightMultAddr;
	extern uintptr_t g_isDialogueMenuCloseAddr;
	extern uintptr_t g_containerHandle;
	extern uintptr_t g_barterHandle;
	extern bool g_isTrainingMenu;

	void UtilsGetAddresses();

	class TESObjectREFR;
	class Actor;

	extern Actor* g_refTarget;

	// 1E8
	struct UnkCellInfo
	{
		UInt16          unk01;
		UInt32          unk04;
		bool			highProcess;		// 08 - AI Processing for actors in high, not tested
		bool			lowProcess;			// 09 - AI Processing for actors in low, not tested
		bool			middleHighProcess;	// 0A - AI Processing for actors in middle high, not tested
		bool			middleLowProcess;	// 0B - AI Processing for actors in middle low, not tested
		bool			aiSchedule;			// 0C - toggle AI Scheduling, not tested
		bool			subtitle;			// 0D - toggle dialog subtitle, doesn't work :(
		UInt8			unk0E;
		UInt32          numActorsInHigh;	// 10
		UInt32			unk14;
		UInt32          unk18;
		float			removeExcessComplexDeadTime;	// 1C
		HANDLE          semaphore;			// 20
		UInt32			unk28;
		UInt32			unk2C;
		tArray<UInt32>  actorHandles;	// 30 - current cell?
		tArray<UInt32>	unk48Handles;	// 48 - all?
		tArray<UInt32>	unk60Handles;	// 60 - previous cell, because null for the first time?
		tArray<UInt32>	unk78Handles;	// 78 - editor/original cell?
		UInt64			unk90[11];		// 90
		tArray<void*>	unkE8Array;		// E8
		SimpleLock		unkE8lock;		// 100
		tArray<void*>	unk108Array;	// 108
		SimpleLock		unk108lock;		// 120

		// more

		//static UnkCellInfo** GetSingleton();
	};
	STATIC_ASSERT(offsetof(UnkCellInfo, actorHandles) == 0x30);

	class TESCameraController
	{
	public:
		TESCameraController() {}

		UInt32 unk00;
		float  startRotZ; // 04
		float  startRotX; // 08
		float  endRotZ;   // 0C
		float  endRotX;   // 10
		float  unk14;     // 14
		float  unk18;     // 18
		UInt8  unk1C;     // 1C
		UInt8  pad1D[3];  // 1D

		static TESCameraController* GetSingleton();
	};

	// D8 or E0
	class MenuTopicManager : public BSTEventSink<MenuOpenCloseEvent>
	{
	public:
		virtual ~MenuTopicManager();

		struct DialogueInfo
		{
			tList<BSString>		responseText;	// 00
			TESQuest*			quest;			// 10
			TESTopicInfo*		topicInfo;		// 18
			TESTopic*			topic;			// 20
			DialogueInfo*		unk28;			// 28
			UInt32				unk30[2];		// 30
			TESTopic*			unk38;			// 38, same as topic above??
		};
		STATIC_ASSERT(offsetof(DialogueInfo, unk28) == 0x28);

		struct Dialogue
		{	
			BSString		topicText;		// 00
			bool			unk10;			// 10
			DialogueInfo	info;			// 18
		};
		STATIC_ASSERT(offsetof(Dialogue, info) == 0x18);
		STATIC_ASSERT(sizeof(Dialogue) == 0x58);

		BSTEventSink<void>	playerPositionEvent;	// 08
		UInt64				unk10;					// 10
		tList<Dialogue>*	clickedDialogueList;	// 18, "clicked dialogue"
		tList<Dialogue>*	initDialogueList;		// 20, all available "dialogue", game will iterate through this then assign pointer above
		UInt64				unk28;
		UInt64				unk30;
		Dialogue*			currentDialogue;		// 38
		CRITICAL_SECTION	critSection;			// 40
		UInt32	talkingHandle;						// 68	 - init'd to g_InvalidRefHandler
		UInt32	handle2;							// 6C	 - init'd to g_InvalidRefHandler
		void*	unk70;								// 70
		UInt64	unk78;								// 78
		tArray<void *>	unk80;						// 80
		tArray<void *>	unk98;						// 98
		UInt8	unkB0;								// B0	- dialouge init'd by NPCs
		bool	isInDialogueState;					// B1
		bool	unkB2;			// true, if dialogue's flag contains goodbye
		UInt8	unkB3;
		UInt8	unkB4;
		UInt8	unkB5;
		UInt8	unkB6;
		UInt8	unkB7;
		bool	unkB8;			// seems to be skiptext
		bool	unkB9;			// actor still talking but dialogue list is refreshed, last skiptext
		UInt8	unkBA;
		UInt8	unkBB;
		UInt16	padBC;
		tArray<void *>	unkC0;

		static MenuTopicManager * GetSingleton();
		NiPointer<TESObjectREFR> GetDialogueTarget();
	};
	STATIC_ASSERT(offsetof(MenuTopicManager, currentDialogue) == 0x38);
	STATIC_ASSERT(offsetof(MenuTopicManager, talkingHandle) == 0x68);

	// 80808
	class StringCache
	{
	public:
		struct Ref
		{
			const char	* data;

			Ref();
			Ref(const char * buf);
			~Ref();
			void Release();

			bool operator==(const Ref& lhs) const { return data == lhs.data; }
			bool operator<(const Ref& lhs) const { return data < lhs.data; }

			const char * c_str() const { return operator const char *(); }
			const char * Get() const { return c_str(); }
			operator const char *() const { return data ? data : ""; }
		};

		// 10
		struct Lock
		{
			UInt32	unk00;	// 00 - set to 80000000 when locked
			UInt32	pad04;	// 04
			UInt64	pad08;	// 08
		};

		void	* lut[0x10000];	// 00000
		Lock	lock[0x80];		// 80000
		UInt8	isInit;			// 80800
	};

	typedef StringCache::Ref BSFixedString;
}