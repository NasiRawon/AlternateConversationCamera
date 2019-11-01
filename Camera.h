#pragma once

#include "skse64/GameCamera.h"
#include "PatternScanner.h"
#include "Utils.h"
#include "Havok.h"

#define PlayerRefHandle 0x100000

namespace Tralala
{
	extern uintptr_t g_playerCameraAddr;
	extern uintptr_t g_pushTargetCamAddr;
	extern uintptr_t g_pushCurrentCamAddr;

	void PlayerCameraGetAddress(void);

	class Actor;

	// 20
	class TESCameraState
	{
	public:
		TESCameraState();
		virtual ~TESCameraState();

		virtual void OnStateStart();	// pure
		virtual void OnStateEnd();	// pure
		virtual void OnUpdate(TESCameraState * out);
		virtual void GetCameraRotation(NiQuaternion * rot);
		virtual void GetCameraPosition(NiPoint3 * pos);
		virtual void Unk_06();	// pure
		virtual void Unk_07();	// pure
		virtual void Unk_08();	// pure

		BSIntrusiveRefCounted	refCount;		// 08
		TESCamera				* camera;		// 10
		UInt32					stateId;		// 18
		UInt32					pad1C;			// 1C
	};

	// 90
	class FirstPersonState : public TESCameraState
	{
	public:
		FirstPersonState();
		virtual ~FirstPersonState();

		PlayerInputHandler		inputHandler;		// 20
		NiPoint3				actorPos;			// 30
		float					unk3C[3];			// 3C
		NiPoint3				unk48;				// 48
		NiNode					* cameraNode;		// 58
		NiNode					* controllerNode;	// 60
		float					sittingRot;			// 68
		float					unk6C[2];			// 6C
		UInt32					unk74;				// 74
		UInt32					unk78;				// 78
		float					unk7C;				// 7C
		UInt32					unk80;				// 80
		bool					disableRotX;		// 84 - lock rotX to 0 and cant rotate camera around x axis
		UInt8					unk85[3];			// 85
		UInt64					unk88;				// 88
	};
	STATIC_ASSERT(offsetof(FirstPersonState, sittingRot) == 0x68);
	STATIC_ASSERT(sizeof(FirstPersonState) == 0x90);

	// E8
	class ThirdPersonState : public TESCameraState
	{
	public:
		ThirdPersonState();
		virtual ~ThirdPersonState();
		virtual void SetCameraHandle(UInt32 * handle);			// set HorseCameraState + 0xE8
		virtual void SetOverShoulderPos(void);
		virtual void UpdateMode(bool weaponDrawn);
		virtual bool IsInCustomMode(void);						// animcam, fov-slide mode, idle
		virtual void ProcessCameraAngle(bool weaponSheathed);
		virtual void SetCameraRotation(void);					// set camRot (this + 0x4C)
		virtual void ProcessIdleCameraRot(NiPoint2 *);			// playercontrols + 0x2C = NiPoint2

		PlayerInputHandler		inputHandler;				// 20
		NiNode					* cameraNode;				// 30
		NiNode					* controllerNode;			// 38
		NiPoint3				camPos;						// 40
		NiQuaternion			camRot;						// 4C
		float					fOverShoulderPosX;			// 5C
		float					fOverShoulderCombatAddY;	// 60
		float					fOverShoulderPosZ;			// 64
		float					basePosX;					// 68
		float					basePosY;					// 6C
		float					basePosZ;					// 70
		float					dstPosY;					// 74
		float					curPosY;					// 78
		float					dstRotZ;					// 7C - camRotZ, this is same as PlayerCamera->unk154
		float					curRotZ;					// 80 - same as above
		float					savedZoom;					// 84
		float					unk88;						// 88 - related to y-axis
		float					unk8C;						// 8C - init'd 7F7FFFFF
		NiPoint3				collisionPos;				// 90
		float					unk9C;						// 9C - init'd 7F7FFFFF
		UInt64					unkA0;						// A0 - NiNode
		UInt64					unkA8;						// A8
		NiQuaternion			unkB0;						// B0 - use by slerp
		float					unkC0[5];					// C0
		float					diffRotZ;					// D4 - difference between camera and actor
		float					diffRotX;					// D8
		bool					isFreeLook;					// DC
		UInt8					unkDD[3];					// DD
		bool					toggleAnimCam;				// E0
		bool					toggleCamMode;				// E1 - 0 is CME Camera3rd node, 1 is normal Camera3rd node
		UInt8					padE2[5];					// E2

		float	GetDistanceWithinTargetHead(Tralala::Actor * target);
		void	SetFirstPersonSmooth(float minZoom, bool overShoulder = false);
		void	SetShoulderPos(const NiPoint3& pos);
		void	ProcessCameraCollision();
	};
	STATIC_ASSERT(offsetof(ThirdPersonState, diffRotZ) == 0xD4);
	STATIC_ASSERT(offsetof(ThirdPersonState, diffRotX) == 0xD8);
	STATIC_ASSERT(offsetof(ThirdPersonState, toggleAnimCam) == 0xE0);
	STATIC_ASSERT(sizeof(ThirdPersonState) == 0xE8);

	// 110
	class DragonCameraState : public ThirdPersonState
	{
	public:
		DragonCameraState();
		virtual ~DragonCameraState();

		UInt32	dragonHandle;	// 0E8
		UInt32	unkEC;			// 0EC
		UInt8	unkF0;			// 0F0
		UInt8	padF1[3];		// 0F1
		float	unkF4;			// 0F4 - init'd to 1
		UInt8	unkF8;			// 0F8
		UInt8	padF9[3];		// 0F9
		float	unkFC[4];		// 0FC
		UInt32	unk10C;			// 10C
	};

	// F8
	class HorseCameraState : public ThirdPersonState
	{
	public:
		HorseCameraState();
		virtual ~HorseCameraState();

		UInt32	horseHandle;	// E8 - same as PlayerCamera + 0x3C
		UInt32	unkEC;			// EC
		UInt8	unkF0;			// F0
		UInt8	padF1[7];		// F1
	};

	// 50
	class TweenMenuCameraState : public TESCameraState
	{
	public:
		TweenMenuCameraState();
		virtual ~TweenMenuCameraState();

		NiQuaternion	originCamStateRot;		// 20
		float			FOV;					// 30
		float			camRotXMod;				// 34 ???
		float			camRotZMod;				// 38 ???
		float			localRotXMod;			// 3C - related to selection mode
		float			localRotZMod;			// 40 - related to selection mode
		float			curZoomFOVMod;			// 44
		float			dstZoomFOVMod;			// 48 - FOV + (this)
		bool			unk4C;					// 4C
		UInt8			pad4D[3];				// 4D
	};

	// 78
	class TransitionState : public TESCameraState
	{
	public:
		TransitionState();
		virtual ~TransitionState();

		NiQuaternion	unk20;					// 20 
		NiQuaternion	unk30;					// 30
		NiPoint3		unk40;					// 40
		NiPoint3		unk4C;					// 4C
		UInt32			unk58;					// 58
		float			unk5C;					// 5C
		NiPoint3		camPos;					// 60
		float			unk6C;					// 6C
		TESCameraState*	unk70;					// 70
	};
	STATIC_ASSERT(sizeof(TransitionState) == 0x78);

	class MapCameraStates
	{
	public:
		// 78
		class World : public TESCameraState
		{
		public:
			World();
			virtual ~World();

			NiPoint3	unk20;		// 20
			float		unk2C[8];	// 2C
			float		unk4C;		// 4C
			float		unk50;		// 50
			float		unk54;		// 54
			float		unk58;		// 58
			UInt32		unk5C;		// 5C
			float		unk60;		// 60
			UInt32		unk64;		// 64
			void*		unk68;		// 68
			bool		unk70;		// 70
		};
		STATIC_ASSERT(sizeof(World) == 0x78);

		// 20
		class Exit : public TESCameraState
		{
		public:
			Exit();
			virtual ~Exit();

		};
		STATIC_ASSERT(sizeof(Exit) == 0x20);

		// 88
		class Transition : public TransitionState
		{
		public:
			Transition();
			virtual ~Transition();

			float		unk78;		// 78
			bool		unk7C;		// 7C
			UInt32		unk80;		// 80
		};
		STATIC_ASSERT(sizeof(Transition) == 0x88);
	};

	// 38
	class TESCamera
	{
	public:
		TESCamera();
		virtual ~TESCamera();

		virtual void SetNode(NiNode * node);
		virtual void Update();

		float		rotZ;	// 08
		float		rotX;	// 0C
		NiPoint3	pos;	// 10
		float		zoom;	// 1C
		NiNode		* cameraNode;	// 20 - First child is usually NiCamera
		TESCameraState	* cameraState;	// 28
		bool		isActive;			// 30
		UInt8		pad31[7];		// 31

		void SetCameraState(TESCameraState * newCamState);
		NiCamera* GetNiCamera();
	};
	STATIC_ASSERT(offsetof(TESCamera, cameraNode) == 0x20);
	STATIC_ASSERT(sizeof(TESCamera) == 0x38);

	// 168
	class PlayerCamera : public TESCamera
	{
	public:
		PlayerCamera();
		virtual ~PlayerCamera();

		enum
		{
			kCameraState_FirstPerson = 0,
			kCameraState_AutoVanity,
			kCameraState_VATS,
			kCameraState_Free,
			kCameraState_IronSights,
			kCameraState_Furniture,
			kCameraState_Transition,
			kCameraState_TweenMenu,
			kCameraState_ThirdPerson1,
			kCameraState_ThirdPerson2,
			kCameraState_Horse,
			kCameraState_Bleedout,
			kCameraState_Dragon,
			kNumCameraStates
		};

		enum
		{
			kRotate_Fast = 0,
			kRotate_Slow,
			KRotate_Stop
		};

		struct Data40 // seems to be bstsmallarray, this is very useful for retrieving previous camera state
		{
			UInt32	unk00;											// 00 - reserve space for savedCameraStates? init'd to 80000000
			UInt32	pad04;											// 04 - probably pad
			TESCameraState * savedCameraStates[kNumCameraStates];	// 08 - first slot always init'd to ThirdPerson2
			UInt32	unk70;											// 70 - num stored camera states?
			UInt32	pad74;											// 74 - probably pad
		};
		STATIC_ASSERT(sizeof(Data40) == 0x78);

		// for camera's collision
		struct SimpleShapePhantom
		{
			void * simpleShapePhantom1;
			void * simpleShapePhantom2;
		};

		UInt32	unk38;											// 038
		UInt32	cameraRefHandle;								// 03C - by default is Player 0x100000, will set to horse or dragon or other actor
		Data40	unk40;											// 040 - bstsmallarray
		TESCameraState * cameraStates[kNumCameraStates];		// 0B8
		SimpleShapePhantom*	simpleShapePhantoms;				// 120
		void*	rigidBody;										// 128 - bhkRigidBody
		UInt32	objectFadeHandle;								// 130 
		SimpleLock	cameraLock;									// 134
		float	worldFOV;										// 13C
		float	firstPersonFOV;									// 140
		NiPoint3	cameraPos;									// 144 - ???
		float	idleTimer;										// 150
		float	camRotZ;										// 154 - yaw?
		float	unk158;											// 158 - roll, camRotY??
		float	unk15C;											// 15C - pitch, camRotX??
		bool	allowAutoVanity;								// 160
		bool	isBowZoom;										// 161 - related to eagle eye perk
		bool	isWeapSheathed;									// 162 - init'd to 1
		bool	isProcessed;									// 163 - ???
		UInt8	unk164;											// 164 - related zero pitch handler, unk158 and unk15C
		bool	unk165;											// 165 - on vampire feed, set to 1 from 1stperson camera, related to vampire feed handler
		UInt8	pad166[2];										// 166

		bool IsCameraFirstPerson();
		bool IsCameraThirdPerson();

		inline FirstPersonState * GetFirstPersonCamera()
		{
			return (FirstPersonState*)cameraStates[kCameraState_FirstPerson];
		}

		inline ThirdPersonState * GetThirdPersonCamera()
		{
			return (ThirdPersonState*)cameraStates[kCameraState_ThirdPerson2];
		}

		TESCameraState * ProcessCameraTransition(UInt32 srcCamState, UInt32 dstCamState, bool face2face);
		void SetCameraTarget(Actor* target);
		void ForceFirstPerson();
		void ForceThirdPerson();
		void UpdateThirdPersonMode(bool weaponDrawn);
		bool GetDistanceWithTargetBone(Actor * target, NiPoint3 * dist);
		float GetDistanceWithTargetBone(Actor * target, bool firstPerson);
		
		// this function will handle the actor's fading if camera penetrates the target
		bool GetPenetration(Actor * target, NiPoint3* sourcePos, float radiusToCheck, float offsetActorFromGround);
		
		// bhkWorldM is for accessing the hkpWorld's linear cast
		// resultActor is for fading purposes, except player
		// resultInfo contains information of closest point collision
		// resultPos contains position of collision in Bethesda scale
		// resultPos is both input and output
		// this function will call linear cast, most likely sphere cast itself 
		bool GetClosestPoint(void* bhkWorldM, NiPoint3* sourcePos, NiPoint3* resultPos,
			Havok::hkpRootCdPoint* resultInfo, Actor** resultActor, float radiusToCheck);

		bool ProcessCollision(NiPoint3* camPos, bool isFade);

		bool IsInCameraView(NiPoint3* pos, int mode);

		static PlayerCamera *	GetSingleton();

	};

	STATIC_ASSERT(offsetof(PlayerCamera, cameraStates) == 0xB8);
	STATIC_ASSERT(offsetof(PlayerCamera, camRotZ) == 0x154);
	STATIC_ASSERT(offsetof(PlayerCamera, pad166) == 0x166);
	STATIC_ASSERT(sizeof(PlayerCamera) == 0x168);
}