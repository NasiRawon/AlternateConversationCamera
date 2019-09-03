#pragma once

#include <array>
#include <algorithm>
#include <string>
#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")

#define MAKE_RVA(base, address) ((uintptr_t)address - (uintptr_t)base)
#define APP_NAME "Alternate Conversation Camera"

template<std::size_t SIZE>
const BYTE* scan_memory_internal(void* address_low, std::size_t nbytes,
	const std::array<BYTE, SIZE>& bytes_to_find, BYTE offset, bool positive_offset)
{
	const BYTE* addresses_found = nullptr;

	MEMORY_BASIC_INFORMATION mbi{};

	BYTE* address = static_cast<BYTE*>(address_low);
	BYTE* address_high = address + nbytes;

	while (address < address_high && VirtualQuery(address, std::addressof(mbi), sizeof(mbi)))
	{
		// committed memory, readable, wont raise exception guard page
		if ((mbi.State == MEM_COMMIT) && (mbi.Protect & PAGE_EXECUTE_READ) && !(mbi.Protect & PAGE_GUARD))
		{
			const BYTE* begin = static_cast<const BYTE*>(mbi.BaseAddress);
			const BYTE* end = begin + mbi.RegionSize;

			const BYTE* found = std::search(begin, end, bytes_to_find.begin(), bytes_to_find.end());
			if (found != end)
			{
				addresses_found = found;
				break;
			}
		}

		address += mbi.RegionSize;
		mbi = {};
	}
	if (addresses_found)
	{
		if (positive_offset)
			addresses_found += offset;
		else
			addresses_found -= offset;
	}
	else
	{
		std::string errorMessage = "[ERROR] Unknown pattern: ";

		for (int i = 0; i < bytes_to_find.size(); i++)
		{
			auto IntToHexString = [](int val)->std::string
			{
				char buf[2 * _MAX_INT_DIG];

				sprintf_s(buf, sizeof(buf), "%02x", val);
				return (std::string(buf));
			};

			errorMessage += IntToHexString(bytes_to_find[i]);
		}

		MessageBoxA(NULL, errorMessage.c_str(), APP_NAME, MB_OK | MB_ICONERROR);
	}

	return addresses_found;
}


template<std::size_t SIZE>
const BYTE* scan_memory_data_internal(void* address_low, std::size_t nbytes,
	const std::array<BYTE, SIZE>& bytes_to_find, BYTE offset, bool positive_offset, BYTE start, BYTE size)
{
	const BYTE* addresses_found = nullptr;

	MEMORY_BASIC_INFORMATION mbi{};

	BYTE* address = static_cast<BYTE*>(address_low);
	BYTE* address_high = address + nbytes;

	while (address < address_high && VirtualQuery(address, std::addressof(mbi), sizeof(mbi)))
	{
		// committed memory, readable, wont raise exception guard page
		if ((mbi.State == MEM_COMMIT) && (mbi.Protect & PAGE_EXECUTE_READ) && !(mbi.Protect & PAGE_GUARD))
		{
			const BYTE* begin = static_cast<const BYTE*>(mbi.BaseAddress);
			const BYTE* end = begin + mbi.RegionSize;

			const BYTE* found = std::search(begin, end, bytes_to_find.begin(), bytes_to_find.end());
			if (found != end)
			{
				addresses_found = found;
				break;
			}
		}

		address += mbi.RegionSize;
		mbi = {};
	}
	if (addresses_found)
	{

		if (positive_offset)
			addresses_found += offset;
		else
			addresses_found -= offset;

		DWORD new_address = 0;
		BYTE new_size = size;

		if ((size - start) != 4)
			new_size = size - 1;		

		auto it = std::find(addresses_found, addresses_found + new_size, *(addresses_found + start));
		if (it != addresses_found + new_size)
		{
			for (int i = 0; i < (new_size - start); i++)
			{
				new_address += (*(it + i) << (i << 3));
			}

			addresses_found += (new_address + size);
			uintptr_t temp_address = MAKE_RVA(address_low, addresses_found);

			if ((temp_address & MAXDWORD) != temp_address)
				addresses_found -= 0x100000000;
		}
	}
	else
	{	
		std::string errorMessage = "[ERROR] Unknown pattern: ";

		for (int i = 0; i < bytes_to_find.size(); i++)
		{
			auto IntToHexString = [](int val)->std::string
			{
				char buf[2 * _MAX_INT_DIG];

				sprintf_s(buf, sizeof(buf), "%02x", val);
				return (std::string(buf));
			};

			errorMessage += IntToHexString(bytes_to_find[i]);
		}

		MessageBoxA(NULL, errorMessage.c_str(), APP_NAME, MB_OK | MB_ICONERROR);
	}

	return addresses_found;
}


template<std::size_t SIZE>
const BYTE* scan_memory(const std::array<BYTE, SIZE>& bytes_to_find, const BYTE offset, bool positive_offset)
{
	auto base = GetModuleHandle(0);
	if (base == nullptr)
		return {};

	MODULEINFO minfo {};
	GetModuleInformation(GetCurrentProcess(), base, std::addressof(minfo), sizeof(minfo));
	return scan_memory_internal(base, minfo.SizeOfImage, bytes_to_find, offset, positive_offset);
}

template<std::size_t SIZE>
const BYTE* scan_memory_data(const std::array<BYTE, SIZE>& bytes_to_find, const BYTE offset, 
	bool positive_offset, BYTE start, BYTE size)
{
	auto base = GetModuleHandle(0);
	if (base == nullptr)
		return{};

	MODULEINFO minfo{};
	GetModuleInformation(GetCurrentProcess(), base, std::addressof(minfo), sizeof(minfo));
	return scan_memory_data_internal(base, minfo.SizeOfImage, bytes_to_find, offset, positive_offset, start, size);
}