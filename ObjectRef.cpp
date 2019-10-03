#include "ObjectRef.h"
#include "skse64/NiNodes.h"

namespace Tralala
{
	uintptr_t g_playerCharacterAddr = 0;
	uintptr_t g_invalidRefHandleAddr = 0;
	uintptr_t g_lookupAddr = 0;
	uintptr_t g_isonmountAddr = 0;
	uintptr_t g_setAngleZAddr = 0;
	uintptr_t g_setAngleXAddr = 0;
	uintptr_t g_equipWeaponAddr = 0;
	uintptr_t g_setAnimBoolAddr = 0;
	uintptr_t g_setTargetLocAddr = 0;
	uintptr_t g_clearHeadTrackAddr = 0;
	uintptr_t g_setDialogueAddr = 0;
	uintptr_t g_isCastingAddr = 0;
	uintptr_t g_isSneakingAddr = 0;
	uintptr_t g_sneakingHeightAddr = 0;
	uintptr_t g_cameraHeightAddr = 0;
	uintptr_t g_getFurnHandleAddr = 0;
	uintptr_t g_getbhkWorldMAddr = 0;
	uintptr_t g_unk_0x5ECF90Addr = 0;
	uintptr_t g_isInAirAddr = 0;

	void ObjectRefGetAddresses()
	{
		const std::array<BYTE, 8> pattern = { 0x8B, 0x40, 0x10, 0xC1, 0xE8, 0x17, 0xA8, 0x01 };
		g_playerCharacterAddr = (uintptr_t)scan_memory_data(pattern, 0x23, true, 0x3, 0x7);

		const std::array<BYTE, 10> invpattern = { 0x8B, 0x0C, 0x03, 0x89, 0x4C, 0x24, 0x40, 0x48, 0x8B, 0xD6 };
		g_invalidRefHandleAddr = (uintptr_t)scan_memory_data(invpattern, 0x47, true, 0x2, 0x6);

		const std::array<BYTE, 7> lookpattern = { 0x48, 0x6B, 0xD8, 0x78, 0x48, 0x8B, 0xCE };
		g_lookupAddr = (uintptr_t)scan_memory_data(lookpattern, 0x32, true, 0x1, 0x5);

		const std::array<BYTE, 7> mountpattern = { 0x48, 0x83, 0xC2, 0x54, 0x0F, 0x28, 0xDE };
		g_isonmountAddr = (uintptr_t)scan_memory_data(mountpattern, 0x7A, false, 0x1, 0x5);

		const std::array<BYTE, 8> angleZpattern = { 0x8B, 0xC2, 0xC1, 0xE8, 0x15, 0x83, 0xE0, 0x0F };
		g_setAngleZAddr = (uintptr_t)scan_memory(angleZpattern, 0x6, false);

		const std::array<BYTE, 12> angleXpattern = { 0xC1, 0xE9, 0x05, 0x83, 0xE1, 0x07, 0x83, 0xE9, 0x03, 0x83, 0xF9, 0x02 };
		g_setAngleXAddr = (uintptr_t)scan_memory(angleXpattern, 0x6B, false);

		const std::array<BYTE, 6> eqWeappattern = { 0x48, 0x89, 0x3E, 0x8B, 0x45, 0x5F };
		g_equipWeaponAddr = (uintptr_t)scan_memory_data(eqWeappattern, 0x40, true, 0x1, 0x5);

		const std::array<BYTE, 8> animBpattern = { 0x4C, 0x8B, 0x03, 0x48, 0x8D, 0x54, 0x24, 0x70 };
		g_setAnimBoolAddr = (uintptr_t)scan_memory_data(animBpattern, 0x3D, true, 0x1, 0x5);

		const std::array<BYTE, 11> tgtLocpattern = { 0xC6, 0x45, 0x67, 0x00, 0x48, 0x8B, 0x86, 0xF0, 0x01, 0x00, 0x00 };
		g_setTargetLocAddr = (uintptr_t)scan_memory_data(tgtLocpattern, 0xA4, true, 0x1, 0x5);

		const std::array<BYTE, 7> clearpattern = { 0x41, 0x8B, 0x00, 0x49, 0x83, 0xE8, 0x04 };
		g_clearHeadTrackAddr = (uintptr_t)scan_memory(clearpattern, 0xEE, false);

		const std::array<BYTE, 6> diapattern = { 0x4C, 0x8B, 0xCB, 0x40, 0x84, 0xF6};
		g_setDialogueAddr = (uintptr_t)scan_memory_data(diapattern, 0x63, true, 0x1, 0x5);

		const std::array<BYTE, 6> castpattern = { 0x41, 0x8B, 0xEF, 0x4C, 0x8B, 0xF2 };
		g_isCastingAddr = (uintptr_t)scan_memory(castpattern, 0x24, false);

		const std::array<BYTE, 8> sneakpattern = { 0x41, 0x8B, 0xC4, 0x25, 0x00, 0x00, 0xFF, 0xFF };
		g_isSneakingAddr = (uintptr_t)scan_memory_data(sneakpattern, 0x54, true, 0x1, 0x5);

		const std::array<BYTE, 6> sneakHpattern = { 0x48, 0x8B, 0x06, 0x0F, 0x28, 0xC8 };
		g_sneakingHeightAddr = (uintptr_t)scan_memory_data(sneakHpattern, 0x9E, true, 0x1, 0x5);

		g_cameraHeightAddr = (uintptr_t)scan_memory_data(sneakHpattern, 0xA5, true, 0x1, 0x5);

		const std::array<BYTE, 8> furnpattern = { 0x48, 0x05, 0x08, 0x02, 0x00, 0x00, 0x8B, 0x00 };
		g_getFurnHandleAddr = (uintptr_t)scan_memory(furnpattern, 0x9, false);

		const std::array<BYTE, 8> bhkWorldpattern = { 0x45, 0x0F, 0xB6, 0xC1, 0x41, 0x0F, 0xB6, 0xC9 };
		g_getbhkWorldMAddr = (uintptr_t)scan_memory_data(bhkWorldpattern, 0x1F, true, 0x1, 0x5);

		const std::array<BYTE, 8> unkpattern = { 0x0F, 0x28, 0xDF, 0x4C, 0x8D, 0x44, 0x24, 0x60 };
		g_unk_0x5ECF90Addr = (uintptr_t)scan_memory_data(unkpattern, 0x7E, true, 0x1, 0x5);

		const std::array<BYTE, 7> airpattern = { 0x83, 0x7D, 0x50, 0x00, 0x0F, 0x94, 0xC3 };
		g_isInAirAddr = (uintptr_t)scan_memory_data(airpattern, 0x9B, false, 0x1, 0x5);
	}

	void ActorProcessManager::SetTargetLocation(TESObjectREFR* source, NiPoint3* location)
	{
		typedef void(*SetTargetLocation_t)(ActorProcessManager*, TESObjectREFR*, NiPoint3*);
		SetTargetLocation_t SetTargetLocation = (SetTargetLocation_t)g_setTargetLocAddr;

		SetTargetLocation(this, source, location);
	}

	void ActorProcessManager::SetDialogueHeadTrackingTarget(TESObjectREFR* target)
	{
		typedef void(*SetDialogueHTTarget_t)(ActorProcessManager*, TESObjectREFR*);
		SetDialogueHTTarget_t SetTarget = (SetDialogueHTTarget_t)g_setDialogueAddr;

		SetTarget(this, target);
	}

	void ActorProcessManager::ClearHeadTracking()
	{
		typedef void(*ClearHeadTracking_t)(ActorProcessManager*);
		ClearHeadTracking_t ClearHeadTracking = (ClearHeadTracking_t)g_clearHeadTrackAddr;

		ClearHeadTracking(this);
	}

	UInt32* ActorProcessManager::GetFurnitureHandle(UInt32* handle)
	{
		typedef UInt32* (*GetFurnitureHandle_t)(ActorProcessManager*, UInt32*);
		GetFurnitureHandle_t GetFurnitureHandle = (GetFurnitureHandle_t)g_getFurnHandleAddr;

		return GetFurnitureHandle(this, handle);
	}

	bool IAnimationGraphManagerHolder::SetAnimationVariableBool(const BSFixedString& variableName, bool value)
	{
		typedef bool(*SetAnimVarBool_t)(IAnimationGraphManagerHolder*, const BSFixedString&, bool);
		SetAnimVarBool_t SetAnimVarBool = (SetAnimVarBool_t)g_setAnimBoolAddr;

		return SetAnimVarBool(this, variableName, value);
	}

	UInt32 InvalidRefHandle()
	{
		return *(UInt32*)g_invalidRefHandleAddr;
	}

	bool LookupRefByHandle(UInt32& refHandle, NiPointer<TESObjectREFR>& refrOut)
	{
		typedef bool(*LookupREFRByHandle_t)(UInt32 &, NiPointer<TESObjectREFR> &);
		LookupREFRByHandle_t LookupREFRByHandle = (LookupREFRByHandle_t)g_lookupAddr;

		return LookupREFRByHandle(refHandle, refrOut);
	}

	float TESObjectREFR::GetDistance(TESObjectREFR * target)
	{
		float x, y, z;

		x = pos.x - target->pos.x;
		y = pos.y - target->pos.y;
		z = pos.z - target->pos.z;

		return sqrt(x*x + y*y + z*z);
	}

	float TESObjectREFR::GetTargetHeight()
	{
		NiPoint3 p1, p2;
		return GetBoundRightBackTop(&p1)->z - GetBoundLeftFrontBottom(&p2)->z;
	}

	float TESObjectREFR::GetTargetWidth()
	{	
		NiPoint3 p1, p2;
		return GetBoundRightBackTop(&p1)->x - GetBoundLeftFrontBottom(&p2)->x;
	}

	void* TESObjectREFR::GetbhkWorldM()
	{
		if (!parentCell)
			return nullptr;

		typedef void* (*GetbhkWorldM_t)(TESObjectCELL*);
		GetbhkWorldM_t GetbhkWorldM = (GetbhkWorldM_t)g_getbhkWorldMAddr;

		return GetbhkWorldM(parentCell);
	}

	void TESObjectREFR::IncRef()
	{
		handleRefObject.IncRef();
	}

	void TESObjectREFR::DecRef()
	{
		handleRefObject.DecRef();
	}

	bool Actor::IsOnMount()
	{
		typedef bool(*IsOnMount_t)(Actor*);
		IsOnMount_t IsOnMount = (IsOnMount_t)g_isonmountAddr;

		return IsOnMount(this);
	}

	bool Actor::IsOnCarriage()
	{
		UInt32 handle = 0;
		processManager->GetFurnitureHandle(&handle);

		UInt32 sitState = actorState.GetSitState();
		if ((sitState == ActorState::kSitState_Sitting) && (handle == InvalidRefHandle()))
			return true;

		return false;
	}

	bool Actor::IsTalking()
	{
		if ((flags1 & 0x180) != 0x180)
			return true;

		if (unk108 > 0)
			return true;

		return false;
	}

	bool Actor::IsInAir()
	{
		typedef bool(*IsInAir_t)(Actor*);
		IsInAir_t IsInAir = (IsInAir_t)g_isInAirAddr;

		return IsInAir(this);
	}

	void Actor::SetAngleX(float angle)
	{
		typedef void(*SetAngleX_t)(Actor * actor, float angle);
		SetAngleX_t SetAngleX = (SetAngleX_t)g_setAngleXAddr;

		SetAngleX(this, angle);
	}
	
	void Actor::SetAngleZ(float angle)
	{
		typedef void(*SetAngleZ_t)(Actor * actor, float angle);
		SetAngleZ_t SetAngleZ = (SetAngleZ_t)g_setAngleZAddr;

		SetAngleZ(this, angle);
	}

	void Actor::GetTargetNeckPosition(NiPoint3 * pos)
	{
		NiAVObject* node = this->GetNiNode();
		if (!node)
			return GetMarkerPosition(pos);
		
		BSFixedString neckName("NPC Neck [Neck]");
		node = node->GetObjectByName(&neckName.data);
		if (!node)
			return GetMarkerPosition(pos);

		pos->x = node->m_worldTransform.pos.x;
		pos->y = node->m_worldTransform.pos.y;
		pos->z = node->m_worldTransform.pos.z;
	}

	TESObjectWEAP* Actor::GetEquippedWeapon(bool isLeftHand)
	{
		typedef TESObjectWEAP * (*GetEquippedWeapon_t)(Actor * actor, bool isLeftHand);
		GetEquippedWeapon_t GetEquippedWeapon = (GetEquippedWeapon_t)g_equipWeaponAddr;

		return GetEquippedWeapon(this, isLeftHand);
	}

	bool Actor::IsFlyingActor()
	{
		return ((race->data.raceFlags & TESRace::kRace_Flies) == TESRace::kRace_Flies);
	}

	bool Actor::IsNotInFurniture()
	{
		return ((actorState.flags08 & 0x0003C000) == 0);
	}

	bool Actor::IsCasting(MagicItem* spell)
	{
		typedef bool(*IsCasting_t)(Actor*, MagicItem*);
		IsCasting_t IsCasting = (IsCasting_t)g_isCastingAddr;

		return IsCasting(this, spell);
	}

	bool Actor::GetTargetHeadNodePosition(NiPoint3 * pos, bool * compare)
	{
		if (!processManager)
			return false;

		if (!processManager->middleProcess)
			return false;

		if (!processManager->middleProcess->unk158)
			return false;

		NiNode* headNode = (NiNode*)processManager->middleProcess->unk158;

		if (*compare)
		{
			BSFixedString headName("NPC Head [Head]");
			BSFixedString name(headNode->m_name);
			if (name == headName)
				*compare = true;
			else
				*compare = false;
		}

		pos->x = headNode->m_worldTransform.pos.x;
		pos->y = headNode->m_worldTransform.pos.y;
		pos->z = headNode->m_worldTransform.pos.z;

		

		return true;
	}
	
	bool Actor::IsSneaking()
	{
		typedef bool(*IsSneaking_t)(Actor*);
		IsSneaking_t IsSneaking = (IsSneaking_t)g_isSneakingAddr;

		return IsSneaking(this);
	}

	float Actor::GetSneakingHeight(bool isCamera)
	{
		typedef float(*GetSneakingHeight_t)(Actor*, bool);
		GetSneakingHeight_t GetSneakingHeight = (GetSneakingHeight_t)g_sneakingHeightAddr;

		return GetSneakingHeight(this, isCamera);
	}

	float Actor::GetCameraHeight()
	{
		typedef float(*GetCameraHeight_t)(Actor*);
		GetCameraHeight_t GetCameraHeight = (GetCameraHeight_t)g_cameraHeightAddr;

		return GetCameraHeight(this);
	}

	void Actor::Unk_0x5ECF90()
	{
		typedef void(*Unk_0x5ECF90_t)(Actor*);
		Unk_0x5ECF90_t Unk_0x5ECF90 = (Unk_0x5ECF90_t)g_unk_0x5ECF90Addr;

		return Unk_0x5ECF90(this);
	}

	PlayerCharacter * PlayerCharacter::GetSingleton()
	{
		return *(PlayerCharacter**)g_playerCharacterAddr;
	}

	bool PlayerCharacter::SetIsNPCAnimVar(bool var)
	{
		BSFixedString isNPCVar("IsNPC");
		bool ret = false;

		if (this)
			ret = this->animGraphHolder.SetAnimationVariableBool(isNPCVar, var);
		
		return ret;
	}

	bool PlayerCharacter::GetIsNPCAnimVar()
	{
		BSFixedString isNPCVar("IsNPC");
		bool ret = false;

		if (this)
			this->animGraphHolder.GetAnimationVariableBool(isNPCVar, ret);

		return ret;
	}
}
