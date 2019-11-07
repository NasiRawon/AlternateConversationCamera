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
#include "Havok.h"

#include <shlobj.h>

IDebugLog				gLog;
PluginHandle			g_pluginHandle = kPluginHandle_Invalid;

void * g_moduleHandle = nullptr;

uintptr_t g_defaultWorldFOVAddr = 0;
uintptr_t g_defaultFirstFOVAddr = 0;
uintptr_t g_onCameraMoveAddr = 0;
uintptr_t g_disablePlayerLookAddr = 0;
uintptr_t g_dialogueMenuAddr = 0;
uintptr_t g_raceMenuAddr = 0;
uintptr_t g_processMovementAddr = 0;
uintptr_t g_minCurrentZoomAddr = 0;
uintptr_t g_onPlayerDeathAddr = 0;
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
uintptr_t g_thirdCamUpdateAddr = 0;
uintptr_t g_camCollisionAddr = 0;

void MainGetAddresses()
{
	const std::array<BYTE, 10> worldFOVpattern = { 0x0F, 0x28, 0xC1, 0xF3, 0x0F, 0x5E, 0xC6, 0x0F, 0x28, 0xF0 };
	g_defaultWorldFOVAddr = (uintptr_t)scan_memory_data(worldFOVpattern, 0x5F, true, 0x4, 0x8);

	g_defaultFirstFOVAddr = (uintptr_t)scan_memory_data(worldFOVpattern, 0x6F, true, 0x4, 0x8);

	const std::array<BYTE, 6> minZoompattern = { 0x8B, 0x47, 0x58, 0x89, 0x47, 0x64 };
	g_minCurrentZoomAddr = (uintptr_t)scan_memory_data(minZoompattern, 0x6, true, 0x4, 0x8);

	const std::array<BYTE, 11> cameraMovepattern = { 0x4C, 0x8B, 0xC3, 0x48, 0x8B, 0xD7, 0xB9, 0x14, 0x00, 0x00, 0x00 };
	g_onCameraMoveAddr = (uintptr_t)scan_memory(cameraMovepattern, 0x6F, false);

	const std::array<BYTE, 7> lookpattern = { 0x48, 0x8B, 0x01, 0x4C, 0x8D, 0x43, 0x1C };
	g_disablePlayerLookAddr = (uintptr_t)scan_memory(lookpattern, 0x91, false);

	g_dialogueMenuAddr = (uintptr_t)scan_memory(lookpattern, 0xB4, false);

	g_raceMenuAddr = (uintptr_t)scan_memory(lookpattern, 0xBE, false);

	const std::array<BYTE, 6> processMovepattern = { 0x0F, 0x28, 0xF7, 0x48, 0x8B, 0xCB };
	g_processMovementAddr = (uintptr_t)scan_memory(processMovepattern, 0x4B, false);

	const std::array<BYTE, 11> playerDeathpattern = { 0x4C, 0x8B, 0xC3, 0x48, 0x8B, 0xD7, 0xB9, 0x14, 0x00, 0x00, 0x00 };
	g_onPlayerDeathAddr = (uintptr_t)scan_memory(playerDeathpattern, 0xD0, true);

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

	const std::array<BYTE, 9> onUpdatePattern = { 0x48, 0x8D, 0x54, 0x24, 0x38, 0x48, 0x8B, 0x4B, 0x10 };
	g_thirdCamUpdateAddr = (uintptr_t)scan_memory(onUpdatePattern, 0x99, false);

	const std::array<BYTE, 6> camColPattern = { 0x0F, 0x28, 0xD8, 0x0F, 0x2F, 0xD8 };
	g_camCollisionAddr = (uintptr_t)scan_memory(camColPattern, 0xBF, false);
}


namespace Tralala
{
	const float PI = 3.1415927f;

	float g_firstDistance = 0.0f;
	float g_thirdDistance = 0.0f;
	float g_worldfovTo = 0.0f;
	float g_firstfovTo = 0.0f;
	float g_delay = 0.0f;
	float g_diffRotX = 0.0f;
	int g_fovStep = 0;
	bool g_isNPC = false;
	bool g_processSwitch = false;
	bool g_endSwitch = false;

	Actor* g_refTarget = nullptr;

	static void SetZoom(float targetFOV)
	{
		float mult = roundf(1.0f / *(float*)g_deltaTimeAddr) / 60.0f;

		int step = Settings::fCameraSpeed * 3.6f * (*(float*)g_deltaTimeAddr) * mult * mult;
		if (step <= 0) step = 1;
		g_worldfovTo = targetFOV;
		g_firstfovTo = targetFOV;
		g_fovStep = step;
	}

	static void ResetZoom()
	{
		float mult = roundf(1.0f / *(float*)g_deltaTimeAddr) / 60.0f;

		int step = Settings::fCameraSpeed * 3.6f * (*(float*)g_deltaTimeAddr) * mult * mult;
		if (step <= 0) step = 1;
		g_worldfovTo = *(float*)g_defaultWorldFOVAddr;
		g_firstfovTo = *(float*)g_defaultFirstFOVAddr;
		g_fovStep = step;
	}

	static bool IsValidFaceToFace(PlayerCamera * camera, Actor** ret)
	{
		if (!camera->IsCameraFirstPerson() && !camera->IsCameraThirdPerson())
			return false;

		if (MenuControls::GetSingleton()->beastForm)
			return false;

		if (PlayerCharacter::GetSingleton()->IsOnMount())
			return false;

		if (PlayerCharacter::GetSingleton()->IsOnCarriage())
			return false;

		TESObjectREFR* target = nullptr;
		target = MenuTopicManager::GetSingleton()->GetDialogueTarget();
		if (!target || !target->IsCharacter())
			return false;

		*ret = (Actor*)target;

		return true;
	}


	void DialogueMenuEventHandler(MenuOpenCloseEvent * evn)
	{
		PlayerCharacter * player = PlayerCharacter::GetSingleton();
		PlayerCamera * camera = PlayerCamera::GetSingleton();

		static UInt32 cameraStateIDStarter = 0;
		float minZoom = *(float*)g_minCurrentZoomAddr;
		//g_switchProcess = false;

		ThirdPersonState* tps = camera->GetThirdPersonCamera();

		if (evn->opening)
		{
			Actor* actor = nullptr;
		
			if (!IsValidFaceToFace(camera, &actor))
			{
				g_refTarget = nullptr;
				return;
			}

			if (Settings::bHeadTracking)
				player->processManager->SetDialogueHeadTrackingTarget(actor);

			float thisFOV = 0.0f;
			float distance = 0.0f;
			float prefDist = Settings::f1stZoom;
			g_delay = 60.0f; // init delay, so it won't instantly switch to the npc
			g_diffRotX = 0.0f;
			g_endSwitch = false;
			g_processSwitch = false;

			if (camera->IsCameraThirdPerson())
			{
				cameraStateIDStarter = PlayerCamera::kCameraState_ThirdPerson2;

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
			else
			{
				cameraStateIDStarter = PlayerCamera::kCameraState_FirstPerson;
				if (Settings::bForceThirdPerson)
				{
					camera->ForceThirdPerson();

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
				thisFOV = roundf((atanf(prefDist / distance) * 2.0f) * 180.0f / PI);
				if (thisFOV < 20.0f)
					thisFOV = 20.0f;

				SetZoom(thisFOV);

				g_firstDistance = distance;
				g_refTarget = actor;
			}
			else
			{
				if (cameraStateIDStarter == PlayerCamera::kCameraState_ThirdPerson2)
				{
					prefDist = Settings::f3rdZoom;

					if (actor->IsFlyingActor())
						prefDist = Settings::fDragonZoom;

					distance = camera->GetDistanceWithTargetBone(actor, false);
					thisFOV = roundf((atanf(prefDist / distance) * 2.0f) * 180.0f / PI);
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
		else
		{		

			if (!g_refTarget)
			{
				cameraStateIDStarter = 0;
				return;
			}

			g_refTarget = nullptr;

			if (Settings::bHeadTracking)
				player->processManager->ClearHeadTracking();

			if (Settings::bSwitchTarget)
			{
				g_delay = 0.0f;
				camera->SetCameraTarget(player);
			}

			if (camera->IsCameraFirstPerson() || camera->IsCameraThirdPerson())
			{
				if (cameraStateIDStarter == PlayerCamera::kCameraState_ThirdPerson2)
				{
					tps->UpdateMode(!camera->isWeapSheathed);

					if (player->IsNotInFurniture() && camera->IsCameraThirdPerson())
					{
						if (!Settings::bSwitchTarget)
						{
							player->rot.z = camera->camRotZ;
							player->rot.x -= tps->diffRotX;
						}
						else
						{
							player->rot.x -= g_diffRotX;
						}
					}

					tps->diffRotX = tps->diffRotZ = 0.0f;

					if (camera->IsCameraFirstPerson())
						camera->ForceThirdPerson();

				}
				else
				{
					if (Settings::bForceThirdPerson && camera->IsCameraThirdPerson())
					{
						tps->diffRotX = tps->diffRotZ = 0.0f;
						tps->UpdateMode(!camera->isWeapSheathed);
						tps->SetFirstPersonSmooth(minZoom, true);
					}
				}
			}

			ResetZoom();
		}
	}

	void MenuEventHandler(MenuOpenCloseEvent * evn) 
	{
		if (evn->opening)
		{
			BSFixedString racesexMenu("RaceSex Menu");
		
			if (evn->menuName == racesexMenu) // fix for face sculptor
			{
				PlayerCamera* camera = PlayerCamera::GetSingleton();
				if (camera)
				{
					camera->worldFOV = *(float*)g_defaultWorldFOVAddr;
					camera->firstPersonFOV = *(float*)g_defaultFirstFOVAddr;
				}
			}
		}
	}

	float RotateCamera(PlayerCamera * camera, Actor* source, Actor* target, TESCameraController* controller, bool calcCrosshairToBoneMag)
	{
		NiPoint3 pos;
		camera->GetDistanceWithTargetBone(target, &pos);
		controller->unk1C = 1;

		float xy, rotZ, rotX = 0.0f;

		xy = sqrtf(pos.x*pos.x + pos.y*pos.y);
		rotZ = atan2f(pos.x, pos.y);
		rotX = atan2f(-pos.z, xy);

		float diffAngleZ, diffAngleX = 0.0f;

		FirstPersonState* fps = camera->GetFirstPersonCamera();
		ThirdPersonState* tps = camera->GetThirdPersonCamera();

		if (source->IsNotInFurniture())
		{
			if (camera->IsCameraThirdPerson())
			{
				float angleDiffZ, angleDiffX;
				{
					source->rot.x -= tps->diffRotX;
					tps->diffRotX = tps->diffRotZ = 0.0f;

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

				float mult = roundf(1.0f / *(float*)g_deltaTimeAddr) / 60.0f;

				angleDiffZ = angleDiffZ / (Settings::fCameraSpeed * 1.8f * (*(float*)g_deltaTimeAddr) * mult * mult);
				angleDiffX = angleDiffX / (Settings::fCameraSpeed * 1.8f * (*(float*)g_deltaTimeAddr) * mult * mult);

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

				float mult = roundf(1.0f / *(float*)g_deltaTimeAddr) / 60.0f;

				angleDiffZ = angleDiffZ / (Settings::fCameraSpeed * 1.8f * (*(float*)g_deltaTimeAddr) * mult * mult);
				angleDiffX = angleDiffX / (Settings::fCameraSpeed * 1.8f * (*(float*)g_deltaTimeAddr) * mult * mult);

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

				float mult = roundf(1.0f / *(float*)g_deltaTimeAddr) / 60.0f;

				angleDiffZ = angleDiffZ / (Settings::fCameraSpeed * 1.8f * (*(float*)g_deltaTimeAddr) * mult * mult);
				angleDiffX = angleDiffX / (Settings::fCameraSpeed * 1.8f * (*(float*)g_deltaTimeAddr) * mult * mult);

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
						if (sqrtf(angleDiffX*angleDiffX + angleDiffZ*angleDiffZ) <= 0.0005f)
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
					angleDiffX = atan2f(pos.z, xy) - (tps->diffRotX - source->rot.x);

					while (angleDiffZ < -PI)
						angleDiffZ += 2.0f * PI;
					while (angleDiffZ > PI)
						angleDiffZ -= 2.0f * PI;
					while (angleDiffX < -PI)
						angleDiffX += 2.0f * PI;
					while (angleDiffX > PI)
						angleDiffX -= 2.0f * PI;
				}

				float mult = roundf(1.0f / *(float*)g_deltaTimeAddr) / 60.0f;

				angleDiffZ = angleDiffZ / (Settings::fCameraSpeed * 1.8f * (*(float*)g_deltaTimeAddr) * mult * mult);
				angleDiffX = angleDiffX / (Settings::fCameraSpeed * 1.8f * (*(float*)g_deltaTimeAddr) * mult * mult);

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
			crosshairDist = sqrtf(diffAngleZ*diffAngleZ + diffAngleX*diffAngleX);

		return crosshairDist;
	}

	TESObjectWEAP * OnCameraMove(PlayerCharacter * player, bool isLeftHand)
	{
		if (Settings::bHeadTracking && Settings::bConversationHT && player->IsOnMount())
			player->SetIsNPCAnimVar();

		PlayerCamera* camera = PlayerCamera::GetSingleton();
		TESCameraController * controller = TESCameraController::GetSingleton();
		MenuTopicManager* mtm = MenuTopicManager::GetSingleton();

		if (camera->IsCameraFirstPerson() || camera->IsCameraThirdPerson())
		{
			if (Settings::bHeadTracking) 
			{
				BSFixedString isNPCVar("IsNPC");

				if (Settings::bConversationHT && g_refTarget && mtm && mtm->isInDialogueState)
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

				if (g_refTarget && camera->cameraRefHandle == PlayerRefHandle && !g_processSwitch)
					RotateCamera(camera, player, g_refTarget, controller, false);

				g_fovStep--;
			}
			else
			{
				if(!g_refTarget)
					return player->GetEquippedWeapon(isLeftHand);

				if (Settings::bLockOn || mtm->talkingHandle == 0)
				{
					static bool startSwitch = false;
					float prefDist = Settings::f1stZoom;

					ThirdPersonState* tps = camera->GetThirdPersonCamera();

					if (Settings::bForceFirstPerson || camera->IsCameraFirstPerson())
					{
						if (g_refTarget->IsFlyingActor())
							prefDist = Settings::fDragonZoom;

						float distance = camera->GetDistanceWithTargetBone(g_refTarget, true);

						if (abs(g_firstDistance - distance) >= 5.0f)
						{
							float newFOV = roundf((atanf(prefDist / distance) * 2.0f) * 180.0f / PI);
							if (newFOV < 20.0f)
								newFOV = 20.0f;

							SetZoom(newFOV);
							g_firstDistance = distance;
						}

					}
					else
					{
						if (Settings::bSwitchTarget)
						{
							if (tps->curPosY == tps->dstPosY)
							{
								if (g_thirdDistance == 0.0f)
								{
									prefDist = Settings::f3rdZoom;

									if (g_refTarget->IsFlyingActor())
										prefDist = Settings::fDragonZoom;

									float distance = camera->GetDistanceWithTargetBone(g_refTarget, false);

									float thisFOV = roundf((atanf(prefDist / distance) * 2.0f) * 180.0f / PI);
									if (thisFOV < 30.0f)
										thisFOV = 30.0f;

									SetZoom(thisFOV);
									g_thirdDistance = distance;
								}
								else
								{
									if ((g_refTarget->IsTalking() || mtm->unk70) && !mtm->unkB9)
									{
										camera->SetCameraTarget(player);

										if (g_delay >= 60.0f) // init
										{
											startSwitch = false;
										}
										else // normal				
										{
											g_delay = 10.0f;
											startSwitch = true;
										}
									}
									else
									{
										float delay = *(float*)g_deltaTimeAddr * 60.f;

										if (g_delay > 0.0f)
											g_delay -= delay;
										if (g_delay < 0.0f)
											g_delay = 0.0f;

										if (startSwitch && camera->cameraRefHandle == PlayerRefHandle
											&& g_delay <= 0.0f && !g_endSwitch)
										{										
											camera->SetCameraTarget(g_refTarget);
											startSwitch = false;
											g_processSwitch = true;							
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

							if (abs(g_thirdDistance - distance) >= 10.0f)
							{
								float newFOV = roundf((atanf(prefDist / distance) * 2.0f) * 180.0f / PI);
								if (newFOV < 30.0f)
									newFOV = 30.0f;

								SetZoom(newFOV);
								g_thirdDistance = distance;
							}
						}
					}

					if (camera->cameraRefHandle == PlayerRefHandle)
					{
						if (Settings::bSwitchTarget && camera->IsCameraThirdPerson())
						{
							if (!g_processSwitch) // init switch process
							{
								g_diffRotX = tps->diffRotX;
								float targetDist = RotateCamera(camera, player, g_refTarget, controller, true);
								if (targetDist <= 0.001f)
									startSwitch = true;
							}
							else
							{
								startSwitch = true;
								if (mtm->unkB2)
									g_endSwitch = true;
							}
						}
						else
						{
							RotateCamera(camera, player, g_refTarget, controller, false);
						}
					}
				}
				else
				{
					float crosshairDist = RotateCamera(camera, player, g_refTarget, controller, true);
					if (crosshairDist <= 0.001f)
						g_refTarget = nullptr;
				}
			}
		}

		return player->GetEquippedWeapon(isLeftHand);
	}

	void OnPlayerDeath()
	{
		g_refTarget = nullptr;
		PlayerCamera * camera = PlayerCamera::GetSingleton();
		if (camera)
		{
			camera->worldFOV = *(float*)g_defaultWorldFOVAddr;
			camera->firstPersonFOV = *(float*)g_defaultFirstFOVAddr;

			PlayerCharacter * player = PlayerCharacter::GetSingleton();
			if (Settings::bSwitchTarget)
				camera->SetCameraTarget(player);
		}
	}

	// FECKIN' PHYSICS
	/*
	float GetThirdPersonCameraHeight_Hook(Tralala::Actor * actor)
	{
		Tralala::PlayerCharacter* player = Tralala::PlayerCharacter::GetSingleton();

		static float playerHeight = 0.0f;
		static float npcHeight = 0.0f;

		if (!Settings::bSwitchTarget || !Tralala::MenuTopicManager::GetSingleton()->isInDialogueState
			|| !g_refTarget)
		{
			playerHeight = 0.0f;
			npcHeight = 0.0f;

			return actor->GetCameraHeight();
		}
		
		NiPoint3 headPos;
		bool ret = false;

		if (!actor->GetTargetHeadNodePosition(&headPos, &ret))
			actor->GetMarkerPosition(&headPos);

		float headPosZ = headPos.z - actor->pos.z;

		if (actor->actorState.IsWeaponDrawn() || actor->IsSneaking()
			|| !actor->IsNotInFurniture())
			headPosZ += 5.0f;

		if (actor == player)
		{
			npcHeight = 0.0f;

			if (playerHeight == 0.0f)
			{
				playerHeight = headPosZ;
				return headPosZ;
			}

			if (!actor->IsNotInFurniture())
				return playerHeight;

			if ((headPosZ - playerHeight) >= 3.0f)
				playerHeight += 0.5f;
			else if ((headPosZ - playerHeight) <= -3.0f)
				playerHeight -= 0.5f;

			return playerHeight;
		}
		else
		{
			playerHeight = 0.0f;

			if (npcHeight == 0.0f)
			{
				npcHeight = headPosZ;
				return headPosZ;
			}

			if (!actor->IsNotInFurniture())
				return npcHeight;

			if ((headPosZ - npcHeight) >= 3.0f)
				npcHeight += 0.5f;
			else if ((headPosZ - npcHeight) <= -3.0f)
				npcHeight -= 0.5f;

			return npcHeight;
		}
	}
	*/

	float GetThirdPersonCameraHeight_Hook(Actor* actor)
	{
		static float npcCamPosZ = 0.0f;

		if (!Settings::bSwitchTarget || !MenuTopicManager::GetSingleton()->isInDialogueState
			|| !g_refTarget)
		{
			npcCamPosZ = 0.0f;
			return actor->GetCameraHeight();
		}

		NiPoint3 headPos;
		bool ret = true;

		if (!actor->GetTargetHeadNodePosition(&headPos, &ret))
		{
			ret = false;
			actor->GetMarkerPosition(&headPos);
		}

		float camPosZ = headPos.z - actor->pos.z;

		PlayerCharacter* player = PlayerCharacter::GetSingleton();
		if (actor == player)
		{
			if (!actor->IsNotInFurniture())
				return camPosZ + 6.0f;

			return actor->GetCameraHeight();
		}
		else
		{
			if (!ret)
				return camPosZ;

			if (actor->IsInAir() || actor->IsSneaking())
				return camPosZ;

			if (actor->actorState.IsWeaponDrawn() ||
				!actor->IsNotInFurniture())
			{
				if (npcCamPosZ <= 0.0f)
					npcCamPosZ = camPosZ + 6.0f;

				return npcCamPosZ;
			}

			return camPosZ;
		}
	}


	// TO DO - FIX THE FECKIN' CAMERA COLLISION
	void TPCamProcessCollision_Hook(ThirdPersonState* tps)
	{
		if (!Settings::bSwitchTarget)
			return tps->ProcessCameraCollision();

		PlayerCharacter* player = PlayerCharacter::GetSingleton();
		PlayerCamera* camera = (PlayerCamera*)tps->camera;

		static bool isPCCollided = false;
		static bool isNPCCollided = false;
		static bool isFade = false;
		static bool pcRotate = true;
		static bool npcRotate = true;
		static float curDistance = 0.0f;
		static float pcHeight = 0.0f;
		static float npcHeight = 0.0f;

		if (MenuTopicManager::GetSingleton()->isInDialogueState && g_refTarget && player->parentCell)
		{
			if (camera->cameraRefHandle != PlayerRefHandle)
			{
				if (g_refTarget->movementCtrlNPC)
				{
					MovementControllerNPC* movementCtrl = g_refTarget->movementCtrlNPC;

					BSFixedString intfc("IMovementSetGoal");

					IMovementInterface* movementIntfc = movementCtrl->QueryMovementInterface(intfc);
					if (movementIntfc)
					{
						IMovementSetGoal* setGoal = (IMovementSetGoal*)movementIntfc;
						UInt32 unkVar = *(UInt32*)(setGoal + 0x6);

						if(unkVar != 3)
							setGoal->ClearPathingRequest();
					}
				}

				pcRotate = true;

				NiPoint3 targetHeadPos;
				bool targetHeadPosRet = false;
				if (!player->GetTargetHeadNodePosition(&targetHeadPos, &targetHeadPosRet))
					player->GetMarkerPosition(&targetHeadPos);

				NiPoint3 headPos;
				bool headPosRet = true;
				if (!g_refTarget->GetTargetHeadNodePosition(&headPos, &headPosRet))
				{
					headPosRet = false;
					g_refTarget->GetMarkerPosition(&headPos);
				}
				
				NiPoint3 resultPos;
				resultPos = tps->camPos;

				targetHeadPos.x = player->pos.x;
				targetHeadPos.y = player->pos.y;

				Havok::hkpRootCdPoint resultInfo;
				Actor* resultActor = nullptr;

				if (camera->GetClosestPoint(player->GetbhkWorldM(), &targetHeadPos, &resultPos, &resultInfo,
					&resultActor, 1.0f))
				{
	#if 0
					NiAVObject * niObject = resultInfo.hkpCollidableB->GetNiObject();
					if (niObject)
					{
						_MESSAGE("__PLAYER__ name %s",
							niObject->m_name);
					}

	#endif
				
					isPCCollided = true;
				}

				if (isPCCollided)
				{
					NiPoint3 offset(Settings::fHumanCamOffsetX, 
						Settings::fHumanCamOffsetY, 
						Settings::fHumanCamOffsetZ);
					if (!headPosRet)
					{
						offset.x = Settings::fCreatureCamOffsetX;
						offset.y = Settings::fCreatureCamOffsetY;
						offset.z = Settings::fCreatureCamOffsetZ;
					}

					// make sure the camera's owner has 3d loaded
					if (g_refTarget->loadedState && g_refTarget->loadedState->node)
					{
						NiPoint3 ret;
						ret = g_refTarget->loadedState->node->m_worldTransform.rot * offset;
						tps->camPos = headPos + ret;
					}
					else
					{
						// either this or resultPos
						// however, i prefer fake first person instead of collision pos
						tps->camPos = headPos;
					}
				}

				NiPoint3 targetPos;
				player->GetTargetNeckPosition(&targetPos);

				if (!player->IsSneaking() && player->actorState.IsWeaponDrawn())
				{
					if (pcHeight <= 0.0f)
						pcHeight = targetPos.z - player->pos.z;

					targetPos.x = player->pos.x;
					targetPos.y = player->pos.y;
					targetPos.z = player->pos.z + pcHeight;
				}

				NiPoint3 deltaPos = targetPos - tps->camPos;

				float xy = sqrtf(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y);
				float rotZ = atan2f(deltaPos.x, deltaPos.y);
				float rotX = atan2f(deltaPos.z, xy);

				while (rotZ < 0.0f)
					rotZ += 2.0f * PI;
				while (rotZ > 2.0f * PI)
					rotZ -= 2.0f * PI;
				while (rotX < -PI)
					rotX += 2.0f * PI;
				while (rotX > PI)
					rotX -= 2.0f * PI;

				float camRotX = tps->diffRotX - g_refTarget->rot.x;
				float diffAngleX = 0.0f;
				float diffAngleZ = 0.0f;
				float origDiffAngleX = rotX - camRotX;
				float origDiffAngleZ = rotZ - camera->camRotZ;

				if (npcRotate)
				{
					diffAngleX = rotX - camRotX;
					diffAngleZ = rotZ - camera->camRotZ;

					while (diffAngleZ < -PI)
						diffAngleZ += 2.0f * PI;
					while (diffAngleZ > PI)
						diffAngleZ -= 2.0f * PI;
					while (diffAngleX < -PI)
						diffAngleX += 2.0f * PI;
					while (diffAngleX > PI)
						diffAngleX -= 2.0f * PI;
				}

				float distance = camera->GetDistanceWithTargetBone(player, false);
				if (abs(curDistance - distance) >= 10.0f)
				{
					float newFOV = roundf((atanf(Settings::f3rdZoom / distance) * 2.0f) * 180.0f / PI);
					if (newFOV < 30.0f)
						newFOV = 30.0f;

					SetZoom(newFOV);
					curDistance = distance;
				}

				if (fabs(diffAngleX) < 0.001f && fabs(diffAngleZ) < 0.001f)
				{
					npcRotate = false;

					while (origDiffAngleZ < -PI)
						origDiffAngleZ += 2.0f * PI;
					while (origDiffAngleZ > PI)
						origDiffAngleZ -= 2.0f * PI;
					while (origDiffAngleX < -PI)
						origDiffAngleX += 2.0f * PI;
					while (origDiffAngleX > PI)
						origDiffAngleX -= 2.0f * PI;

					float mult = (*(float*)g_deltaTimeAddr * 60.f);

					if (camera->IsInCameraView(&targetPos, PlayerCamera::KRotate_Stop))
					{
						constexpr float speed = 1.0f / 90.f;

						diffAngleX = origDiffAngleX * speed * mult;
						diffAngleZ = origDiffAngleZ * speed * mult;
					}
					else if (camera->IsInCameraView(&targetPos, PlayerCamera::kRotate_Slow))
					{
						constexpr float speed = 1.0f / 60.f;

						diffAngleX = origDiffAngleX * speed * mult;
						diffAngleZ = origDiffAngleZ * speed * mult;
					}
					else
					{
						constexpr float speed = 1.0f / 30.f;

						diffAngleX = origDiffAngleX * speed * mult;
						diffAngleZ = origDiffAngleZ * speed * mult;
					}
				}

				tps->diffRotZ += diffAngleZ;
				tps->diffRotX += diffAngleX;

				isNPCCollided = false;

				return;
			}
			else
			{

				NiPoint3 targetHeadPos;
				bool targetHeadPosRet = true;
				if (!g_refTarget->GetTargetHeadNodePosition(&targetHeadPos, &targetHeadPosRet))
				{
					targetHeadPosRet = false;
					g_refTarget->GetMarkerPosition(&targetHeadPos);
				}

				NiPoint3 headPos;
				bool headPosRet = false;
				if (player->GetTargetHeadNodePosition(&headPos, &headPosRet))
					player->GetMarkerPosition(&headPos);

				NiPoint3 resultPos;
				resultPos = tps->camPos;

				targetHeadPos.x = g_refTarget->pos.x;
				targetHeadPos.y = g_refTarget->pos.y;

				Havok::hkpRootCdPoint resultInfo;
				Actor* resultActor = nullptr;

				if (camera->GetClosestPoint(g_refTarget->GetbhkWorldM(), &targetHeadPos, &resultPos, &resultInfo,
					&resultActor, 1.0f))
				{
	#if 0
					NiAVObject * niObject = resultInfo.hkpCollidableB->GetNiObject();
					if (niObject)
					{
						_MESSAGE("__NPC__ name %s",
							niObject->m_name);
					}
	#endif
					isNPCCollided = true;

				}

				if (!g_processSwitch)
				{
					isPCCollided = false;
					isNPCCollided = false;
					pcRotate = true;
					npcRotate = true;
					curDistance = 0.0f;
				
					if (camera->objectFadeHandle == PlayerRefHandle)
					{
						isFade = true;
						return tps->ProcessCameraCollision();
					}

					if (!isFade)
						return tps->ProcessCameraCollision();

					NiPoint3 offset(Settings::fHumanCamOffsetX,
						Settings::fHumanCamOffsetY,
						Settings::fHumanCamOffsetZ);

					// make sure the camera's owner has 3d loaded
					if (player->loadedState && player->loadedState->node)
					{
						NiPoint3 ret;
						ret = player->loadedState->node->m_worldTransform.rot * offset;
						tps->camPos = headPos + ret;
					}
					else
					{
						// either this or resultPos
						// however, i prefer fake first person instead of collision pos
						tps->camPos = headPos;
					}

					return;
				}

				npcRotate = true;
				
				if (isNPCCollided)
				{
					NiPoint3 offset(Settings::fHumanCamOffsetX,
						Settings::fHumanCamOffsetY,
						Settings::fHumanCamOffsetZ);

					// make sure the camera's owner has 3d loaded
					if (player->loadedState && player->loadedState->node)
					{
						NiPoint3 ret;
						ret = player->loadedState->node->m_worldTransform.rot * offset;
						tps->camPos = headPos + ret;
					}
					else
					{
						// either this or resultPos
						// however, i prefer fake first person instead of collision pos
						tps->camPos = headPos;
					}
				}

				NiPoint3 targetPos;
				g_refTarget->GetTargetNeckPosition(&targetPos);

				if (!g_refTarget->IsSneaking() && g_refTarget->actorState.IsWeaponDrawn() && targetHeadPosRet)
				{
					if (npcHeight <= 0.0f)
						npcHeight = targetPos.z - g_refTarget->pos.z;

					targetPos.x = g_refTarget->pos.x;
					targetPos.y = g_refTarget->pos.y;			
					targetPos.z = g_refTarget->pos.z + npcHeight;
				}

				NiPoint3 deltaPos = targetPos - tps->camPos;

				float xy = sqrtf(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y);
				float rotZ = atan2f(deltaPos.x, deltaPos.y);
				float rotX = atan2f(deltaPos.z, xy);

				while (rotZ < 0.0f)
					rotZ += 2.0f * PI;
				while (rotZ > 2.0f * PI)
					rotZ -= 2.0f * PI;
				while (rotX < -PI)
					rotX += 2.0f * PI;
				while (rotX > PI)
					rotX -= 2.0f * PI;

				float camRotX = tps->diffRotX - player->rot.x;
				float diffAngleX = 0.0f;
				float diffAngleZ = 0.0f;
				float origDiffAngleX = rotX - camRotX;
				float origDiffAngleZ = rotZ - camera->camRotZ;

				if (pcRotate)
				{
					diffAngleX = rotX - camRotX;
					diffAngleZ = rotZ - camera->camRotZ;

					while (diffAngleZ < -PI)
						diffAngleZ += 2.0f * PI;
					while (diffAngleZ > PI)
						diffAngleZ -= 2.0f * PI;
					while (diffAngleX < -PI)
						diffAngleX += 2.0f * PI;
					while (diffAngleX > PI)
						diffAngleX -= 2.0f * PI;
				}

				float distance = camera->GetDistanceWithTargetBone(g_refTarget, false);
				if (abs(curDistance - distance) >= 10.0f)
				{
					float prefDist = Settings::f3rdZoom;

					if (g_refTarget->IsFlyingActor())
						prefDist = Settings::fDragonZoom;

					float newFOV = roundf((atanf(prefDist / distance) * 2.0f) * 180.0f / PI);
					if (newFOV < 30.0f)
						newFOV = 30.0f;

					SetZoom(newFOV);
					curDistance = distance;
				}

				if (fabs(diffAngleX) < 0.001f && fabs(diffAngleZ) < 0.001f)
				{
					pcRotate = false;

					while (origDiffAngleZ < -PI)
						origDiffAngleZ += 2.0f * PI;
					while (origDiffAngleZ > PI)
						origDiffAngleZ -= 2.0f * PI;
					while (origDiffAngleX < -PI)
						origDiffAngleX += 2.0f * PI;
					while (origDiffAngleX > PI)
						origDiffAngleX -= 2.0f * PI;

					float mult = (*(float*)g_deltaTimeAddr * 60.f);

					if (camera->IsInCameraView(&targetPos, PlayerCamera::KRotate_Stop))
					{
						constexpr float speed = 1.0f / 90.f;

						diffAngleX = origDiffAngleX * speed * mult;
						diffAngleZ = origDiffAngleZ * speed * mult;
					}
					else if (camera->IsInCameraView(&targetPos, PlayerCamera::kRotate_Slow))
					{
						constexpr float speed = 1.0f / 60.f;

						diffAngleX = origDiffAngleX * speed * mult;
						diffAngleZ = origDiffAngleZ * speed * mult;
					}
					else
					{
						constexpr float speed = 1.0f / 30.f;

						diffAngleX = origDiffAngleX * speed * mult;
						diffAngleZ = origDiffAngleZ * speed * mult;
					}
				}

				tps->diffRotZ += diffAngleZ;
				tps->diffRotX += diffAngleX;

				g_diffRotX = tps->diffRotX;
				isPCCollided = false;

				return;
			}
		}

		isFade = false;
		isPCCollided = false;
		isNPCCollided = false;
		pcRotate = true;
		npcRotate = true;
		curDistance = 0.0f;
		pcHeight = 0.0f;
		npcHeight = 0.0f;

		return tps->ProcessCameraCollision();
	}

	bool CameraProcessCollision_Hook(PlayerCamera* camera, NiPoint3* camPos, bool isFade)
	{
		PlayerCharacter* player = PlayerCharacter::GetSingleton();

		if (Settings::bSwitchTarget && MenuTopicManager::GetSingleton()->isInDialogueState
			&& g_refTarget && camera->cameraRefHandle == PlayerRefHandle && player->processManager)
		{
			if (camera->objectFadeHandle == PlayerRefHandle)
			{
				camera->objectFadeHandle = InvalidRefHandle();
				if (player->processManager->unk10)
				{
					UInt32 unkVar = player->processManager->unk10->unk130;
					if (unkVar == 2)
					{
						player->Unk_0x5ECF90();
					}
				}
			}
		
			isFade = false;
		}
	
		return camera->ProcessCollision(camPos, isFade);
	}

	bool SetFreeLook(ThirdPersonState * tps, bool freeLook)
	{
		PlayerCamera * camera = (PlayerCamera*)tps->camera;

		if (Settings::bSwitchTarget && MenuTopicManager::GetSingleton()->isInDialogueState
			&& g_refTarget /*&& (!camera->isWeapSheathed || camera->cameraRefHandle != PlayerRefHandle)*/)
		{
			freeLook = true;
		}

		return freeLook;
	}

	void SetTargetLocationHook(ActorProcessManager* apm, TESObjectREFR* source, NiPoint3* location)
	{

		if (!Settings::bHeadTracking)
			return apm->SetTargetLocation(source, location);

		if (source != PlayerCharacter::GetSingleton())
			return apm->SetTargetLocation(source, location);

		if (!g_refTarget)
			return apm->SetTargetLocation(source, location);

		NiPoint3 pos;
		g_refTarget->GetMarkerPosition(&pos);

		return apm->SetTargetLocation(source, &pos);
	}

	bool ClearHeadTrackingTargetHook(Actor* source)
	{

		SInt32 updateActorPerFrame = *(SInt32*)g_actorPerFrameAddr;
		if (*(SInt32*)g_updateActorAddr > updateActorPerFrame)
		{
			return true;
		}
		else
		{
			PlayerCharacter* player = PlayerCharacter::GetSingleton();
			MenuTopicManager* mtm = MenuTopicManager::GetSingleton();

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
		PlayerCharacter* player = PlayerCharacter::GetSingleton();

		if (player)
		{
			g_isNPC = player->GetIsNPCAnimVar();

			if (player->SetIsNPCAnimVar())
				_MESSAGE("Reset IsNPC animation variable.");
		}
	}

	void FinishSavingHook()
	{
		PlayerCharacter* player = PlayerCharacter::GetSingleton();

		if (player && g_isNPC)
			player->SetIsNPCAnimVar(true);
	}
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
		Havok::GetAddresses();

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
			InstallHookDialogueMenu_Code code(codeBuf, GetFnAddr(Tralala::DialogueMenuEventHandler));
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
			InstallHookMenuEventHandler_Code code(codeBuf, GetFnAddr(Tralala::MenuEventHandler));
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
			InstallHookOnCameraMove_Code code(codeBuf, GetFnAddr(Tralala::OnCameraMove));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write5Branch(g_onCameraMoveAddr, uintptr_t(code.getCode())))
				return false;
		}

		{
			struct InstallHookDisablePlayerLook_Code : Xbyak::CodeGenerator {
				InstallHookDisablePlayerLook_Code(void * buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label normal;
					Xbyak::Label funcLabel;
					Xbyak::Label retn;

					push(rax);
					push(rcx);

					mov(cl, al);

					mov(rax, Settings::bLockOn);
					test(al, al);
					je(normal);

					push(rcx);
					push(rdx);
					sub(rsp, 0x20);

					mov(rax, Tralala::g_playerCameraAddr);
					xor(rdx, rdx);
					mov(ptr[rsp], rdx);
					lea(rdx, ptr[rsp]);
					mov(rcx, ptr[rax]);
					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);
					pop(rdx);
					pop(rcx);

					test(al, al);
					je(normal);

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
			InstallHookDisablePlayerLook_Code code(codeBuf, GetFnAddr(Tralala::IsValidFaceToFace));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write6Branch(g_disablePlayerLookAddr, uintptr_t(code.getCode())))
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
			InstallHookOnPlayerDeath_Code code(codeBuf, GetFnAddr(Tralala::OnPlayerDeath));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write6Branch(g_onPlayerDeathAddr, uintptr_t(code.getCode())))
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
			InstallHookSetFreeLook_Code code(codeBuf, GetFnAddr(Tralala::SetFreeLook));
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
			InstallHookTargetLocation_Code code(codeBuf, GetFnAddr(Tralala::SetTargetLocationHook));
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
			InstallHookClearTarget_Code code(codeBuf, GetFnAddr(Tralala::ClearHeadTrackingTargetHook));
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
			InstallHookBeginSaving_Code code(codeBuf, GetFnAddr(Tralala::BeginSavingHook));
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
			InstallHookFinishSavingCode code(codeBuf, GetFnAddr(Tralala::FinishSavingHook));
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
			InstallHookCameraHeightCode code(codeBuf, GetFnAddr(Tralala::GetThirdPersonCameraHeight_Hook));
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
			InstallHookThirdCamColCode code(codeBuf, GetFnAddr(Tralala::TPCamProcessCollision_Hook));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write5Branch(g_thirdCamColAddr, uintptr_t(code.getCode())))
				return false;
		}

		{
			struct InstallHookCameraCollisionCode : Xbyak::CodeGenerator {
				InstallHookCameraCollisionCode(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
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
					dq(g_camCollisionAddr + 0x5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookCameraCollisionCode code(codeBuf, GetFnAddr(Tralala::CameraProcessCollision_Hook));
			g_localTrampoline.EndAlloc(code.getCurr());


			if (!g_branchTrampoline.Write5Branch(g_camCollisionAddr, uintptr_t(code.getCode())))
				return false;
		}

		Menus::InstallHook();
		Graphics::InstallHook();

		SafeWrite8(g_firstFOVAddr + 0x3, 0xB8);	// change xmm6 to xmm7

		return true;
	}
}