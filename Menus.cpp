#include "Menus.h"
#include "Utils.h"
#include "Settings.h"
#include "PatternScanner.h"

uintptr_t g_hudMenuNextFrameAddr = 0;

namespace Tralala
{
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

				if (mtm->unkB5)
					counter++;
				else
					counter = 0;

				if (mtm->isInDialogueState && (counter <= Settings::uDelay))
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
		
		return true;
	}
}