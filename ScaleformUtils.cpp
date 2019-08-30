#include "ScaleformUtils.h"
#include "PatternScanner.h"

namespace Tralala
{
	uintptr_t g_scaleFormHeapAddr = 0;
	uintptr_t g_addManagedAddr = 0;
	uintptr_t g_relManagedAddr = 0;
	uintptr_t g_getDispInfoAddr = 0;
	uintptr_t g_setDispInfoAddr = 0;

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

		if (IsManaged())
			AddManaged_Internal(objectInterface, this, data.obj);
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
}