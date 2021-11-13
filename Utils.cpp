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
	uintptr_t g_isDialogueMenuCloseAddr = 0;
	uintptr_t g_containerHandle = 0;
	uintptr_t g_barterHandle = 0;

	void UtilsGetAddresses()
	{
		const std::array<BYTE, 13> Topicpattern = { 0x48, 0x85, 0xC0, 0x41, 0xB1, 0x01, 0x48, 0x8B, 0xD7, 0x4C, 0x0F, 0x45, 0xC0 };
		g_MenuTopicManagerAddr = (uintptr_t)scan_memory(Topicpattern, 0x55, true, 0x3, 0x7);

		const std::array<BYTE, 12> Controllerpattern = { 0xF3, 0x44, 0x0F, 0x10, 0x80, 0x54, 0x01, 0x00, 0x00, 0x48, 0x8B, 0xCE };
		g_TESCameraControllerAddr = (uintptr_t)scan_memory(Controllerpattern, 0x61, true, 0x3, 0x7) - (uintptr_t)0x1C;

		const std::array<BYTE, 8> ctorpattern = { 0x4D, 0x8D, 0x52, 0x01, 0x41, 0x0F, 0x47, 0xC9 };
		g_bsfixedstringCtorAddr = (uintptr_t)scan_memory(ctorpattern, 0x4E, false);

		const std::array<BYTE, 7> dtorpattern = { 0x44, 0x8D, 0x42, 0x38, 0x48, 0x8B, 0xCB };
		g_bsfixedstringDtorAddr = (uintptr_t)scan_memory(dtorpattern, 0x18, false, 0x3, 0x7);

		const std::array<BYTE, 14> wdispattern = { 0x4C, 0x8D, 0x4C, 0x24, 0x58, 0x4C, 0x8D, 0x44, 0x24, 0x60, 0x48, 0x8D, 0x55, 0xA8 };
		g_iSizeWDisplayAddr = (uintptr_t)scan_memory(wdispattern, 0x3F, false, 0x2, 0x6);

		const std::array<BYTE, 14> hdispattern = { 0x4C, 0x8D, 0x4C, 0x24, 0x58, 0x4C, 0x8D, 0x44, 0x24, 0x60, 0x48, 0x8D, 0x55, 0xA8 };
		g_iSizeHDisplayAddr = (uintptr_t)scan_memory(hdispattern, 0x35, false, 0x2, 0x6);

		const std::array<BYTE, 10> deltaTimepattern = { 0xC6, 0x81, 0x18, 0x02, 0x00, 0x00, 0x00, 0x41, 0xB0, 0x01 };
		g_deltaTimeAddr = (uintptr_t)scan_memory(deltaTimepattern, 0xA, true, 0x4, 0x8);

		const std::array<BYTE, 8> sneakpattern = { 0x8B, 0x41, 0x54, 0x48, 0x8B, 0xFA, 0x89, 0x02 };
		g_sneakHeightMultAddr = (uintptr_t)scan_memory(sneakpattern, 0x50, true, 0x4, 0x8);

		const std::array<BYTE, 5> dialpattern = { 0x32, 0xC9, 0x45, 0x84, 0xF6 };
		g_isDialogueMenuCloseAddr = (uintptr_t)scan_memory(dialpattern, 0x1E, true, 0x2, 0x7);

		const std::array<BYTE, 6> contpattern = { 0x48, 0x3B, 0xD0, 0x0F, 0x94, 0xC2 };
		g_containerHandle = (uintptr_t)scan_memory(contpattern, 0x39, true, 0x2, 0x6);

		const std::array<BYTE, 7> bartpattern = { 0x4C, 0x8B, 0xEF, 0x48, 0x8B, 0x43, 0x28 };
		g_barterHandle = (uintptr_t)scan_memory(bartpattern, 0x33, false, 0x3, 0x7);
	}

	TESCameraController* TESCameraController::GetSingleton()
	{
		return (TESCameraController*)g_TESCameraControllerAddr;
	}

	MenuTopicManager* MenuTopicManager::GetSingleton()
	{
		return *(MenuTopicManager**)g_MenuTopicManagerAddr;
	}

	NiPointer<TESObjectREFR> MenuTopicManager::GetDialogueTarget()
	{
		NiPointer<TESObjectREFR> refr;
		if (talkingHandle != Tralala::InvalidRefHandle())
			LookupRefByHandle(talkingHandle, refr);

		if (!refr)
		{
			if (handle2 != Tralala::InvalidRefHandle())
				LookupRefByHandle(handle2, refr);
		}

		return refr;
	}

	StringCache::Ref::Ref()
	{
		typedef StringCache::Ref* (*ctor_t)(StringCache::Ref*, const char*);
		ctor_t ctor = (ctor_t)g_bsfixedstringCtorAddr;

		ctor(this, "");
	}

	StringCache::Ref::Ref(const char* buf)
	{
		typedef StringCache::Ref* (*ctor_t)(StringCache::Ref*, const char*);
		ctor_t ctor = (ctor_t)g_bsfixedstringCtorAddr;

		ctor(this, buf);
	}

	StringCache::Ref::~Ref()
	{
		Release();
	}

	void StringCache::Ref::Release()
	{
		typedef void(*dtor_t)(StringCache::Ref*);
		dtor_t dtor = (dtor_t)g_bsfixedstringDtorAddr;

		dtor(this);
	}

}