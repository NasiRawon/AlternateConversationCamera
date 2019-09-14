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
		const std::array<BYTE, 9> pattern = { 0x8B, 0x4D, 0xC7, 0x0F, 0xB6, 0xC9, 0x83, 0xF8, 0x04 };
		g_playerCameraAddr = (uintptr_t)scan_memory_data(pattern, 0x21, true, 0x3, 0x7);

		const std::array<BYTE, 9> camStatepattern = { 0x48, 0x8B, 0x4F, 0x28, 0x48, 0x3B, 0xD9, 0x74, 0x29 };
		g_setCameraStateAddr = (uintptr_t)scan_memory(camStatepattern, 0x1F, false);

		const std::array<BYTE, 7> camTargetpattern = { 0x89, 0x41, 0x3C, 0x48, 0x83, 0xC4, 0x20 };
		g_setCamTargetAddr = (uintptr_t)scan_memory(camTargetpattern, 0x1C, false);

		const std::array<BYTE, 6> firstPersonpattern = { 0x48, 0x39, 0x41, 0x28, 0x74, 0x19 };
		g_forceFirstPersonAddr = (uintptr_t)scan_memory(firstPersonpattern, 0x1B, false);

		const std::array<BYTE, 8> camColpattern = { 0xF3, 0x0F, 0x59, 0x5F, 0x18, 0x0F, 0x28, 0xC7 };
		g_processCamColAddr = (uintptr_t)scan_memory_data(camColpattern, 0x7B, true, 0x1, 0x5);

		const std::array<BYTE, 8> penetpattern = { 0xF3, 0x0F, 0x10, 0x55, 0x80, 0x0F, 0x28, 0xCA };
		g_getPenetrationAddr = (uintptr_t)scan_memory_data(penetpattern, 0x7D, true, 0x1, 0x5);

		g_getClosestPointAddr = (uintptr_t)scan_memory_data(penetpattern, 0xF0, false, 0x1, 0x5);

		const std::array<BYTE, 6> processColPattern = { 0x0F, 0x28, 0xD8, 0x0F, 0x2F, 0xD8 };
		g_camProcessColAddr = (uintptr_t)scan_memory_data(processColPattern, 0xBF, false, 0x1, 0x5);

		//static const BYTE updateThirdPayload[] = { 0x41, 0x0F, 0xB6, 0xD1, 0x41, 0x0F, 0xB6, 0xD9 };
		//std::vector<BYTE> updateThirdpattern(updateThirdPayload, updateThirdPayload + sizeof(updateThirdPayload) / sizeof(updateThirdPayload[0]));
		//g_updateThirdPersonAddr = (uintptr_t)scan_memory_data(updateThirdpattern, 0x1F, true, 0x1, 0x5);
		
		//static const BYTE pushTargetPayload[] = { 0x48, 0x83, 0xC1, 0x40, 0x48, 0x8D, 0x53, 0x28 };
		//std::vector<BYTE> pushTargetpattern(pushTargetPayload, pushTargetPayload + sizeof(pushTargetPayload) / sizeof(pushTargetPayload[0]));
		//g_pushTargetCamAddr = (uintptr_t)scan_memory_data(pushTargetpattern, 0x13, false, 0x1, 0x5);

		//static const BYTE pushCurPayload[] = { 0x48, 0x83, 0xC1, 0x40, 0x48, 0x8D, 0x53, 0x28 };
		//std::vector<BYTE> pushCurpattern(pushCurPayload, pushCurPayload + sizeof(pushCurPayload) / sizeof(pushCurPayload[0]));
		//g_pushCurrentCamAddr = (uintptr_t)scan_memory_data(pushCurpattern, 0x8, true, 0x1, 0x5);
	}

	float ThirdPersonState::GetDistanceWithinTargetHead(Tralala::Actor * target)
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

		return (dist.x*dist.x + dist.y*dist.y + dist.z*dist.z);
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

	void TESCamera::SetCameraState(TESCameraState * camState)
	{
		typedef UInt32(*SetCameraState_t)(Tralala::TESCamera* camera, Tralala::TESCameraState* cameraState);
		SetCameraState_t SetCameraState = (SetCameraState_t)g_setCameraStateAddr;

		SetCameraState(this, camState);
	}

	PlayerCamera * PlayerCamera::GetSingleton()
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

	TESCameraState * PlayerCamera::ProcessCameraTransition(UInt32 srcCamState, UInt32 dstCamState, bool face2face)
	{
		TESCameraState * target = cameraStates[srcCamState];

		TweenMenuCameraState * tween = (TweenMenuCameraState*)cameraStates[kCameraState_TweenMenu];
		if (cameraState == tween)
		{
			typedef SInt32(*PushTargetCamState_t)(Data40 * data40, TESCameraState * camState);
			PushTargetCamState_t PushTargetCamState = (PushTargetCamState_t)g_pushTargetCamAddr;

			PushTargetCamState(&unk40, target);
		}
		else
		{
			typedef SInt32(*PushCurrentCamState_t)(Data40 * data40, TESCameraState ** camState);
			PushCurrentCamState_t PushCurrentCamState = (PushCurrentCamState_t)g_pushCurrentCamAddr;

			if (face2face)
			{
				TESCameraState * destination = cameraStates[dstCamState];
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
		typedef void(*ForceFirstPerson_t)(PlayerCamera * camera);
		ForceFirstPerson_t ForceFirstPerson = (ForceFirstPerson_t)g_forceFirstPersonAddr;

		ForceFirstPerson(this);
	}

	void PlayerCamera::ForceThirdPerson(bool smooth)
	{
		Tralala::ThirdPersonState * tps = GetThirdPersonCamera();

		tps->basePosX = tps->fOverShoulderPosX;
		tps->basePosY = tps->fOverShoulderCombatAddY;
		tps->basePosZ = tps->fOverShoulderPosZ;
		if (smooth)
			tps->dstPosY = tps->savedZoom;
		else
			tps->dstPosY = tps->curPosY;

		SetCameraState(tps);
	}

	void PlayerCamera::UpdateThirdPersonMode(bool weaponDrawn)
	{
		typedef void(*UpdateThirdPersonMode_t)(PlayerCamera * camera, bool weaponDrawn);
		UpdateThirdPersonMode_t UpdateThirdPersonMode = (UpdateThirdPersonMode_t)g_updateThirdPersonAddr;

		UpdateThirdPersonMode(this, weaponDrawn);
	}

	bool PlayerCamera::GetDistanceWithTargetBone(Tralala::Actor * target, NiPoint3 * dist)
	{
		if (!target || !target->GetNiNode())
			return false;

		NiPoint3 targetPos;

		target->GetTargetBonePosition(&targetPos);

		dist->x = targetPos.x - cameraPos.x;
		dist->y = targetPos.y - cameraPos.y;
		dist->z = targetPos.z - cameraPos.z;

		return true;
	}

	float PlayerCamera::GetDistanceWithTargetBone(Tralala::Actor * target, bool firstPerson)
	{
		NiPoint3 neckPos;
		NiPoint3 camPos;
		
		target->GetTargetBonePosition(&neckPos);

		if (firstPerson)
		{
			Tralala::PlayerCharacter * player = Tralala::PlayerCharacter::GetSingleton();

			camPos.x = player->pos.x;
			camPos.y = player->pos.y;

			FirstPersonState * fps = GetFirstPersonCamera();
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
			ThirdPersonState * tps = GetThirdPersonCamera();
			camPos = tps->camPos;
		}

		float dx = neckPos.x - camPos.x;
		float dy = neckPos.y - camPos.y;
		float dz = neckPos.z - camPos.z;

		return sqrt(dx*dx + dy*dy + dz*dz);
	}

	bool PlayerCamera::GetPenetration(Actor * target, NiPoint3* sourcePos, float radiusToCheck, 
		float offsetActorFromGround)
	{
		typedef bool(*GetPenetration_t)(SimpleShapePhantom*, Actor *, NiPoint3*, float, float);
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
}