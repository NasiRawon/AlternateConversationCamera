#pragma once

#include <d3d11.h>

namespace Tralala
{

	// actually it's part of bigger struct
	// address = g_swapChainAddr
	struct Render
	{
		//bunch of window information
		//refresh rate, vsync, borderless, etc
		//device	-10
		//context	-08
		HWND						clientHandle;		// 00
		UINT64						unk08;				// 08
		UINT						clientWidth;		// 10
		UINT						clientHeight;		// 14
		IDXGISwapChain*				swapChain;			// 18
		UINT64						unk20[2];			// 20
		ID3D11RenderTargetView*		targetView;			// 30
		ID3D11ShaderResourceView*	shaderResourceView;	// 38
		// more ..... i don't care
	};
	STATIC_ASSERT(offsetof(Render, swapChain) == 0x18);
}

namespace Graphics
{
	void GetAddresses();
	bool InstallHook();
}