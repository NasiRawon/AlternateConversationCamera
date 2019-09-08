#pragma once

#include "skse64/NiObjects.h"

namespace Havok
{

	void GetAddresses();

	__declspec(align(16)) struct hkVector4
	{
		float data[4];
	};


	// 04
	class hkpBroadPhaseHandle
	{
	public:

		UInt32	id; // 00
	};
	STATIC_ASSERT(sizeof(hkpBroadPhaseHandle) == 0x4);

	// C
	class hkpTypedBroadPhaseHandle : public hkpBroadPhaseHandle
	{
	public:

		SInt8		type;					// 04
		SInt8		ownerOffset;			// 05
		SInt8		objectQualityType;		// 06
		UInt32		collisionFilterInfo;	// 08
	};
	STATIC_ASSERT(sizeof(hkpTypedBroadPhaseHandle) == 0xC);


	// 20
	struct hkpCdBody
	{
		const void* shape;		// 00
		UInt32				shapeKey;	// 08
		const void* motion;		// 10
		const hkpCdBody* parent;		// 18 
	};

	// 70
	class hkpCollidable : public hkpCdBody
	{
	public:

		struct BoundingVolumeData
		{
			UInt32		min[3];						// 00
			UInt8		expansionMin[3];			// 0C
			UInt8		expansionShift;				// 0F
			UInt32		max[3];						// 10
			UInt8		expansionMax[3];			// 1C
			UInt16		numChildShapeAabbs;			// 20
			UInt16		capacityChildShapeAabbs;	// 22
			UInt32* childShapeAabbs;			// 28
			UInt32* childShapeKeys;				// 30
		};
		STATIC_ASSERT(offsetof(BoundingVolumeData, childShapeAabbs) == 0x28);

		SInt8						ownerOffset;				// 20
		UInt8						forceCollideOntoPpu;		// 21
		UInt16						shapeSizeOnSpu;				// 22
		hkpTypedBroadPhaseHandle	broadPhaseHandle;			// 24
		BoundingVolumeData			boundingVolumeData;			// 30
		float						allowedPenetrationDepth;	// 68

		UInt32 GetCollisionFilterInfo()
		{
			return broadPhaseHandle.collisionFilterInfo;
		}

		NiAVObject* GetNiObject();
	};
	STATIC_ASSERT(offsetof(hkpCollidable, boundingVolumeData) == 0x30);
	STATIC_ASSERT(sizeof(hkpCollidable) == 0x70);

	// struct for collision information
	struct hkpRootCdPoint
	{
		hkVector4		position;			// position of the contact/collision in Havok scale
		hkVector4		normal;				// contains distance
		hkpCollidable* hkpCollidableA;		// camera's phantom itself
		UInt32			shapeKeyA;			// shape of camera's phantom which is sphere
		hkpCollidable* hkpCollidableB;		// object that collides with the camera's phantom
		UInt32			shapeKeyB;			// shape of the object
	};
	STATIC_ASSERT(sizeof(hkpRootCdPoint) == 0x40);
}