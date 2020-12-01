#include "ScaleformUtils.h"
#include "PatternScanner.h"


namespace Tralala
{
	uintptr_t g_scaleFormHeapAddr = 0;
	uintptr_t g_addManagedAddr = 0;
	uintptr_t g_relManagedAddr = 0;
	uintptr_t g_getDispInfoAddr = 0;
	uintptr_t g_setDispInfoAddr = 0;
	uintptr_t g_getMemberAddr = 0;
	uintptr_t g_setMemberAddr = 0;
	uintptr_t g_setTextAddr = 0;


	void ScaleformUtilGetAddresses()
	{
		const std::array<BYTE, 7> heappattern = { 0x89, 0x45, 0xEF, 0xC6, 0x45, 0x67, 0x00 };
		g_scaleFormHeapAddr = (uintptr_t)scan_memory_data(heappattern, 0x2C, false, 0x3, 0x7);

		const std::array<BYTE, 7> addpattern = { 0x41, 0xFF, 0x00, 0x48, 0x83, 0xC4, 0x28 };
		g_addManagedAddr = (uintptr_t)scan_memory(addpattern, 0x22, false);

		const std::array<BYTE, 6> relpattern = { 0x8B, 0x42, 0x08, 0x49, 0x8B, 0xD8 };
		g_relManagedAddr = (uintptr_t)scan_memory(relpattern, 0x6, false);

		const std::array<BYTE, 6> getpattern = { 0x83, 0xF8, 0x04, 0x48, 0x8B, 0xCF };
		g_getDispInfoAddr = (uintptr_t)scan_memory(getpattern, 0x8C, false);

		const std::array<BYTE, 7> setpattern = { 0x41, 0x0F, 0x94, 0xC7, 0xC0, 0xE8, 0x05 };
		g_setDispInfoAddr = (uintptr_t)scan_memory(setpattern, 0xC4, false);

		const std::array<BYTE, 6> gmemberpattern = { 0x44, 0x8B, 0xE0, 0x49, 0x8B, 0xC6 };
		g_getMemberAddr = (uintptr_t)scan_memory_data(gmemberpattern, 0x1A, false, 0x1, 0x5);

		const std::array<BYTE, 8> smemberpattern = { 0xF3, 0x0F, 0xE6, 0xC0, 0x4C, 0x89, 0x75, 0x00 };
		g_setMemberAddr = (uintptr_t)scan_memory_data(smemberpattern, 0x2A, false, 0x1, 0x5);

		const std::array<BYTE, 7> textpattern = { 0xF3, 0x48, 0x0F, 0x2C, 0xC1, 0x2B, 0xC8 };
		g_setTextAddr = (uintptr_t)scan_memory_data(textpattern, 0x4D, true, 0x1, 0x5);
	}


	ScaleformHeap* ScaleformHeap::GetSingleton()
	{
		return *(ScaleformHeap * *)g_scaleFormHeapAddr;
	}


	void* ScaleformHeap_Allocate(UInt32 size)
	{
		return ScaleformHeap::GetSingleton()->Allocate(size);
	}


	void ScaleformHeap_Free(void* ptr)
	{
		return ScaleformHeap::GetSingleton()->Free(ptr);
	}


	GFxValue::~GFxValue()
	{
		CleanManaged();
	}


	void GFxValue::AddManaged()
	{
		typedef void(*AddManaged_Internal_t)(ObjectInterface*, GFxValue*, void*);
		AddManaged_Internal_t AddManaged_Internal = (AddManaged_Internal_t)g_addManagedAddr;

		if (IsManaged()) AddManaged_Internal(objectInterface, this, data.obj);
	}


	void GFxValue::AddManaged(const GFxValue& src)
	{
		if (IsManaged())
		{
			typedef void(*AddManaged_Internal_t)(ObjectInterface*, GFxValue*, void*);
			AddManaged_Internal_t AddManaged_Internal = (AddManaged_Internal_t)g_addManagedAddr;

			objectInterface = src.objectInterface;
			AddManaged_Internal(objectInterface, this, data.obj);
		}
	}


	void GFxValue::CleanManaged()
	{
		if (IsManaged())
		{
			typedef void(*ReleaseManaged_Internal_t)(ObjectInterface*, GFxValue*, void*);
			ReleaseManaged_Internal_t ReleaseManaged_Internal = (ReleaseManaged_Internal_t)g_relManagedAddr;

			ReleaseManaged_Internal(objectInterface, this, data.obj);

			objectInterface = nullptr;
			type = kType_Undefined;
		}
	}


	bool GFxValue::GetDisplayInfo(DisplayInfo* displayInfo)
	{
		typedef bool(*GetDisplayInfo_t)(ObjectInterface*, void*, DisplayInfo*);
		GetDisplayInfo_t GetDisplayInfo = (GetDisplayInfo_t)g_getDispInfoAddr;
		return GetDisplayInfo(objectInterface, data.obj, displayInfo);
	}


	bool GFxValue::SetDisplayInfo(DisplayInfo* displayInfo)
	{
		typedef bool(*SetDisplayInfo_t)(ObjectInterface*, void*, DisplayInfo*);
		SetDisplayInfo_t SetDisplayInfo = (SetDisplayInfo_t)g_setDispInfoAddr;
		return SetDisplayInfo(objectInterface, data.obj, displayInfo);
	}


	bool GFxValue::GetMember(const char* name, GFxValue* value)
	{
		typedef bool(*GetMember_t)(ObjectInterface*, void*, const char*, GFxValue*, bool);
		GetMember_t GetMember = (GetMember_t)g_getMemberAddr;
		return GetMember(objectInterface, data.obj, name, value, IsDisplayObject());
	}


	bool GFxValue::SetMember(const char* name, GFxValue* value)
	{
		typedef bool(*SetMember_t)(ObjectInterface*, void*, const char*, GFxValue*, bool);
		SetMember_t SetMember = (SetMember_t)g_setMemberAddr;
		return SetMember(objectInterface, data.obj, name, value, IsDisplayObject());
	}


	bool GFxValue::SetText(const char* text, bool html)
	{
		typedef bool(*SetText_t)(ObjectInterface*, void*, const char*, bool);
		SetText_t SetText = (SetText_t)g_setTextAddr;
		return SetText(objectInterface, data.obj, text, html);
	}


	void GFxValue::SetString(const char* value)
	{
		CleanManaged();

		type = kType_String;
		data.string = value;
	}


	void GFxValue::SetNumber(double value)
	{
		CleanManaged();

		type = kType_Number;
		data.number = value;
	}


	const char* GFxValue::GetString(void) const
	{
		if (GetType() != kType_String) return NULL;
		if (IsManaged()) return *data.managedString;
		else return data.string;
	}
}
