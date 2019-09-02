#include "skse64/PluginAPI.h"
#include "skse64_common/Relocation.h"
#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/SafeWrite.h"
#include "skse64/NiNodes.h"
#include "xbyak/xbyak.h"

#include "Camera.h"
#include "Controls.h"
#include "Utils.h"
#include "Settings.h"
#include "ObjectRef.h"
#include "Graphics.h"
#include "Menus.h"
#include "ScaleformUtils.h"

#include <shlobj.h>

#define PlayerRefHandle 0x100000

IDebugLog				gLog;
PluginHandle			g_pluginHandle = kPluginHandle_Invalid;

void * g_moduleHandle = nullptr;

const float PI = 3.1415927;

float g_firstDistance = 0.0f;
float g_thirdDistance = 0.0f;
float g_worldfovTo = 0.0f;
float g_firstfovTo = 0.0f;
float g_delay = 0.0f;
int g_fovStep = 0;
bool g_isNPC = false;

Tralala::Actor* g_refTarget = nullptr;

uintptr_t g_defaultWorldFOVAddr = 0;
uintptr_t g_defaultFirstFOVAddr = 0;
uintptr_t g_onCameraMoveAddr = 0;
uintptr_t g_disablePlayerLookAddr = 0;
uintptr_t g_dialogueMenuAddr = 0;
uintptr_t g_raceMenuAddr = 0;
uintptr_t g_processMovementAddr = 0;
uintptr_t g_minCurrentZoomAddr = 0;
uintptr_t g_zoomSpeedAddr = 0;
uintptr_t g_onPlayerDeathAddr = 0;
uintptr_t g_setObjectFadeAddr = 0;
uintptr_t g_freeLookAddr = 0;
uintptr_t g_firstFOVAddr = 0;
uintptr_t g_targetLocAddr = 0;
uintptr_t g_clearTargetAddr = 0;
uintptr_t g_updateActorAddr = 0;
uintptr_t g_actorPerFrameAddr = 0;
uintptr_t g_beginSaveAddr = 0;
uintptr_t g_finishSaveAddr = 0;
uintptr_t g_thirdCamHeightAddr = 0;
uintptr_t g_thirdCamColAddr = 0;

void MainGetAddresses()
{
	const std::array<BYTE, 10> worldFOVpattern = { 0x0F, 0x28, 0xC1, 0xF3, 0x0F, 0x5E, 0xC6, 0x0F, 0x28, 0xF0 };
	g_defaultWorldFOVAddr = (uintptr_t)scan_memory_data(worldFOVpattern, 0x5F, true, 0x4, 0x8);

	const std::array<BYTE, 10> firstFOVpattern = { 0x0F, 0x28, 0xC1, 0xF3, 0x0F, 0x5E, 0xC6, 0x0F, 0x28, 0xF0 };
	g_defaultFirstFOVAddr = (uintptr_t)scan_memory_data(firstFOVpattern, 0x6F, true, 0x4, 0x8);

	const std::array<BYTE, 6> minZoompattern = { 0x8B, 0x47, 0x58, 0x89, 0x47, 0x64 };
	g_minCurrentZoomAddr = (uintptr_t)scan_memory_data(minZoompattern, 0x6, true, 0x4, 0x8);

	const std::array<BYTE, 6> zoomSpeedpattern = { 0x0F, 0x28, 0xCB, 0x0F, 0x54, 0xCD };
	g_zoomSpeedAddr = (uintptr_t)scan_memory_data(zoomSpeedpattern, 0x17, false, 0x4, 0x8);

	const std::array<BYTE, 11> cameraMovepattern = { 0x4C, 0x8B, 0xC3, 0x48, 0x8B, 0xD7, 0xB9, 0x14, 0x00, 0x00, 0x00 };
	g_onCameraMoveAddr = (uintptr_t)scan_memory(cameraMovepattern, 0x6F, false);

	const std::array<BYTE, 7> lookpattern = { 0x48, 0x8B, 0x01, 0x4C, 0x8D, 0x43, 0x1C };
	g_disablePlayerLookAddr = (uintptr_t)scan_memory(lookpattern, 0x91, false);

	const std::array<BYTE, 7> dialoguepattern = { 0x48, 0x8B, 0x01, 0x4C, 0x8D, 0x43, 0x1C };
	g_dialogueMenuAddr = (uintptr_t)scan_memory(dialoguepattern, 0xB4, false);

	const std::array<BYTE, 7> racepattern = { 0x48, 0x8B, 0x01, 0x4C, 0x8D, 0x43, 0x1C };
	g_raceMenuAddr = (uintptr_t)scan_memory(racepattern, 0xBE, false);

	const std::array<BYTE, 6> processMovepattern = { 0x0F, 0x28, 0xF7, 0x48, 0x8B, 0xCB };
	g_processMovementAddr = (uintptr_t)scan_memory(processMovepattern, 0x4B, false);

	const std::array<BYTE, 11> playerDeathpattern = { 0x4C, 0x8B, 0xC3, 0x48, 0x8B, 0xD7, 0xB9, 0x14, 0x00, 0x00, 0x00 };
	g_onPlayerDeathAddr = (uintptr_t)scan_memory(playerDeathpattern, 0xD0, true);

	const std::array<BYTE, 8> setFadepattern = { 0xF3, 0x0F, 0x10, 0x55, 0x80, 0x0F, 0x28, 0xCA };
	g_setObjectFadeAddr = (uintptr_t)scan_memory(setFadepattern, 0xBD, true);

	const std::array<BYTE, 4> freeLookpattern = { 0x80, 0x79, 0x4B, 0x00 };
	g_freeLookAddr = (uintptr_t)scan_memory(freeLookpattern, 0x10, false);

	const std::array<BYTE, 10> patchFOVpattern = { 0x0F, 0x28, 0xC1, 0xF3, 0x0F, 0x5E, 0xC6, 0x0F, 0x28, 0xF0 };
	g_firstFOVAddr = (uintptr_t)scan_memory(patchFOVpattern, 0x55, true);

	const std::array<BYTE, 11> tgtLocpattern = { 0xC6, 0x45, 0x67, 0x00, 0x48, 0x8B, 0x86, 0xF0, 0x01, 0x00, 0x00 };
	g_targetLocAddr = (uintptr_t)scan_memory(tgtLocpattern, 0xA4, true);

	const std::array<BYTE, 6> clrTgtpattern = { 0x49, 0x8B, 0xC7, 0x0F, 0x57, 0xFF };
	g_clearTargetAddr = (uintptr_t)scan_memory(clrTgtpattern, 0x4A, false);

	g_actorPerFrameAddr = (uintptr_t)scan_memory_data(clrTgtpattern, 0x4A, false, 0x2, 0x6);

	g_updateActorAddr = (uintptr_t)scan_memory_data(clrTgtpattern, 0x44, false, 0x2, 0x6);

	const std::array<BYTE, 7> beginPattern = { 0x4C, 0x8D, 0x45, 0x20, 0x48, 0x8B, 0xD7 };
	g_beginSaveAddr = (uintptr_t)scan_memory(beginPattern, 0x64, false);

	const std::array<BYTE, 7> finishPattern = { 0x89, 0x45, 0xA4, 0x4C, 0x8D, 0x45, 0x20 };
	g_finishSaveAddr = (uintptr_t)scan_memory(finishPattern, 0x8D, true);

	const std::array<BYTE, 8> camHeightPattern = { 0xF3, 0x0F, 0x59, 0x5F, 0x18, 0x0F, 0x28, 0xC7 };
	g_thirdCamHeightAddr = (uintptr_t)scan_memory(camHeightPattern, 0x71, false);

	g_thirdCamColAddr = (uintptr_t)scan_memory(camHeightPattern, 0x7B, true);
}


static void SetZoom(float targetFOV)
{
	float mult = round(1.0f / *(float*)Tralala::g_deltaTimeAddr) / 60.0f;

	int step = Settings::fCameraSpeed * 3.6f * (*(float*)Tralala::g_deltaTimeAddr) * mult * mult;
	if (step <= 0) step = 1;
	g_worldfovTo = targetFOV;
	g_firstfovTo = targetFOV;
	g_fovStep = step;
}

static void ResetZoom()
{
	float mult = round(1.0f / *(float*)Tralala::g_deltaTimeAddr) / 60.0f;

	int step = Settings::fCameraSpeed * 3.6f * (*(float*)Tralala::g_deltaTimeAddr) * mult * mult;
	if (step <= 0) step = 1;
	g_worldfovTo = *(float*)g_defaultWorldFOVAddr;
	g_firstfovTo = *(float*)g_defaultFirstFOVAddr;
	g_fovStep = step;
}

static bool IsValidFaceToFace(Tralala::PlayerCamera * camera)
{
	if (!camera->IsCameraFirstPerson() && !camera->IsCameraThirdPerson())
		return false;

	if (Tralala::MenuControls::GetSingleton()->beastForm)
		return false;

	if (Tralala::PlayerCharacter::GetSingleton()->IsOnMount())
		return false;

	if (Tralala::PlayerCharacter::GetSingleton()->IsOnCarriage())
		return false;

	return true;
}


void DialogueMenuEventHandler(MenuOpenCloseEvent * evn)
{
	
	Tralala::PlayerCharacter * player = Tralala::PlayerCharacter::GetSingleton();
	Tralala::PlayerCamera * camera = Tralala::PlayerCamera::GetSingleton();

	static UInt32 cameraStateIDStarter = 0;
	static bool zoom = false;
	float minZoom = *(float*)g_minCurrentZoomAddr;

	Tralala::ThirdPersonState* tps = camera->GetThirdPersonCamera();

	if (evn->opening)
	{
		if (!IsValidFaceToFace(camera))
		{
			g_refTarget = nullptr;
			return;
		}

		Tralala::TESObjectREFR* target = nullptr;

		Tralala::MenuTopicManager * mm = Tralala::MenuTopicManager::GetSingleton();
		if (mm)
			target = mm->GetDialogueTarget();

		if (target && target->IsCharacter())
		{
			Tralala::Actor * actor = (Tralala::Actor*)target;

			if(Settings::bHeadTracking)
				player->processManager->SetDialogueHeadTrackingTarget(actor);

			float thisFOV = 0.0f;
			float distance = 0.0f;
			float prefDist = Settings::f1stZoom;
			g_delay = 75.0f; // init delay, so it won't instantly switch to the npc

			if (camera->IsCameraThirdPerson())
			{
				zoom = true;
				cameraStateIDStarter = Tralala::PlayerCamera::kCameraState_ThirdPerson2;

				if (Settings::bForceFirstPerson)
				{
					if (Settings::bSmoothTransition)
						tps->SetFirstPersonSmooth(minZoom);
					else
						camera->ForceFirstPerson();
				}
				else
				{
					NiPoint3 shoulderPos;
					shoulderPos.x = Settings::fAddOverShoulderPosX;
					shoulderPos.y = Settings::fAddOverShoulderPosY;
					shoulderPos.z = Settings::fAddOverShoulderPozZ;

					tps->SetShoulderPos(shoulderPos);
				}
			}
			else if (camera->IsCameraFirstPerson())
			{
				zoom = true;
				cameraStateIDStarter = Tralala::PlayerCamera::kCameraState_FirstPerson;
				if (Settings::bSwitchTarget && Settings::bForceThirdPerson)
				{
					camera->ForceThirdPerson(true);

					NiPoint3 shoulderPos;
					shoulderPos.x = Settings::fAddOverShoulderPosX;
					shoulderPos.y = Settings::fAddOverShoulderPosY;
					shoulderPos.z = Settings::fAddOverShoulderPozZ;

					tps->SetShoulderPos(shoulderPos);
					
				}		
			}

			if (camera->IsCameraFirstPerson() || Settings::bForceFirstPerson)
			{
				if (actor->IsFlyingActor()) // dragon has big head and absurd position xD
					prefDist = Settings::fDragonZoom;

				distance = camera->GetDistanceWithTargetBone(actor, true);
				thisFOV = round((atan(prefDist / distance) * 2.0f) * 180.0f / PI);
				if (thisFOV < 20.0f)
					thisFOV = 20.0f;

				SetZoom(thisFOV);

				g_firstDistance = distance;
				g_refTarget = actor;
			}
			else if (camera->IsCameraThirdPerson())
			{
				if (cameraStateIDStarter == Tralala::PlayerCamera::kCameraState_ThirdPerson2)
				{
					prefDist = Settings::f3rdZoom;

					if (actor->IsFlyingActor())
						prefDist = Settings::fDragonZoom;

					distance = camera->GetDistanceWithTargetBone(actor, false);
					thisFOV = round((atan(prefDist / distance) * 2.0f) * 180.0f / PI);
					if (thisFOV < 30.0f)
						thisFOV = 30.0f;

					SetZoom(thisFOV);

					g_thirdDistance = distance;
				}
				else
				{
					g_thirdDistance = 0.0f;
				}
	
				g_refTarget = actor;
			}
		}
	}
	else
	{
		g_refTarget = nullptr;

		if(Settings::bHeadTracking)
			player->processManager->ClearHeadTracking();

		if (zoom)
		{
			if (Settings::bSwitchTarget)
			{
				g_delay = 0.0f;
				camera->SetCameraTarget(player);
			}
				
			zoom = false;
			if (camera->IsCameraFirstPerson() || camera->IsCameraThirdPerson())
			{
				if (cameraStateIDStarter == Tralala::PlayerCamera::kCameraState_ThirdPerson2)
				{
					if (player->IsNotInFurniture() && camera->IsCameraThirdPerson())
					{
						if (!Settings::bSwitchTarget)
							player->rot.z = camera->camRotZ;

						player->rot.x -= tps->diffRotX;

						tps->diffRotX = tps->diffRotZ = 0.0f;
					}

					tps->UpdateMode(!camera->isWeapSheathed);

					if (camera->IsCameraFirstPerson())
						camera->ForceThirdPerson(true);

				}
				else
				{
					if (Settings::bSwitchTarget && Settings::bForceThirdPerson && camera->IsCameraThirdPerson())
					{
						tps->UpdateMode(!camera->isWeapSheathed);
						tps->SetFirstPersonSmooth(minZoom, true);					
					}
				}
			}

			ResetZoom();
		}
	}
}

void MenuEventHandler(MenuOpenCloseEvent * evn) 
{
	if (evn->opening)
	{
		Tralala::BSFixedString racesexMenu("RaceSex Menu");
		
		if (evn->menuName == racesexMenu) // fix for face sculptor
		{
			Tralala::PlayerCamera* camera = Tralala::PlayerCamera::GetSingleton();
			if (camera)
			{
				camera->worldFOV = *(float*)g_defaultWorldFOVAddr;
				camera->firstPersonFOV = *(float*)g_defaultFirstFOVAddr;
			}
		}

		racesexMenu.Release();
	}
}

float RotateCamera(Tralala::PlayerCamera * camera, Tralala::Actor* source, Tralala::Actor* target, Tralala::TESCameraController* controller, bool calcCrosshairToBoneMag)
{
	NiPoint3 pos;
	if (!camera->GetDistanceWithTargetBone(target, &pos))
		return 0.0f;

	controller->unk1C = 1;

	float xy, rotZ, rotX = 0.0f;

	xy = sqrt(pos.x*pos.x + pos.y*pos.y);
	rotZ = atan2(pos.x, pos.y);
	rotX = atan2(-pos.z, xy);

	float diffAngleZ, diffAngleX = 0.0f;

	Tralala::FirstPersonState* fps = camera->GetFirstPersonCamera();
	Tralala::ThirdPersonState* tps = camera->GetThirdPersonCamera();

	if (source->IsNotInFurniture())
	{
		if (camera->IsCameraThirdPerson())
		{
			float angleDiffZ, angleDiffX;
			{
				tps->diffRotZ = 0.0f;

				angleDiffZ = rotZ - source->rot.z;
				angleDiffX = rotX - (source->rot.x - tps->diffRotX);

				if (Settings::bForceFirstPerson)
				{
					float zoomSpeed = *(float*)g_zoomSpeedAddr;
					float deltaTime = *(float*)Tralala::g_deltaTimeAddr;
					float nextZoom = tps->curPosY - (zoomSpeed*deltaTime);

					if (nextZoom < tps->dstPosY)
					{
						source->rot.x -= tps->diffRotX;

						tps->diffRotX = tps->diffRotZ = 0.0f;
					}
				}
				
				while (angleDiffZ < -PI)
					angleDiffZ += 2.0f * PI;
				while (angleDiffZ > PI)
					angleDiffZ -= 2.0f * PI;
				while (angleDiffX < -PI)
					angleDiffX += 2.0f * PI;
				while (angleDiffX > PI)
					angleDiffX -= 2.0f * PI;
			}

			float mult = round(1.0f / *(float*)Tralala::g_deltaTimeAddr) / 60.0f;

			angleDiffZ = angleDiffZ / (Settings::fCameraSpeed * 1.8f * (*(float*)Tralala::g_deltaTimeAddr) * mult * mult);
			angleDiffX = angleDiffX / (Settings::fCameraSpeed * 1.8f * (*(float*)Tralala::g_deltaTimeAddr) * mult * mult);

			float angleZ, angleX;
			{
				angleZ = source->rot.z + angleDiffZ;
				angleX = source->rot.x + angleDiffX;

				while (angleZ < 0.0f)
					angleZ += 2.0f * PI;
				while (angleZ > 2.0f * PI)
					angleZ -= 2.0f * PI;
				while (angleX < -PI)
					angleX += 2.0f * PI;
				while (angleX > PI)
					angleX -= 2.0f * PI;
			}

			source->SetAngleZ(angleZ);
			source->SetAngleX(angleX);

			if (calcCrosshairToBoneMag)
			{
				diffAngleZ = angleDiffZ;
				diffAngleX = angleDiffX;
			}
		}
		else if (camera->IsCameraFirstPerson())
		{
			float angleDiffZ, angleDiffX;
			{
				angleDiffZ = rotZ - source->rot.z;
				angleDiffX = rotX - source->rot.x;

				while (angleDiffZ < -PI)
					angleDiffZ += 2.0f * PI;
				while (angleDiffZ > PI)
					angleDiffZ -= 2.0f * PI;
				while (angleDiffX < -PI)
					angleDiffX += 2.0f * PI;
				while (angleDiffX > PI)
					angleDiffX -= 2.0f * PI;
			}

			float mult = round(1.0f / *(float*)Tralala::g_deltaTimeAddr) / 60.0f;

			angleDiffZ = angleDiffZ / (Settings::fCameraSpeed * 1.8f * (*(float*)Tralala::g_deltaTimeAddr) * mult * mult);
			angleDiffX = angleDiffX / (Settings::fCameraSpeed * 1.8f * (*(float*)Tralala::g_deltaTimeAddr) * mult * mult);

			float angleZ, angleX;
			{
				angleZ = source->rot.z + angleDiffZ;
				angleX = source->rot.x + angleDiffX;

				while (angleZ < 0.0f)
					angleZ += 2.0f * PI;
				while (angleZ > 2.0f * PI)
					angleZ -= 2.0f * PI;
				while (angleX < -PI)
					angleX += 2.0f * PI;
				while (angleX > PI)
					angleX -= 2.0f * PI;
			}

			controller->startRotZ = source->rot.z;
			controller->startRotX = source->rot.x;
			controller->endRotZ = angleZ;
			controller->endRotX = angleX;
			controller->unk14 = Settings::fCameraSpeed;
			controller->unk1C = 0;

			if (calcCrosshairToBoneMag)
			{
				diffAngleZ = angleDiffZ;
				diffAngleX = angleDiffX;
			}
		}
	}
	else
	{
		if (camera->IsCameraFirstPerson())
		{
			float angleDiffZ, angleDiffX;
			{
				angleDiffZ = rotZ - camera->camRotZ;
				angleDiffX = rotX - source->rot.x;

				while (angleDiffZ < -PI)
					angleDiffZ += 2.0f * PI;
				while (angleDiffZ > PI)
					angleDiffZ -= 2.0f * PI;
				while (angleDiffX < -PI)
					angleDiffX += 2.0f * PI;
				while (angleDiffX > PI)
					angleDiffX -= 2.0f * PI;
			}

			float mult = round(1.0f / *(float*)Tralala::g_deltaTimeAddr) / 60.0f;

			angleDiffZ = angleDiffZ / (Settings::fCameraSpeed * 1.8f * (*(float*)Tralala::g_deltaTimeAddr) * mult * mult);
			angleDiffX = angleDiffX / (Settings::fCameraSpeed * 1.8f * (*(float*)Tralala::g_deltaTimeAddr) * mult * mult);

			float angleX;
			{
				angleX = source->rot.x + angleDiffX;

				while (angleX < -PI)
					angleX += 2.0f * PI;
				while (angleX > PI)
					angleX -= 2.0f * PI;
			}

			source->SetAngleX(angleX);
			diffAngleX = angleDiffX;

			if (fps->sittingRot < 1.56f && fps->sittingRot > -1.56f) // PI / 2 but I use rounded value
			{
				fps->sittingRot += angleDiffZ;
				if (calcCrosshairToBoneMag)
				{
					if (sqrt(angleDiffX*angleDiffX + angleDiffZ*angleDiffZ) <= 0.0005f)
						diffAngleZ = 0.0f;
					else
						diffAngleZ = angleDiffZ;
				}
			}
			else
			{
				fps->sittingRot += 0.0f;
				if (calcCrosshairToBoneMag)
					diffAngleZ = 0.0f;
			}
		}
		else if (camera->IsCameraThirdPerson())
		{
			float angleDiffZ, angleDiffX;
			{
				angleDiffZ = rotZ - camera->camRotZ;
				angleDiffX = atan2(pos.z, xy) - (tps->diffRotX - source->rot.x);

				while (angleDiffZ < -PI)
					angleDiffZ += 2.0f * PI;
				while (angleDiffZ > PI)
					angleDiffZ -= 2.0f * PI;
				while (angleDiffX < -PI)
					angleDiffX += 2.0f * PI;
				while (angleDiffX > PI)
					angleDiffX -= 2.0f * PI;
			}

			float mult = round(1.0f / *(float*)Tralala::g_deltaTimeAddr) / 60.0f;

			angleDiffZ = angleDiffZ / (Settings::fCameraSpeed * 1.8f * (*(float*)Tralala::g_deltaTimeAddr) * mult * mult);
			angleDiffX = angleDiffX / (Settings::fCameraSpeed * 1.8f * (*(float*)Tralala::g_deltaTimeAddr) * mult * mult);

			tps->diffRotZ += angleDiffZ;
			tps->diffRotX += angleDiffX;

			if (calcCrosshairToBoneMag)
			{
				diffAngleZ = angleDiffZ;
				diffAngleX = angleDiffX;
			}	
		}
	}

	float crosshairDist = 0.0f;

	if (calcCrosshairToBoneMag)
		crosshairDist = sqrt(diffAngleZ*diffAngleZ + diffAngleX*diffAngleX);

	return crosshairDist;
}

TESObjectWEAP * OnCameraMove(Tralala::PlayerCharacter * player, bool isLeftHand)
{
	Tralala::PlayerCamera* camera = Tralala::PlayerCamera::GetSingleton();
	Tralala::TESCameraController * controller = Tralala::TESCameraController::GetSingleton();
	Tralala::MenuTopicManager* mtm = Tralala::MenuTopicManager::GetSingleton();

	if (camera->IsCameraFirstPerson() || camera->IsCameraThirdPerson())
	{
		if (Settings::bHeadTracking && mtm) 
		{
			Tralala::BSFixedString isNPCVar("IsNPC");

			if (mtm->isInDialogueState)
				player->animGraphHolder.SetAnimationVariableBool(isNPCVar, true);
			else
			{
				UInt32 attackState = player->actorState.flags08 >> 0x1C; 
				switch (attackState)
				{
				case 0x1: // fix for melee
				case 0x2:
				case 0x3:
				case 0x4:
				case 0x8: // fix for bow
				case 0x9:
				case 0xA:
					player->animGraphHolder.SetAnimationVariableBool(isNPCVar, false);
					break;
				}

				if (player->IsCasting()) // fix for spell
					player->animGraphHolder.SetAnimationVariableBool(isNPCVar, false);
			}

			isNPCVar.Release();
		}
			

		if (g_fovStep > 0)
		{
			if (g_fovStep == 1)
			{
				camera->worldFOV = g_worldfovTo;;
				camera->firstPersonFOV = g_firstfovTo;
			}
			else
			{
				float diff = g_worldfovTo - camera->worldFOV;
				camera->worldFOV += diff / g_fovStep;
				float diff2 = g_firstfovTo - camera->firstPersonFOV;
				camera->firstPersonFOV += diff2 / g_fovStep;
			}

			if (g_refTarget && camera->cameraRefHandle == PlayerRefHandle)
				RotateCamera(camera, player, g_refTarget, controller, false);

			g_fovStep--;
		}
		else
		{
			if (g_refTarget)
			{
				if (Settings::bLockOn || mtm->talkingHandle == 0)
				{
					static Tralala::Actor* playerAsTarget = nullptr;
					static bool switchReady = false;

					float prefDist = Settings::f1stZoom;

					if (Settings::bForceFirstPerson || camera->IsCameraFirstPerson())
					{
						if (g_refTarget->IsFlyingActor())
							prefDist = Settings::fDragonZoom;

						float distance = camera->GetDistanceWithTargetBone(g_refTarget, true);

						if (abs(g_firstDistance - distance) >= 5.0f)
						{
							float newFOV = round((atan(prefDist / distance) * 2.0f) * 180.0f / PI);
							if (newFOV < 20.0f)
								newFOV = 20.0f;

							SetZoom(newFOV);
							g_firstDistance = distance;
						}

						playerAsTarget = nullptr;
					}
					else
					{
						if (Settings::bSwitchTarget)
						{
							Tralala::ThirdPersonState * tps = camera->GetThirdPersonCamera();

							if (g_thirdDistance == 0 && tps->curPosY == tps->dstPosY)
							{
								prefDist = Settings::f3rdZoom;

								if (g_refTarget->IsFlyingActor())
									prefDist = Settings::fDragonZoom;

								float distance = camera->GetDistanceWithTargetBone(g_refTarget, false);

								float thisFOV = round((atan(prefDist / distance) * 2.0f) * 180.0f / PI);
								if (thisFOV < 30.0f)
									thisFOV = 30.0f;

								SetZoom(thisFOV);
								g_thirdDistance = distance;
							}
							else
							{
								if (tps->curPosY == tps->dstPosY)
								{
									if (g_refTarget->IsTalking() || mtm->unk70)
									{
										camera->SetCameraTarget(player);

										if (g_delay == 75.0f) // init
										{
											g_delay = 50.0f;
											switchReady = false;
										}							
										else if (g_delay == 50.0f)
										{
											g_delay = 50.0f;
											switchReady = false;
										}	
										else // normal				
										{
											g_delay = 20.0f;
											switchReady = true;
										}	
									}
									else
									{
										float delay = (1.0f / 0.016667f) * (*(float*)Tralala::g_deltaTimeAddr);

										if (g_delay > 0.0f)
											g_delay -= delay;
										if (g_delay < 0.0f)
											g_delay = 0.0f;

										if (switchReady && camera->cameraRefHandle == PlayerRefHandle 
											&& g_delay == 0.0f)
										{
											camera->SetCameraTarget(g_refTarget);
											playerAsTarget = player;
											switchReady = false;
										}
									}
								}
							}
						}
						else
						{
							prefDist = Settings::f3rdZoom;

							if (g_refTarget->IsFlyingActor())
								prefDist = Settings::fDragonZoom;

							float distance = camera->GetDistanceWithTargetBone(g_refTarget, false);

							if (abs(g_thirdDistance - distance) >= 5.0f)
							{
								float newFOV = round((atan(prefDist / distance) * 2.0f) * 180.0f / PI);
								if (newFOV < 30.0f)
									newFOV = 30.0f;

								SetZoom(newFOV);
								g_thirdDistance = distance;
							}
						}
					}

					if (camera->cameraRefHandle == PlayerRefHandle)
					{
						if (Settings::bSwitchTarget)
						{
							static float targetDist = 0.0f;

							if (Settings::bForceThirdPerson)
							{
								targetDist = RotateCamera(camera, player, g_refTarget, controller, true);
							}
							else
							{
								if (camera->IsCameraFirstPerson())
									RotateCamera(camera, player, g_refTarget, controller, false);
								else
									targetDist = RotateCamera(camera, player, g_refTarget, controller, true);
							}

							if (targetDist <= 0.001f)
								switchReady = true;
						}
						else
						{
							RotateCamera(camera, player, g_refTarget, controller, false);
						}
					}
					else
					{
						if (playerAsTarget)
						{
							static float playerNeckDist = 0.0f;

							playerNeckDist = RotateCamera(camera, g_refTarget, playerAsTarget, controller, true);
							if (playerNeckDist <= 0.0005f)
								playerAsTarget = nullptr;
						}
					}	
				}
				else
				{
					float crosshairDist = RotateCamera(camera, player, g_refTarget, controller, true);
					if (crosshairDist <= 0.0005f)
						g_refTarget = nullptr;
				}
			}
		}
	}

	return player->GetEquippedWeapon(isLeftHand);
}

void OnPlayerDeath()
{
	g_refTarget = nullptr;
	Tralala::PlayerCamera * camera = Tralala::PlayerCamera::GetSingleton();
	if (camera)
	{
		camera->worldFOV = *(float*)g_defaultWorldFOVAddr;
		camera->firstPersonFOV = *(float*)g_defaultFirstFOVAddr;

		Tralala::PlayerCharacter * player = Tralala::PlayerCharacter::GetSingleton();
		if (Settings::bSwitchTarget)
			camera->SetCameraTarget(player);
	}
}

float GetThirdPersonCameraHeight_Hook(Tralala::Actor * actor)
{
	if (!Tralala::MenuTopicManager::GetSingleton()->isInDialogueState)
		return actor->GetCameraHeight();

	float headPos = 0.0f;

	actor->GetTargetHeadNodePosition(&headPos);
	if (headPos == 0.0f)
		return actor->GetCameraHeight();

	return (headPos - actor->pos.z);
}

void TPCamProcessCollision_Hook(Tralala::ThirdPersonState* tps)
{

	return tps->ProcessCameraCollision();

	//WIP, 3rd person camera collision to improve the switch pov feature
#if 0
	Tralala::PlayerCamera * camera = (Tralala::PlayerCamera*)tps->camera;

	if (Settings::bSwitchTarget && Tralala::MenuTopicManager::GetSingleton()->isInDialogueState
		&& camera->cameraRefHandle != PlayerRefHandle && g_refTarget)
	{
		UInt32 furnHandle = 0;

		g_refTarget->processManager->GetFurnitureHandle(&furnHandle);
		if (furnHandle != Tralala::InvalidRefHandle())
		{
			Tralala::TESObjectREFR* ref = nullptr;
			Tralala::LookupRefByHandle(&furnHandle, &ref);

			if (ref)
			{
				float furnX = ref->pos.x;
				float furnY = ref->pos.y;
				float furnZ = ref->pos.z;

				float camX = tps->camPos.x;
				float camY = tps->camPos.y;
				float camZ = tps->camPos.z;

				float furnWidth = ref->GetTargetWidth();
				float furnHeight = ref->GetTargetHeight();

				_MESSAGE("furn X %f Y %f Z %f", furnX, furnY, furnZ);
				_MESSAGE("cam X %f Y %f Z %f", camX, camY, camZ);
				_MESSAGE("furn Width %f Height %f", furnWidth, furnHeight);

				ref->handleRefObject.DecRef();

				if ((camX - furnX) > (furnWidth / 2))
				{
					_MESSAGE("Width Collision off");
					return;
				}
				else if ((furnZ + furnHeight) < (camZ + 5))
				{
					_MESSAGE("Height Collision off");
					return;
				}
				else
				{
					return tps->ProcessCameraCollision();
				}
			}
		}
		else
		{
			return tps->ProcessCameraCollision();
		}	
	}
	else
	{
		return tps->ProcessCameraCollision();
	}
#endif
}

Tralala::Actor * SetObjectFade(Tralala::Actor * actor)
{
	Tralala::PlayerCamera * camera = Tralala::PlayerCamera::GetSingleton();
	Tralala::ThirdPersonState * tps = camera->GetThirdPersonCamera();

	if (Settings::bSwitchTarget && Tralala::MenuTopicManager::GetSingleton()->isInDialogueState
		&& camera->cameraRefHandle != PlayerRefHandle && camera->cameraState == tps && actor != g_refTarget)
	{

		float dist = tps->GetDistanceWithinTargetHead(g_refTarget);

		if (dist <= 125.0f)
			return g_refTarget;
	}

	return actor;
}

bool SetFreeLook(Tralala::ThirdPersonState * tps, bool freeLook)
{
	Tralala::PlayerCamera * camera = (Tralala::PlayerCamera*)tps->camera;

	if (Settings::bSwitchTarget && Tralala::MenuTopicManager::GetSingleton()->isInDialogueState
		&& g_refTarget && !camera->isWeapSheathed)
	{
		freeLook = true;
	}

	return freeLook;
}

void SetTargetLocationHook(Tralala::ActorProcessManager* apm, Tralala::TESObjectREFR* source, NiPoint3* location)
{

	if (!Settings::bHeadTracking)
		return apm->SetTargetLocation(source, location);

	Tralala::MenuTopicManager* mtm = Tralala::MenuTopicManager::GetSingleton();

	if (source != Tralala::PlayerCharacter::GetSingleton())
		return apm->SetTargetLocation(source, location);

	if (!mtm->isInDialogueState)
		return apm->SetTargetLocation(source, location);

	Tralala::TESObjectREFR* target = mtm->GetDialogueTarget();
	if (!target || (!target->IsCharacter()))
		return apm->SetTargetLocation(source, location);

	NiPoint3 pos;

	Tralala::Actor* actor = (Tralala::Actor*)target;
	actor->GetTargetBonePosition(&pos);

	return apm->SetTargetLocation(source, &pos);
}

bool ClearHeadTrackingTargetHook(Tralala::Actor* source)
{

	SInt32 updateActorPerFrame = *(SInt32*)g_actorPerFrameAddr;
	if (*(SInt32*)g_updateActorAddr > updateActorPerFrame)
	{
		return true;
	}
	else
	{
		Tralala::PlayerCharacter* player = Tralala::PlayerCharacter::GetSingleton();
		Tralala::MenuTopicManager* mtm = Tralala::MenuTopicManager::GetSingleton();

		if (source == player)
		{
			if (Settings::bHeadTracking && Settings::bConversationHT && !mtm->isInDialogueState)
				return true; 
		}

		return false;
	}	
}

void BeginSavingHook()
{
	Tralala::PlayerCharacter* player = Tralala::PlayerCharacter::GetSingleton();

	if (player)
	{
		g_isNPC = player->GetIsNPCAnimVar();

		if (player->SetIsNPCAnimVar())
			_MESSAGE("Reset IsNPC animation variable.");
	}
}

void FinishSavingHook()
{
	Tralala::PlayerCharacter* player = Tralala::PlayerCharacter::GetSingleton();

	if (player && g_isNPC)
		player->SetIsNPCAnimVar(true);
}

extern "C"
{

	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{

		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\Alternate Conversation Camera.log");

		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "Alternate Conversation Camera";
		info->version = 1;

		g_pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor)
		{
			_MESSAGE("loaded in editor, marking as incompatible");
			return false;
		}

		if (!g_branchTrampoline.Create(1024 * 64))
		{
			_ERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
			return false;
		}

		if (!g_localTrampoline.Create(1024 * 64, g_moduleHandle))
		{
			_ERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
			return false;
		}

		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)
	{
		_MESSAGE("Load");
		
		MainGetAddresses();
		Tralala::UtilsGetAddresses();
		Tralala::PlayerCameraGetAddress();
		Tralala::ControlsGetAddresses();
		Tralala::ObjectRefGetAddresses();
		Tralala::ScaleformUtilGetAddresses();
		Menus::GetAddresses();
		Graphics::GetAddresses();

		Settings::Load();
		{
			struct InstallHookDialogueMenu_Code : Xbyak::CodeGenerator {
				InstallHookDialogueMenu_Code(void * buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel1;
					Xbyak::Label retnLabel2;
					Xbyak::Label funcLabel;
					Xbyak::Label dialogueClose;

					push(rcx);
					push(rdx);
					push(r8);
					push(r9);
					sub(rsp, 0x20);

					mov(rcx, rdx);
					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);
					pop(r9);
					pop(r8);
					pop(rdx);
					pop(rcx);

					cmp(byte[rdx + 0x8], 0);
					je(dialogueClose);

					jmp(ptr[rip + retnLabel1]);

					L(funcLabel);
					dq(funcAddr);

					L(dialogueClose);
					jmp(ptr[rip + retnLabel2]);

					L(retnLabel1);
					dq(g_dialogueMenuAddr + 0x6);

					L(retnLabel2);
					dq(g_dialogueMenuAddr + 0xD0);
				}
			};

			void * codeBuf = g_localTrampoline.StartAlloc();
			InstallHookDialogueMenu_Code code(codeBuf, GetFnAddr(DialogueMenuEventHandler));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write6Branch(g_dialogueMenuAddr, uintptr_t(code.getCode())))
				return false;
		}
		
		{
			struct InstallHookMenuEventHandler_Code : Xbyak::CodeGenerator {
				InstallHookMenuEventHandler_Code(void * buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel1;
					Xbyak::Label retnLabel2;
					Xbyak::Label funcLabel;
					Xbyak::Label notdialogueMenu;

					push(rcx);
					push(rdx);
					push(r8);
					push(r9);
					sub(rsp, 0x20);

					mov(rcx, rdx);
					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);
					pop(r9);
					pop(r8);
					pop(rdx);
					pop(rcx);

					cmp(qword[rdx], r8);
					jne(notdialogueMenu);

					jmp(ptr[rip + retnLabel1]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel1);
					dq(g_raceMenuAddr + 0x6);

					L(notdialogueMenu);
					jmp(ptr[rip + retnLabel2]);

					L(retnLabel2);
					dq(g_raceMenuAddr + 0xED);
				}
			};

			void * codeBuf = g_localTrampoline.StartAlloc();
			InstallHookMenuEventHandler_Code code(codeBuf, GetFnAddr(MenuEventHandler));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write6Branch(g_raceMenuAddr, uintptr_t(code.getCode())))
				return false;
		}
		
		{
			struct InstallHookOnCameraMove_Code : Xbyak::CodeGenerator {
				InstallHookOnCameraMove_Code(void * buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;

					sub(rsp, 0x20);

					xor(edx, edx);
					mov(rcx, rdi);
					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_onCameraMoveAddr + 0x5);
				}
			};

			void * codeBuf = g_localTrampoline.StartAlloc();
			InstallHookOnCameraMove_Code code(codeBuf, GetFnAddr(OnCameraMove));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write5Branch(g_onCameraMoveAddr, uintptr_t(code.getCode())))
				return false;
		}

		{
			struct InstallHookDisablePlayerLook_Code : Xbyak::CodeGenerator {
				InstallHookDisablePlayerLook_Code(void * buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label normal;
					Xbyak::Label disabled;
					Xbyak::Label normPopStack;
					Xbyak::Label disPopStack;
					Xbyak::Label funcLabel;
					Xbyak::Label retn;

					push(rax);
					push(rcx);

					mov(cl, al);

					mov(rax, Settings::bLockOn);
					test(al, al);
					je(normal);

					push(rcx);
					sub(rsp, 0x28);

					mov(rax, Tralala::g_playerCameraAddr);
					mov(rcx, ptr[rax]);
					call(ptr[rip + funcLabel]);

					add(rsp, 0x28);
					pop(rcx);

					test(al, al);
					je(normal);

					push(rcx);
					push(rdx);
					mov(rcx, Settings::bSwitchTarget);
					mov(rdx, Settings::bForceThirdPerson);

					test(cl, dl);
					jne(normPopStack);

					test(cl, cl);
					je(disPopStack);

					mov(rax, Tralala::g_playerCameraAddr);
					mov(rax, ptr[rax]);
					mov(rax, ptr[rax + 0x28]);
					mov(eax, dword[rax + 0x18]);
					test(eax, eax);
					je(disPopStack);

					L(normPopStack);
					pop(rdx);
					pop(rcx);
					jmp(normal);

					L(disPopStack);
					pop(rdx);
					pop(rcx);

					L(disabled);
					mov(rax, Tralala::g_playerControlsAddr);
					mov(rax, ptr[rax]);
					mov(byte[rax + 0x1D8], 0x0);
					pop(rcx);
					pop(rax);
					jmp(ptr[rip + retn]);
					
					L(normal);
					mov(rax, Tralala::g_playerControlsAddr);
					mov(rax, ptr[rax]);
					mov(byte[rax + 0x1D8], cl);
					pop(rcx);
					pop(rax);
					jmp(ptr[rip + retn]);

					L(retn);
					dq(g_disablePlayerLookAddr + 0x6);
					
					L(funcLabel);
					dq(funcAddr);
				}
			};

			void * codeBuf = g_localTrampoline.StartAlloc();
			InstallHookDisablePlayerLook_Code code(codeBuf, GetFnAddr(IsValidFaceToFace));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write6Branch(g_disablePlayerLookAddr, uintptr_t(code.getCode())))
				return false;
		}
		
		{
			struct InstallHookProcessMovement_Code : Xbyak::CodeGenerator {
				InstallHookProcessMovement_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label process;
					Xbyak::Label stop;
					Xbyak::Label skip;
					Xbyak::Label retn1;
					Xbyak::Label retn2;

					push(rax);
					mov(rax, Settings::bStopNPCMove);
					test(al, al);
					je(process);

					mov(rax, Tralala::g_MenuTopicManagerAddr);
					mov(rax, ptr[rax]);
					mov(rax, byte[rax + 0xB1]); // is in dialogue menu
					test(al, al);
					jne(stop);

					L(process);
					pop(rax);
					test(al, al);
					jne(skip);
					jmp(ptr[rip + retn1]);

					L(stop);
					pop(rax);

					L(skip);
					jmp(ptr[rip + retn2]);

					L(retn1);
					dq(g_processMovementAddr + 0x6);

					L(retn2);
					dq(g_processMovementAddr + 0x18E);
				}
			};

			void * codeBuf = g_localTrampoline.StartAlloc();
			InstallHookProcessMovement_Code code(codeBuf);
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write6Branch(g_processMovementAddr, uintptr_t(code.getCode())))
				return false;
		}
		
		{
			struct InstallHookOnPlayerDeath_Code : Xbyak::CodeGenerator {
				InstallHookOnPlayerDeath_Code(void * buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label funcLabel;
					Xbyak::Label skip;
					Xbyak::Label retn1;
					Xbyak::Label retn2;

					test(al, al);
					je(skip);

					push(rcx);
					push(rdx);
					push(r8);
					push(r9);
					sub(rsp, 0x20);

					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);
					pop(r9);
					pop(r8);
					pop(rdx);
					pop(rcx);

					jmp(ptr[rip + retn2]);

					L(funcLabel);
					dq(funcAddr);

					L(skip);
					jmp(ptr[rip + retn1]);

					L(retn1);
					dq(g_onPlayerDeathAddr + 0x6);

					L(retn2);
					dq(g_onPlayerDeathAddr + 0x856);
				}
			};

			void * codeBuf = g_localTrampoline.StartAlloc();
			InstallHookOnPlayerDeath_Code code(codeBuf, GetFnAddr(OnPlayerDeath));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write6Branch(g_onPlayerDeathAddr, uintptr_t(code.getCode())))
				return false;
		}

		{
			struct InstallHookSetObjectFade_Code : Xbyak::CodeGenerator {
				InstallHookSetObjectFade_Code(void * buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;
					
					push(rcx);
					push(rdx);
					push(r8);
					push(r9);
					sub(rsp, 0x20);

					mov(rcx, ptr[rsp + 0x90]);
					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);
					pop(r9);
					pop(r8);
					pop(rdx);
					pop(rcx);

					mov(ptr[rsp + 0x50], rax);
					mov(rdx, ptr[rsp + 0x50]);

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_setObjectFadeAddr + 0x5);
				}
			};

			void * codeBuf = g_localTrampoline.StartAlloc();
			InstallHookSetObjectFade_Code code(codeBuf, GetFnAddr(SetObjectFade));
			g_localTrampoline.EndAlloc(code.getCurr());

			
			if (!g_branchTrampoline.Write5Branch(g_setObjectFadeAddr, uintptr_t(code.getCode())))
				return false;
		}

		{
			struct InstallHookSetFreeLook_Code : Xbyak::CodeGenerator {
				InstallHookSetFreeLook_Code(void * buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label notfree;
					Xbyak::Label retnLabel1;
					Xbyak::Label retnLabel2;
					Xbyak::Label funcLabel;

					push(rax);
					push(rdx);
					sub(rsp, 0x20);

					mov(dl, dil);
					mov(rcx, rbx);
					call(ptr[rip + funcLabel]);

					mov(dil, al);

					add(rsp, 0x20);
					pop(rdx);
					pop(rax);

					cmp(byte[rbx + 0xDC], dil);

					je(notfree);

					jmp(ptr[rip + retnLabel1]);

					L(notfree);
					jmp(ptr[rip + retnLabel2]);

					L(retnLabel1);
					dq(g_freeLookAddr + 0x9);

					L(retnLabel2);
					dq(g_freeLookAddr + 0x74);

					L(funcLabel);
					dq(funcAddr);
				}
			};

			void * codeBuf = g_localTrampoline.StartAlloc();
			InstallHookSetFreeLook_Code code(codeBuf, GetFnAddr(SetFreeLook));
			g_localTrampoline.EndAlloc(code.getCurr());

			if (!g_branchTrampoline.Write5Branch(g_freeLookAddr, uintptr_t(code.getCode())))
				return false;

			SafeWrite16(g_freeLookAddr + 0x5, NOP16);
		}

		{
			struct InstallHookTargetLocation_Code : Xbyak::CodeGenerator {
				InstallHookTargetLocation_Code(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;

					sub(rsp, 0x20);

					lea(r8, ptr[rsp + 0x58]);
					mov(rdx, rsi);
					mov(rcx, ptr[rsi + 0xF0]);
					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_targetLocAddr + 0x5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookTargetLocation_Code code(codeBuf, GetFnAddr(SetTargetLocationHook));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write5Branch(g_targetLocAddr, uintptr_t(code.getCode())))
				return false;
		}

		
		{
			struct InstallHookClearTarget_Code : Xbyak::CodeGenerator {
				InstallHookClearTarget_Code(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel1;
					Xbyak::Label retnLabel2;
					Xbyak::Label funcLabel;
					Xbyak::Label disable;

					push(rax);
					push(rcx);
					sub(rsp, 0x20);

					mov(rcx, rsi);
					call(ptr[rip + funcLabel]);
					
					test(al, al);
					jne(disable);

					add(rsp, 0x20);
					pop(rcx);
					pop(rax);
					mov(rax, ptr[rcx + 0x1F0]);
					jmp(ptr[rip + retnLabel2]);

					L(disable);
					add(rsp, 0x20);
					pop(rcx);
					pop(rax);
					mov(rax, ptr[rcx + 0x1F0]);
					jmp(ptr[rip + retnLabel1]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel1);
					dq(g_clearTargetAddr + 0x798);

					L(retnLabel2);
					dq(g_clearTargetAddr + 0x19);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookClearTarget_Code code(codeBuf, GetFnAddr(ClearHeadTrackingTargetHook));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write6Branch(g_clearTargetAddr, uintptr_t(code.getCode())))
				return false;
		}

		{
			struct InstallHookBeginSaving_Code : Xbyak::CodeGenerator {
				InstallHookBeginSaving_Code(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
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

					mov(qword[rbp + 0x10], 0xFFFFFFFFFFFFFFFE);
					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_beginSaveAddr + 0x6);

				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookBeginSaving_Code code(codeBuf, GetFnAddr(BeginSavingHook));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write6Branch(g_beginSaveAddr, uintptr_t(code.getCode())))
				return false;

			SafeWrite16(g_beginSaveAddr + 0x6, NOP16);
		}

		{
			struct InstallHookFinishSavingCode : Xbyak::CodeGenerator {
				InstallHookFinishSavingCode(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
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

					lea(rcx, qword[rsp + 0x58]);
					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_finishSaveAddr + 0x5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookFinishSavingCode code(codeBuf, GetFnAddr(FinishSavingHook));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write5Branch(g_finishSaveAddr, uintptr_t(code.getCode())))
				return false;
		}

		{
			struct InstallHookCameraHeightCode : Xbyak::CodeGenerator {
				InstallHookCameraHeightCode(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;

					sub(rsp, 0x20);

					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_thirdCamHeightAddr + 0x5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookCameraHeightCode code(codeBuf, GetFnAddr(GetThirdPersonCameraHeight_Hook));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write5Branch(g_thirdCamHeightAddr, uintptr_t(code.getCode())))
				return false;
		}

		{
			struct InstallHookThirdCamColCode : Xbyak::CodeGenerator {
				InstallHookThirdCamColCode(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;

					sub(rsp, 0x20);

					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_thirdCamColAddr + 0x5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookThirdCamColCode code(codeBuf, GetFnAddr(TPCamProcessCollision_Hook));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write5Branch(g_thirdCamColAddr, uintptr_t(code.getCode())))
				return false;
		}

		Menus::InstallHook();
		Graphics::InstallHook();

		SafeWrite8(g_firstFOVAddr + 0x3, 0xB8);	// change xmm6 to xmm7

		return true;
	}
}