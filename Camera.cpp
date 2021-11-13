#include "Camera.h"
#include "ObjectRef.h"
#include "skse64/NiNodes.h"

namespace Tralala
{
	uintptr_t g_playerCameraAddr = 0;
	uintptr_t g_setCameraStateAddr = 0;
	uintptr_t g_pushTargetCamAddr = 0;
	uintptr_t g_pushCurrentCamAddr = 0;
	uintptr_t g_setCamTargetAddr = 0;
	uintptr_t g_forceFirstPersonAddr = 0;
	uintptr_t g_updateThirdPersonAddr = 0;
	uintptr_t g_getPenetrationAddr = 0;
	uintptr_t g_processCamColAddr = 0;
	uintptr_t g_getClosestPointAddr = 0;
	uintptr_t g_camProcessColAddr = 0;

	void PlayerCameraGetAddress()
	{
		const std::array<BYTE, 8> pattern = { 0xF3, 0x0F, 0x11, 0x47, 0x3C, 0x48, 0x85, 0xC9 };
		g_playerCameraAddr = (uintptr_t)scan_memory(pattern, 0x14, true, 0x3, 0x7);

		const std::array<BYTE, 6> camStatepattern = { 0x48, 0x39, 0x41, 0x28, 0x74, 0x19 };
		g_setCameraStateAddr = (uintptr_t)scan_memory(camStatepattern, 0x1A, true, 0x1, 0x5);

		const std::array<BYTE, 7> camTargetpattern = { 0x89, 0x41, 0x3C, 0x48, 0x83, 0xC4, 0x20 };
		g_setCamTargetAddr = (uintptr_t)scan_memory(camTargetpattern, 0x1C, false);

		const std::array<BYTE, 6> firstPersonpattern = { 0x48, 0x39, 0x41, 0x28, 0x74, 0x19 };
		g_forceFirstPersonAddr = (uintptr_t)scan_memory(firstPersonpattern, 0x1B, false);

		const std::array<BYTE, 11> camColpattern = { 0x48, 0x8D, 0x51, 0x40, 0x48, 0x8B, 0x49, 0x10, 0x41, 0xB0, 0x01 };
		g_processCamColAddr = (uintptr_t)scan_memory(camColpattern, 0xD, false);

		const std::array<BYTE, 12> penetpattern = { 0x49, 0x8B, 0xF0, 0x48, 0x8B, 0xFA, 0x4C, 0x8B, 0xE9, 0x45, 0x32, 0xE4 };
		g_getPenetrationAddr = (uintptr_t)scan_memory(penetpattern, 0x3A, false);

		const std::array<BYTE, 17> closestpattern = { 0x4C, 0x8D, 0x4C, 0x24, 0x60, 0x4C, 0x8D, 0x44, 0x24, 0x40, 0x48, 0x8B, 0x8F, 0x20, 0x01, 0x00, 0x00 };
		g_getClosestPointAddr = (uintptr_t)scan_memory(closestpattern, 0x11, true, 0x1, 0x5);

		const std::array<BYTE, 11> processColPattern = { 0x48, 0x8D, 0x51, 0x40, 0x48, 0x8B, 0x49, 0x10, 0x41, 0xB0, 0x01 };
		g_camProcessColAddr = (uintptr_t)scan_memory(processColPattern, 0xB, true, 0x1, 0x5);
	}

	float ThirdPersonState::GetDistanceWithinTargetHead(Actor* target)
	{
		if (!target || !target->processManager || !target->processManager->middleProcess)
			return FLT_MAX;

		NiNode* headNode = (NiNode*)target->processManager->middleProcess->unk158;
		if (!headNode)
			return FLT_MAX;

		NiPoint3 dist;
		dist.x = camPos.x - headNode->m_worldTransform.pos.x;
		dist.y = camPos.y - headNode->m_worldTransform.pos.y;
		dist.z = camPos.z - headNode->m_worldTransform.pos.z;

		return (dist.x * dist.x + dist.y * dist.y + dist.z * dist.z);
	}

	void ThirdPersonState::SetFirstPersonSmooth(float minZoom, bool overShoulder)
	{
		if (unkDD[0] == 0)
		{
			savedZoom = curPosY;
			dstPosY = minZoom;
			unkDD[0] = 1;
		}
		else
		{
			curPosY = dstPosY;
		}

		if (!overShoulder)
			fOverShoulderPosX = fOverShoulderCombatAddY = fOverShoulderPosZ = 0.0f;
	}

	void ThirdPersonState::SetShoulderPos(const NiPoint3& pos)
	{
		this->fOverShoulderPosX = pos.x;
		this->fOverShoulderPosZ = pos.z;
		this->fOverShoulderCombatAddY = pos.y;
	}

	void ThirdPersonState::ProcessCameraCollision()
	{
		typedef void(*ProcessCamCol_t)(ThirdPersonState*);
		ProcessCamCol_t ProcessCamCol = (ProcessCamCol_t)g_processCamColAddr;

		ProcessCamCol(this);
	}

	void TESCamera::SetCameraState(TESCameraState* camState)
	{
		typedef UInt32(*SetCameraState_t)(TESCamera* camera, TESCameraState* cameraState);
		SetCameraState_t SetCameraState = (SetCameraState_t)g_setCameraStateAddr;

		SetCameraState(this, camState);
	}

	NiCamera* TESCamera::GetNiCamera()
	{
		NiCamera* camera = nullptr;
		std::size_t size = cameraNode->m_children.m_arrayBufLen;
		for (std::size_t i = 0; i < size; ++i)
		{
			NiAVObject* pObj = cameraNode->m_children.m_data[i];
			if (!pObj)
				continue;

			if (strcmp(pObj->GetRTTI()->name, "NiCamera") == 0)
			{
				camera = (NiCamera*)pObj;
				break;
			}

		}
		return camera;
	}

	PlayerCamera* PlayerCamera::GetSingleton()
	{
		return *(PlayerCamera**)g_playerCameraAddr;
	}

	bool PlayerCamera::IsCameraFirstPerson()
	{
		return cameraState == cameraStates[kCameraState_FirstPerson];
	}

	bool PlayerCamera::IsCameraThirdPerson()
	{
		return cameraState == cameraStates[kCameraState_ThirdPerson2];
	}

	TESCameraState* PlayerCamera::ProcessCameraTransition(UInt32 srcCamState, UInt32 dstCamState, bool face2face)
	{
		TESCameraState* target = cameraStates[srcCamState];

		TweenMenuCameraState* tween = (TweenMenuCameraState*)cameraStates[kCameraState_TweenMenu];
		if (cameraState == tween)
		{
			typedef SInt32(*PushTargetCamState_t)(Data40* data40, TESCameraState* camState);
			PushTargetCamState_t PushTargetCamState = (PushTargetCamState_t)g_pushTargetCamAddr;

			PushTargetCamState(&unk40, target);
		}
		else
		{
			typedef SInt32(*PushCurrentCamState_t)(Data40* data40, TESCameraState** camState);
			PushCurrentCamState_t PushCurrentCamState = (PushCurrentCamState_t)g_pushCurrentCamAddr;

			if (face2face)
			{
				TESCameraState* destination = cameraStates[dstCamState];
				PushCurrentCamState(&unk40, &destination);
			}
			else
			{
				PushCurrentCamState(&unk40, &cameraState);
			}

			SetCameraState(target);
		}

		return target;
	}

	void PlayerCamera::SetCameraTarget(Actor* target)
	{
		typedef void(*SetCameraTarget_t)(PlayerCamera* camera, Actor* target);
		SetCameraTarget_t SetCameraTarget = (SetCameraTarget_t)g_setCamTargetAddr;

		SetCameraTarget(this, target);
	}

	void PlayerCamera::ForceFirstPerson()
	{
		typedef void(*ForceFirstPerson_t)(PlayerCamera* camera);
		ForceFirstPerson_t ForceFirstPerson = (ForceFirstPerson_t)g_forceFirstPersonAddr;

		ThirdPersonState* tps = GetThirdPersonCamera();
		tps->savedZoom = tps->curPosY;

		ForceFirstPerson(this);
	}

	void PlayerCamera::ForceThirdPerson()
	{
		ThirdPersonState* tps = GetThirdPersonCamera();

		tps->basePosX = tps->fOverShoulderPosX;
		tps->basePosY = tps->fOverShoulderCombatAddY;
		tps->basePosZ = tps->fOverShoulderPosZ;
		tps->dstPosY = tps->savedZoom;

		SetCameraState(tps);
	}

	void PlayerCamera::UpdateThirdPersonMode(bool weaponDrawn)
	{
		typedef void(*UpdateThirdPersonMode_t)(PlayerCamera* camera, bool weaponDrawn);
		UpdateThirdPersonMode_t UpdateThirdPersonMode = (UpdateThirdPersonMode_t)g_updateThirdPersonAddr;

		UpdateThirdPersonMode(this, weaponDrawn);
	}

	bool PlayerCamera::GetDistanceWithTargetBone(Actor* target, NiPoint3* dist)
	{
		NiPoint3 targetPos;

		target->GetTargetNeckPosition(&targetPos);

		dist->x = targetPos.x - cameraPos.x;
		dist->y = targetPos.y - cameraPos.y;
		dist->z = targetPos.z - cameraPos.z;

		return true;
	}

	float PlayerCamera::GetDistanceWithTargetBone(Actor* target, bool firstPerson)
	{
		NiPoint3 neckPos;
		NiPoint3 camPos;

		target->GetTargetNeckPosition(&neckPos);

		if (firstPerson)
		{
			PlayerCharacter* player = PlayerCharacter::GetSingleton();

			camPos.x = player->pos.x;
			camPos.y = player->pos.y;

			FirstPersonState* fps = GetFirstPersonCamera();
			if (fps->cameraNode)
			{
				camPos.z = player->pos.z + fps->cameraNode->m_localTransform.pos.z;
			}
			else
			{
				player->GetMarkerPosition(&camPos);
			}
		}
		else
		{
			ThirdPersonState* tps = GetThirdPersonCamera();
			camPos = tps->camPos;
		}

		float dx = neckPos.x - camPos.x;
		float dy = neckPos.y - camPos.y;
		float dz = neckPos.z - camPos.z;

		return sqrt(dx * dx + dy * dy + dz * dz);
	}

	bool PlayerCamera::GetPenetration(Actor* target, NiPoint3* sourcePos, float radiusToCheck,
		float offsetActorFromGround)
	{
		typedef bool(*GetPenetration_t)(SimpleShapePhantom*, Actor*, NiPoint3*, float, float);
		GetPenetration_t GetPenetration = (GetPenetration_t)g_getPenetrationAddr;

		return GetPenetration(simpleShapePhantoms, target, sourcePos, radiusToCheck, offsetActorFromGround);
	}

	bool PlayerCamera::GetClosestPoint(void* bhkWorldM, NiPoint3* sourcePos, NiPoint3* resultPos,
		Havok::hkpRootCdPoint* resultInfo, Actor** resultActor, float radiusToCheck)
	{
		typedef bool(*GetClosestPoint_t)(SimpleShapePhantom*, void*, NiPoint3*,
			NiPoint3*, Havok::hkpRootCdPoint*, Actor**, float);
		GetClosestPoint_t GetClosestPoint = (GetClosestPoint_t)g_getClosestPointAddr;

		return GetClosestPoint(simpleShapePhantoms, bhkWorldM, sourcePos, resultPos,
			resultInfo, resultActor, radiusToCheck);
	}

	bool PlayerCamera::ProcessCollision(NiPoint3* camPos, bool isFade)
	{
		typedef bool(*ProcessCollision_t)(PlayerCamera*, NiPoint3*, bool);
		ProcessCollision_t ProcessCollision = (ProcessCollision_t)g_camProcessColAddr;

		return ProcessCollision(this, camPos, isFade);
	}

	bool PlayerCamera::IsInCameraView(NiPoint3* pos, int mode)
	{
		NiCamera* niCamera = GetNiCamera();
		if (!niCamera)
			return false;

		// project a world space point to screen space
		float w = pos->x * niCamera->m_aafWorldToCam[3][0] +
			pos->y * niCamera->m_aafWorldToCam[3][1] + pos->z * niCamera->m_aafWorldToCam[3][2] +
			niCamera->m_aafWorldToCam[3][3];

		// Check to see if we're on the appropriate side of the camera.
		if (w > 1e-5f)
		{
			float invW = 1.0f / w;

			float screenX = pos->x * niCamera->m_aafWorldToCam[0][0] + pos->y * niCamera->m_aafWorldToCam[0][1] +
				pos->z * niCamera->m_aafWorldToCam[0][2] + niCamera->m_aafWorldToCam[0][3];
			float screenY = pos->x * niCamera->m_aafWorldToCam[1][0] + pos->y * niCamera->m_aafWorldToCam[1][1] +
				pos->z * niCamera->m_aafWorldToCam[1][2] + niCamera->m_aafWorldToCam[1][3];

			screenX *= invW;
			screenY *= invW;

			screenX *= (niCamera->m_kPort.m_right - niCamera->m_kPort.m_left) * 0.5f;
			screenY *= (niCamera->m_kPort.m_top - niCamera->m_kPort.m_bottom) * 0.5f;

			screenX += (niCamera->m_kPort.m_right + niCamera->m_kPort.m_left) * 0.5f;
			screenY += (niCamera->m_kPort.m_top + niCamera->m_kPort.m_bottom) * 0.5f;

			// If on screen return true. Otherwise, we fall through to false.
			switch (mode)
			{
			case kRotate_Fast:
			{
				if (screenX >= 0.3125f && screenX <= 0.6875f &&
					screenY >= 0.28125f && screenY <= 0.71785f)
				{
					return true;
				}
				break;
			}
			case kRotate_Slow:
			{
				if (screenX >= 0.375f && screenX <= 0.625f &&
					screenY >= 0.34375f && screenY <= 0.65625f)
				{
					return true;
				}
				break;
			}
			case KRotate_Stop:
			{
				if (screenX >= 0.4375f && screenX <= 0.5625f &&
					screenY >= 0.40625f && screenY <= 0.59375f)
				{
					return true;
				}
				break;
			}
			default:
				return false;
			}
		}

		return false;

	}
}