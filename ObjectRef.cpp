#include "ObjectRef.h"

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

	bool LookupRefByHandle(UInt32 * refHandle, TESObjectREFR ** refrOut)
	{
		typedef bool(*LookupREFRByHandle_t)(UInt32 * refHandle, TESObjectREFR ** refrOut);
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

	bool Actor::IsOnMount()
	{
		typedef bool(*IsOnMount_t)(Actor*);
		IsOnMount_t IsOnMount = (IsOnMount_t)g_isonmountAddr;

		return IsOnMount(this);
	}

	bool Actor::IsTalking()
	{
		if ((flags1 & 0x180) != 0x180)
			return true;

		if (unk108 > 0)
			return true;

		return false;
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

	void Actor::GetTargetBonePosition(NiPoint3 * pos)
	{
		BGSBodyPartData* bodyPart = race->bodyPartData;

		if (bodyPart)
		{
			NiAVObject* object = (NiAVObject*)GetNiNode();
			if (object)
			{
				const char * necknameData;

				if (bodyPart->formID == 0x13492 || bodyPart->formID == 0x401BC4A){
					necknameData = "NPC NeckHub";
				}
				else if (bodyPart->formID == 0xBA549){
					necknameData = "Mcrab_Body";
				}
				else if (bodyPart->formID == 0x4FBF5) {
					necknameData = "Canine_Neck2";
				}
				else if (bodyPart->formID == 0x60716){
					necknameData = "HorseNeck4";
				}
				else if (bodyPart->formID == 0x20E26){
					necknameData = "Sabrecat_Neck[Nek2]";
				}
				else if (bodyPart->formID == 0x76B30){
					necknameData = "ElkNeck4";
				}
				else if (bodyPart->formID == 0x868FC){
					necknameData = "NPC Neck2";
				}
				else if (bodyPart->formID == 0x6DC9C){
					necknameData = "RabbitNeck2";
				}
				else if (bodyPart->formID == 0xA919E){
					necknameData = "Neck2";
				}
				else if (bodyPart->formID == 0x8691C){
					necknameData = "FireAtronach_Neck [Neck]";
				}
				else if (bodyPart->formID == 0x6B7C9){
					necknameData = "Jaw";
				}
				else if (bodyPart->formID == 0x517AB || bodyPart->formID == 0x4028537){
					necknameData = "NPC Neck [Neck]2";
				}
				else if (bodyPart->formID == 0x59060){
					necknameData = "[Neck3]";
				}
				else if (bodyPart->formID == 0x2005205){
					necknameData = "ChaurusFlyerNeck";
				}
				else if (bodyPart->formID == 0x4E782){
					necknameData = "Neck3";
				}
				else if (bodyPart->formID == 0x43592){
					necknameData = "DragPriestNPC Neck [Neck]";
				}
				else if (bodyPart->formID == 0x5DDA2 || bodyPart->formID == 0x4017F53){
					necknameData = "NPC Neck";
				}
				else if (bodyPart->formID == 0x17929){
					necknameData = "[body]";
				}
				else if (bodyPart->formID == 0x6F276){
					necknameData = "Goat_Neck4";
				}
				else if (bodyPart->formID == 0x8CA6B){
					necknameData = "Horker_Neck4";
				}
				else if (bodyPart->formID == 0x538F9){
					necknameData = "IW Seg01";
				}
				else if (bodyPart->formID == 0x59255){
					necknameData = "Mammoth Neck";
				}
				else if (bodyPart->formID == 0x264EF){
					necknameData = "Neck";
				}
				else if (bodyPart->formID == 0x42529){
					necknameData = "Wisp Neck";
				}
				else if (bodyPart->formID == 0x86F43){
					necknameData = "Witchlight Body Lag";
				}
				else if (bodyPart->formID == 0x40C6A){
					necknameData = "SlaughterfishNeck";
				}
				else if (bodyPart->formID == 0x4019AD2){
					necknameData = "Neck [Neck]";
				}
				else if (bodyPart->formID == 0x401FEB9){
					necknameData = "NetchPelvis [Pelv]";
				}
				else if (bodyPart->formID == 0x401E2A3){
					necknameData = "Boar_Reikling_Neck";
				}
				else if (bodyPart->formID == 0x401DCBC){
					necknameData = "NPC COM [COM ]";
				}
				else if (bodyPart->formID == 0x402B018){
					necknameData = "MainBody";
				}
				else if (bodyPart->formID == 0x7874D){
					necknameData = "NPC Spine2";
				}
				else if (bodyPart->formID == 0x81C7A){
					necknameData = "DwarvenSpiderBody";
				}
				else if (bodyPart->formID == 0x800EC){
					necknameData = "NPC LowerJaw";
				}
				else{
					necknameData = "NPC Neck [Neck]";
				}

				BSFixedString neckname(necknameData);

				object = object->GetObjectByName(&neckname.data);
				if (object)
				{
					pos->x = object->m_worldTransform.pos.x;
					pos->y = object->m_worldTransform.pos.y;
					pos->z = object->m_worldTransform.pos.z;

				}
				else
				{
					GetMarkerPosition(pos);
				}

				neckname.Release();
			}
			else
			{
				GetMarkerPosition(pos);
			}
		}
		else
		{
			GetMarkerPosition(pos);
		}
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

		isNPCVar.Release();	
		
		return ret;
	}

	bool PlayerCharacter::GetIsNPCAnimVar()
	{
		BSFixedString isNPCVar("IsNPC");
		bool ret = false;

		if (this)
			this->animGraphHolder.GetAnimationVariableBool(isNPCVar, ret);

		isNPCVar.Release();

		return ret;
	}
}
