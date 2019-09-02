#include "ObjectRef.h"
#include "Utils.h"
#include "PatternScanner.h"

namespace Tralala
{
	uintptr_t g_MenuTopicManagerAddr = 0;
	uintptr_t g_TESCameraControllerAddr = 0;
	uintptr_t g_bsfixedstringCtorAddr = 0;
	uintptr_t g_bsfixedstringDtorAddr = 0;
	uintptr_t g_iSizeWDisplayAddr = 0;
	uintptr_t g_iSizeHDisplayAddr = 0;
	uintptr_t g_deltaTimeAddr = 0;
	uintptr_t g_sneakHeightMultAddr = 0;
	//uintptr_t g_unkcellinfoAddr = 0;

	void UtilsGetAddresses()
	{
		const std::array<BYTE, 13> Topicpattern = { 0x48, 0x85, 0xC0, 0x41, 0xB1, 0x01, 0x48, 0x8B, 0xD7, 0x4C, 0x0F, 0x45, 0xC0 };
		g_MenuTopicManagerAddr = (uintptr_t)scan_memory_data(Topicpattern, 0x55, true, 0x3, 0x7);

		const std::array<BYTE, 10> Controllerpattern = { 0xF3, 0x0F, 0x5C, 0xF0, 0x48, 0x8B, 0xCF, 0x0F, 0x28, 0xCE };
		g_TESCameraControllerAddr = (uintptr_t)scan_memory_data(Controllerpattern, 0x99, false, 0x3, 0x7);

		const std::array<BYTE, 6> ctorpattern = { 0x45, 0x33, 0xDB, 0x48, 0x8B, 0xD9 };
		g_bsfixedstringCtorAddr = (uintptr_t)scan_memory(ctorpattern, 0x6, false);

		const std::array<BYTE, 7> dtorpattern = { 0x44, 0x8D, 0x42, 0x38, 0x48, 0x8B, 0xCB };
		g_bsfixedstringDtorAddr = (uintptr_t)scan_memory_data(dtorpattern, 0x18, false, 0x3, 0x7);

		const std::array<BYTE, 6> wdispattern = { 0x89, 0x44, 0x24, 0x5C, 0x33, 0xC0 };
		g_iSizeWDisplayAddr = (uintptr_t)scan_memory_data(wdispattern, 0x36, false, 0x2, 0x6);

		const std::array<BYTE, 6> hdispattern = { 0x89, 0x44, 0x24, 0x5C, 0x33, 0xC0 };
		g_iSizeHDisplayAddr = (uintptr_t)scan_memory_data(hdispattern, 0x2C, false, 0x2, 0x6);

		const std::array<BYTE, 6> deltaTimepattern = { 0x0F, 0x28, 0xCB, 0x0F, 0x54, 0xCD };
		g_deltaTimeAddr = (uintptr_t)scan_memory_data(deltaTimepattern, 0xC, false, 0x4, 0x8);

		const std::array<BYTE, 6> sneakpattern = { 0x48, 0x8B, 0xC3, 0x89, 0x4B, 0x04 };
		g_sneakHeightMultAddr = (uintptr_t)scan_memory_data(sneakpattern, 0x25, true, 0x4, 0x8);

		//static const BYTE payload[] = { 0x0F, 0x94, 0xC0, 0x88, 0x41, 0x0A };
		//std::vector<BYTE> pattern(payload, payload + sizeof(payload) / sizeof(payload[0]));
		//g_unkcellinfoAddr = (uintptr_t)scan_memory_data(pattern, 0x10, false, 0x3, 0x7);

	}

	//UnkCellInfo** UnkCellInfo::GetSingleton()
	//{
	//	return (UnkCellInfo**)g_unkcellinfoAddr;
	//}

	TESCameraController* TESCameraController::GetSingleton()
	{
		return (TESCameraController*)g_TESCameraControllerAddr;
	}

	MenuTopicManager * MenuTopicManager::GetSingleton()
	{
		return *(MenuTopicManager**)g_MenuTopicManagerAddr;
	}

	TESObjectREFR * MenuTopicManager::GetDialogueTarget()
	{
		TESObjectREFR * refr = nullptr;
		if (talkingHandle != Tralala::InvalidRefHandle())
			LookupRefByHandle(&talkingHandle, &refr);

		if (!refr)
		{
			if (handle2 != Tralala::InvalidRefHandle())
				LookupRefByHandle(&handle2, &refr);
		}

		if (refr)
		{
			refr->handleRefObject.DecRef();
		}
		
		return refr;
	}

	StringCache::Ref::Ref()
	{
		typedef StringCache::Ref*(*ctor_t)(StringCache::Ref *, const char *);
		ctor_t ctor = (ctor_t)g_bsfixedstringCtorAddr;

		ctor(this, "");
	}

	StringCache::Ref::Ref(const char * buf)
	{
		typedef StringCache::Ref*(*ctor_t)(StringCache::Ref *, const char *);
		ctor_t ctor = (ctor_t)g_bsfixedstringCtorAddr;

		ctor(this, buf);
	}

	void StringCache::Ref::Release()
	{
		typedef void(*dtor_t)(StringCache::Ref*);
		dtor_t dtor = (dtor_t)g_bsfixedstringDtorAddr;

		dtor(this);
	}
}