#include "ObjectRef.h"
#include "Menus.h"
#include "Utils.h"
#include "Settings.h"
#include "PatternScanner.h"

uintptr_t g_hudMenuNextFrameAddr = 0;
uintptr_t g_barterMenuDtorAddr = 0;
uintptr_t g_trainingMenuCtorAddr = 0;
uintptr_t g_trainingMenuDtorAddr = 0;


namespace Tralala
{
	bool g_isTrainingMenu = false;

	void HUDMenuNextFrame_Hook(Tralala::HUDMenu* hudMenu)
	{
		if (!Settings::bLetterBox)
			return;

		Tralala::GFxValue messagesBlock;

		if (hudMenu->hudBaseInstance.GetMember("MessagesBlock", &messagesBlock))
		{
			Tralala::GFxValue::DisplayInfo dispInfo;

			if (messagesBlock.GetDisplayInfo(&dispInfo))
			{
				static bool bInitDispInfo = false;
				static double oriPosX = 0.0f;
				static double oriPosY = 0.0f;

				if (!bInitDispInfo)
				{
					oriPosX = dispInfo._x;
					oriPosY = dispInfo._y;
					bInitDispInfo = true;
				}

				static UInt64 counter = 0;

				Tralala::MenuTopicManager* mtm = Tralala::MenuTopicManager::GetSingleton();
				if (!mtm)
					return;
				
				bool isDialogueMenuClose = *(bool*)g_isDialogueMenuCloseAddr;

				UInt32 containerHandle = *(UInt32*)g_containerHandle;
				UInt32 barterHandle = *(UInt32*)g_barterHandle;
				UInt32 activeHandle = containerHandle;
				if (barterHandle)
					activeHandle = barterHandle;

				if (mtm->isInDialogueState && !isDialogueMenuClose && !activeHandle && !g_isTrainingMenu)
				{
					if (Settings::fMessagePosX == -601.f)
						dispInfo.SetPosition(oriPosX, Settings::fMessagePosY);
					else
						dispInfo.SetPosition(Settings::fMessagePosX, Settings::fMessagePosY);

					messagesBlock.SetDisplayInfo(&dispInfo);
				}
				else
				{
					dispInfo.SetPosition(oriPosX, oriPosY);
					messagesBlock.SetDisplayInfo(&dispInfo);
				}
			}
		}
	}

	void BarterMenuDtor_Hook()
	{
		UInt32 invalidHandle = InvalidRefHandle();
		*(UInt32*)g_barterHandle = invalidHandle;
	}

	void TrainingMenuCtor_Hook()
	{
		g_isTrainingMenu = true;
	}

	void TrainingMenuDtor_Hook()
	{
		g_isTrainingMenu = false;
	}
}

#include "skse64_common/Utilities.h"
#include "skse64_common/Relocation.h"
#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/SafeWrite.h"
#include "xbyak/xbyak.h"

namespace Menus
{
	void GetAddresses()
	{
		const std::array<BYTE, 9> hudpattern = { 0x90, 0x89, 0x1C, 0x2F, 0x48, 0x8B, 0x5C, 0x24, 0x68 };
		g_hudMenuNextFrameAddr = (uintptr_t)scan_memory(hudpattern, 0x93, false);

		const std::array<BYTE, 9> barterpattern = { 0x90, 0x48, 0x83, 0xBF, 0x80, 0x00, 0x00, 0x00, 0x00 };
		g_barterMenuDtorAddr = (uintptr_t)scan_memory(barterpattern, 0x63, true);

		const std::array<BYTE, 8> traincpattern = { 0x83, 0x4B, 0x1C, 0x04, 0xC6, 0x43, 0x18, 0x03 };
		g_trainingMenuCtorAddr = (uintptr_t)scan_memory(traincpattern, 0x2C, true);

		const std::array<BYTE, 8> traindpattern = { 0x48, 0x89, 0x6B, 0x48, 0x48, 0x8B, 0x4B, 0x38 };
		g_trainingMenuDtorAddr = (uintptr_t)scan_memory(traindpattern, 0x37, true);
	}

	bool InstallHook()
	{
		{
			struct InstallHookHUDNextFrameCode : Xbyak::CodeGenerator {
				InstallHookHUDNextFrameCode(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;

					push(r9);
					push(r8);
					push(rdx);
					push(rcx);
					sub(rsp, 0x20);

					mov(rcx, rsi);
					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);
					pop(rcx);
					pop(rdx);
					pop(r8);
					pop(r9);

					mov(qword[rsp + 0x28], 0);

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_hudMenuNextFrameAddr + 0x5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookHUDNextFrameCode code(codeBuf, GetFnAddr(Tralala::HUDMenuNextFrame_Hook));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write5Branch(g_hudMenuNextFrameAddr, uintptr_t(code.getCode())))
				return false;

			SafeWrite32(g_hudMenuNextFrameAddr + 0x5, NOP32);
		}

		{
			struct InstallHookBarterDtorCode : Xbyak::CodeGenerator {
				InstallHookBarterDtorCode(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;

					push(r9);
					push(r8);
					push(rdx);
					push(rcx);
					sub(rsp, 0x20);

					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);
					pop(rcx);
					pop(rdx);
					pop(r8);
					pop(r9);

					mov(rbx, qword[rsp + 0x40]);

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_barterMenuDtorAddr + 0x5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookBarterDtorCode code(codeBuf, GetFnAddr(Tralala::BarterMenuDtor_Hook));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write5Branch(g_barterMenuDtorAddr, uintptr_t(code.getCode())))
				return false;
		}

		{
			struct InstallHookTrainCtorCode : Xbyak::CodeGenerator {
				InstallHookTrainCtorCode(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;

					push(r9);
					push(r8);
					push(rdx);
					push(rcx);
					sub(rsp, 0x20);

					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);
					pop(rcx);
					pop(rdx);
					pop(r8);
					pop(r9);

					mov(rax, rbx);
					mov(rbx, qword[rsp + 0x58]);

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_trainingMenuCtorAddr + 0x5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookTrainCtorCode code(codeBuf, GetFnAddr(Tralala::TrainingMenuCtor_Hook));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write5Branch(g_trainingMenuCtorAddr, uintptr_t(code.getCode())))
				return false;
		}

		{
			struct InstallHookTrainDtorCode : Xbyak::CodeGenerator {
				InstallHookTrainDtorCode(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;

					push(r9);
					push(r8);
					push(rdx);
					push(rcx);
					sub(rsp, 0x20);

					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);
					pop(rcx);
					pop(rdx);
					pop(r8);
					pop(r9);

					mov(rbx, qword[rsp + 0x40]);

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_trainingMenuDtorAddr + 0x5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookTrainDtorCode code(codeBuf, GetFnAddr(Tralala::TrainingMenuDtor_Hook));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write5Branch(g_trainingMenuDtorAddr, uintptr_t(code.getCode())))
				return false;
		}
		
		return true;
	}
}