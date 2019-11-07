#pragma once

#include "skse64/ScaleformValue.h"

namespace Tralala
{

	class GFxValue;

	void ScaleformUtilGetAddresses();

	class ScaleformHeap
	{
	public:
		virtual void	Fn_00(void);
		virtual void	Fn_01(void);
		virtual void	Fn_02(void);
		virtual void	Fn_03(void);
		virtual void	Fn_04(void);
		virtual void	Fn_05(void);
		virtual void	Fn_06(void);
		virtual void	Fn_07(void);
		virtual void	Fn_08(void);
		virtual void	Fn_09(void);
		virtual void* Allocate(size_t size, UInt32 unk = 0);	// unk is probably align, maybe flags (haven't traced)
		virtual void	Fn_0B(void);
		virtual void	Free(void* ptr);

		static ScaleformHeap* GetSingleton();
	};

	void* ScaleformHeap_Allocate(UInt32 size);
	void ScaleformHeap_Free(void* ptr);

	class GRefCountImplCore
	{
	public:
		GRefCountImplCore() :refCount(1) { }
		virtual ~GRefCountImplCore() { }

		volatile SInt32	refCount;	// 08

		// redirect new/delete to the scaleform heap
		static void* operator new(std::size_t size)
		{
			return ScaleformHeap_Allocate(size);
		}

		static void* operator new(std::size_t size, const std::nothrow_t&)
		{
			return ScaleformHeap_Allocate(size);
		}

		// placement new
		static void* operator new(std::size_t size, void* ptr)
		{
			return ptr;
		}

		static void operator delete(void* ptr)
		{
			ScaleformHeap_Free(ptr);
		}

		// placement delete
		static void operator delete(void*, void*)
		{
			//
		}

		void	AddRef(void)
		{
			InterlockedIncrement(&refCount);
		}

		void	Release(void)
		{
			if (InterlockedDecrement(&refCount) == 0)
			{
				delete this;
			}
		}
	};

	class GRefCountImpl : public GRefCountImplCore
	{
	public:
		GRefCountImpl() { }
		virtual ~GRefCountImpl() { }
	};

	class GRefCountBaseStatImpl : public GRefCountImpl
	{
	public:
		GRefCountBaseStatImpl() { }
		virtual ~GRefCountBaseStatImpl() { }
	};

	class GRefCountBase : public GRefCountBaseStatImpl
	{
	public:
		GRefCountBase() { }
		virtual ~GRefCountBase() { }
	};

	class GFxMovieView : public GRefCountBase
	{
	public:
		GFxMovieView();
		virtual ~GFxMovieView();

		virtual void* GetMovieDef();
		virtual UInt32	Unk_02(void);
		virtual UInt32	Unk_03(void);
		virtual void	GotoFrame(UInt32 idx);
		virtual bool	GotoLabeledFrame(const char* label, UInt32 unk);
		virtual void	SetPause(UInt32 pause);
		virtual UInt32	GetPause(void);
		virtual void	SetVisible(bool visible);
		virtual bool	GetVisible();
		virtual bool	IsPathAvailable(const char* path);
		virtual void	CreateString(GFxValue* value, const char* str);
		virtual void	CreateWideString(GFxValue* value, const wchar_t* str);
		virtual void	CreateObject(GFxValue* value, const char* className = NULL, GFxValue* args = NULL, UInt32 numArgs = 0);
		virtual void	CreateArray(GFxValue* value);
		virtual void	CreateFunction(GFxValue* value, void* callback, void* refcon = NULL);
		virtual bool	SetVariable(const char* name, GFxValue* value, UInt32 flags);
		virtual bool	GetVariable(GFxValue* value, const char* name);
		virtual bool	SetArray(UInt32 type, const char* name, UInt32 offset, void* buf, UInt32 len, UInt32 flags);
		virtual bool	ResizeArray(const char* name, UInt32 len, UInt32 flags);
		virtual UInt32	GetArrayLen(const char* name);	// 050
		virtual bool	GetArray(UInt32 type, const char* name, UInt32 offset, void* buf, UInt32 len);
		virtual bool	Invoke(const char* name, GFxValue* result, GFxValue* args, UInt32 numArgs);
		virtual bool	Invoke(const char* name, GFxValue* result, const char* fmt, ...);
		virtual bool	Invoke_v(const char* name, GFxValue* result, const char* fmt, va_list args);	// 060
		virtual void	SetViewport(const GViewport& viewDesc);
		virtual void	GetViewport(GViewport* pviewDesc) const;
		virtual void	Unk_1B(UInt32 unk);  // SE: 0D8 (Skyrim32: 06C)
		virtual void	Unk_1C(void);	// 070
		virtual void	Unk_1D(void);
		virtual void	Unk_1E(void);
		virtual GRectF	GetVisibleFrameRect(void);
		virtual void	Unk_20(void);	// 080
		virtual void	Unk_21(void);
		virtual void	Unk_22(void);
		virtual void	Unk_23(void);
		virtual void	Unk_24(void);	// 090
		virtual void	Unk_25(void);
		virtual void	Render(void);
		virtual void	Unk_27(void);
		virtual void	Unk_28(void);	// 0A0
		virtual void	Unk_29(void);
		virtual void	Unk_2A(void);
		virtual void	Unk_2B(void);
		virtual void	Unk_2C(void);	// 0B0
		virtual UInt32	HandleEvent(void* data);
		virtual void	Unk_2E(void);
		virtual void	Unk_2F(void);
		virtual void	Unk_30(void);
		virtual void	Unk_31(void);
		virtual void	Unk_32(void);
		virtual void	Unk_33(void);
		virtual void	Unk_34(void);
		virtual void	Unk_35(void);
		virtual void	Unk_36(void);
		virtual void	Unk_37(void);
		virtual void	Unk_38(void);
		virtual void	Unk_39(void);
		virtual void	Unk_3A(void);
		virtual void	Unk_3B(void);
		virtual void	Unk_3C(void);
		virtual void	Unk_3D(void);
		virtual void	Unk_3E(void);
		virtual void	Unk_3F(void);
		virtual void	Unk_40(void);
		virtual void	Unk_41(void);
		virtual void	Unk_42(void);
		virtual void	Unk_43(void);
		virtual void	Unk_44(void);
		virtual void	ForceCollectGarbage(void);
		// more
	};

	// 10
	class GFxValue
	{
	public:
		GFxValue() :
			objectInterface(NULL), type(kType_Undefined) { }
		GFxValue(double value) : 
			objectInterface(NULL), type(kType_Number) { data.number = value; }
		GFxValue(const GFxValue& src) :
			objectInterface(NULL), type(src.type)
		{
			data = src.data;
			if (src.IsManaged()) AddManaged(src);
		}
		~GFxValue();

		const GFxValue& operator = (const GFxValue& src)
		{
			if (this != &src)
			{
				if (IsManaged()) CleanManaged();
				type = src.type;
				data = src.data;
				if (src.IsManaged()) AddManaged(src);
			}
			return *this;
		}

		enum
		{
			kType_Undefined = 0,
			kType_Null,
			kType_Bool,
			kType_Number,
			kType_String,
			kType_WideString,
			kType_Object,
			kType_Array,
			kType_DisplayObject,

			kTypeFlag_Managed = 1 << 6,

			kMask_Type = 0x8F,	// not sure why it checks the top bit
		};

		union Data
		{
			double			number;
			bool			boolean;
			const char* string;
			const char** managedString;
			const wchar_t* wideString;
			const wchar_t** managedWideString;
			void* obj;
		};

		//#pragma pack (push, 8)
		class DisplayInfo
		{
		public:
			DisplayInfo() : _varsSet(0) {}
			enum
			{
				kChange_x = (1 << 0),
				kChange_y = (1 << 1),
				kChange_rotation = (1 << 2),
				kChange_xscale = (1 << 3),
				kChange_yscale = (1 << 4),
				kChange_alpha = (1 << 5),
				kChange_visible = (1 << 6),
				kChange_z = (1 << 7),
				kChange_xrotation = (1 << 8),
				kChange_yrotation = (1 << 9),
				kChange_zscale = (1 << 10),
				kChange_FOV = (1 << 11),
				kChange_projMatrix3D = (1 << 12),
				kChange_viewMatrix3D = (1 << 13)
			};

			double		_x;
			double		_y;
			double		_rotation;
			double		_xScale;
			double		_yScale;
			double		_alpha;
			bool		_visible;
			double		_z;
			double		_xRotation;
			double		_yRotation;
			double		_zScale;
			double		_perspFOV;
			GMatrix3D	_viewMatrix3D;
			GMatrix3D	_perspectiveMatrix3D;
			UInt16		_varsSet;

			void SetX(double x) { SetFlags(kChange_x); _x = x; }
			void SetY(double y) { SetFlags(kChange_y); _y = y; }
			void SetRotation(double degrees) { SetFlags(kChange_rotation); _rotation = degrees; }
			void SetXScale(double xscale) { SetFlags(kChange_xscale); _xScale = xscale; }
			void SetYScale(double yscale) { SetFlags(kChange_yscale); _yScale = yscale; }
			void SetAlpha(double alpha) { SetFlags(kChange_alpha); _alpha = alpha; }
			void SetVisible(bool visible) { SetFlags(kChange_visible); _visible = visible; }
			void SetZ(double z) { SetFlags(kChange_z); _z = z; }
			void SetXRotation(double degrees) { SetFlags(kChange_xrotation); _xRotation = degrees; }
			void SetYRotation(double degrees) { SetFlags(kChange_yrotation); _yRotation = degrees; }
			void SetZScale(double zscale) { SetFlags(kChange_zscale); _zScale = zscale; }
			void SetFOV(double fov) { SetFlags(kChange_FOV); _perspFOV = fov; }
			void SetProjectionMatrix3D(const GMatrix3D* pmat)
			{
				if (pmat) {
					SetFlags(kChange_projMatrix3D);
					_perspectiveMatrix3D = *pmat;
				}
				else
					ClearFlags(kChange_projMatrix3D);
			}
			void SetViewMatrix3D(const GMatrix3D* pmat)
			{
				if (pmat) {
					SetFlags(kChange_viewMatrix3D);
					_viewMatrix3D = *pmat;
				}
				else
					ClearFlags(kChange_viewMatrix3D);
			}

			// Convenience functions
			void SetPosition(double x, double y) { SetFlags(kChange_x | kChange_y); _x = x; _y = y; }
			void SetScale(double xscale, double yscale) { SetFlags(kChange_xscale | kChange_yscale); _xScale = xscale; _yScale = yscale; }

			void SetFlags(UInt32 flags) { _varsSet |= flags; }
			void ClearFlags(UInt32 flags) { _varsSet &= ~flags; }
		};
		//#pragma pack (pop)

		// 8
		class ObjectInterface
		{
		public:
			GFxMovieRoot* root;
		};

		ObjectInterface* objectInterface;	// 00
		UInt32			type;				// 08
		Data			data;				// 10

		UInt32	GetType(void) const { return type & kMask_Type; }
		bool	IsDisplayObject(void) const { return GetType() == kType_DisplayObject; }
		bool	IsManaged(void) const { return (type & kTypeFlag_Managed) != 0; }
		void	CleanManaged(void);
		void	AddManaged(void);
		void	AddManaged(const GFxValue& src);
		bool	GetDisplayInfo(DisplayInfo* displayInfo);
		bool	SetDisplayInfo(DisplayInfo* displayInfo);
		bool	GetMember(const char* name, GFxValue* value);
		bool	SetMember(const char* name, GFxValue* value);
		bool	SetText(const char* text, bool html);
		void	SetString(const char* value);
		void	SetNumber(double value);
		const char* GetString(void) const;
	};

}