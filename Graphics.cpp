#include "Settings.h"
#include "Utils.h"
#include "PatternScanner.h"
#include "Graphics.h"
#include "Shader.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

uintptr_t g_unkStructAddr = 0;
uintptr_t g_presentAddr = 0;
uintptr_t g_swapChainAddr = 0;
uintptr_t g_menuRenderAddr = 0;
uintptr_t g_cleanAddr = 0;

namespace Tralala
{
	BOOL g_bInitialised = false;
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pContext = nullptr;
	ID3D11RenderTargetView* pRenderTargetView = nullptr;
	ID3D11VertexShader* pVertexShader = nullptr;
	ID3D11InputLayout* pVertexLayout = nullptr;
	ID3D11PixelShader* pPixelShader = nullptr;
	ID3D11Buffer* pVertexBuffer = nullptr;
	ID3D11Buffer* pIndexBuffer = nullptr;
	ID3D11Buffer* pConstantBuffer = nullptr;
	ID3D10Blob* pBlob = nullptr;

	D3D11_VIEWPORT pViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{ 0 };

	struct ConstantBuffer
	{
		DirectX::XMFLOAT2A pos;
	};

	struct Vertex
	{
		DirectX::XMFLOAT2A pos;
	};

	void CleanupD3D11()
	{
		if (!Settings::bLetterBox)
			return;

		// Cleanup the directx stuffs
		_MESSAGE("Cleaning up Direct3D stuff..");

		if (pDevice && !pDevice->Release())
			_MESSAGE("Device has been deleted.");

		if (pContext && !pContext->Release())
			_MESSAGE("Context has been deleted.");

		if (pRenderTargetView && !pRenderTargetView->Release())
			_MESSAGE("TargetView has been deleted.");

		if (pVertexShader && !pVertexShader->Release())
			_MESSAGE("VertexShader has been deleted.");

		if (pPixelShader && !pPixelShader->Release())
			_MESSAGE("PixelShader has been deleted.");

		if (pVertexLayout && !pVertexLayout->Release())
			_MESSAGE("VertexLayout has been deleted.");

		if (pVertexBuffer && !pVertexBuffer->Release())
			_MESSAGE("VertexBuffer has been deleted.");

		if (pIndexBuffer && !pIndexBuffer->Release())
			_MESSAGE("IndexBuffer has been deleted.");

		if (pConstantBuffer && !pConstantBuffer->Release())
			_MESSAGE("ConstantBuffer has been deleted.");

		if (pBlob && !pBlob->Release())
			_MESSAGE("Blob has been deleted.");

		_MESSAGE("Done.");
	}

	static HRESULT GetDeviceAndCtxFromSwapchain(IDXGISwapChain* pSwapChain, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
	{
		HRESULT ret = pSwapChain->GetDevice(__uuidof(ID3D11Device), (PVOID*)ppDevice);

		if (SUCCEEDED(ret))
			(*ppDevice)->GetImmediateContext(ppContext);
		else
			MessageBoxA(NULL, "Failed to get ID3D11Device", APP_NAME, MB_OK | MB_ICONERROR);

		return ret;
	}

	static bool CompileShader(const char* szShader, const char* szEntrypoint, const char* szTarget, ID3D10Blob** pBlob)
	{
		ID3D10Blob* pErrorBlob = nullptr;

		auto hr = D3DCompile(szShader, strlen(szShader), 0, nullptr, nullptr, szEntrypoint, szTarget, D3DCOMPILE_ENABLE_STRICTNESS, 0, pBlob, &pErrorBlob);
		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				char szError[256]{ 0 };
				memcpy(szError, pErrorBlob->GetBufferPointer(), pErrorBlob->GetBufferSize());
				MessageBoxA(nullptr, szError, "Error", MB_OK);
				pErrorBlob->Release();
			}
			return false;
		}
		return true;
	}

#if 1
	static void DrawLetterBox(ID3D11DeviceContext* context, UINT numVertices, float dstPosY)
	{
		// Make sure our render target is set.
		context->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

		// Update view
		ConstantBuffer cb;
		cb.pos = { 0.0f, dstPosY };
		context->UpdateSubresource(pConstantBuffer, 0, nullptr, &cb, 0, 0);
		context->VSSetConstantBuffers(0, 1, &pConstantBuffer);

		// Make sure the input assembler knows how to process our verts/indices
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
		context->IASetInputLayout(pVertexLayout);
		context->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set the shaders we need to render our triangle
		context->VSSetShader(pVertexShader, nullptr, 0);
		context->PSSetShader(pPixelShader, nullptr, 0);

		// Set viewport to context
		context->RSSetViewports(1, pViewports);

		// Draw our triangle
		context->DrawIndexed(numVertices, 0, 0);
	}


	void* PresentHook(IDXGISwapChain* pSwapChain)
	{
		if (!Settings::bLetterBox)
			return *(void**)g_unkStructAddr;

#if 1
		static float srcHeight = 0.0f;
		static float dstHeight = 0.0f;
		static float speed = 0.005f;
#endif

		if (!g_bInitialised) {
			if (FAILED(GetDeviceAndCtxFromSwapchain(pSwapChain, &pDevice, &pContext)))
			{
				CleanupD3D11();
				return *(void**)g_unkStructAddr;
			}

			pContext->OMGetRenderTargets(1, &pRenderTargetView, nullptr);

			// create vertex shader
			if (!CompileShader(szShadez, "VS", "vs_5_0", &pBlob))
			{
				CleanupD3D11();
				return *(void**)g_unkStructAddr;
			}

			if (FAILED(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader)))
			{
				CleanupD3D11();
				return *(void**)g_unkStructAddr;
			}

			// Define/create the input layout for the vertex shader
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};
			UINT numElements = ARRAYSIZE(layout);

			if (FAILED(pDevice->CreateInputLayout(layout, numElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pVertexLayout)))
			{
				CleanupD3D11();
				return *(void**)g_unkStructAddr;
			}

			// create pixel shader
			if (!CompileShader(szShadez, "PS", "ps_5_0", &pBlob))
			{
				CleanupD3D11();
				return *(void**)g_unkStructAddr;
			}

			if (FAILED(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader)))
			{
				CleanupD3D11();
				return *(void**)g_unkStructAddr;
			}

			// Get viewport
			UINT numViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
			pContext->RSGetViewports(&numViewports, pViewports);

			// Just in case
			if (!numViewports || !pViewports[0].Width)
			{
				// Setup viewport
				pViewports[0].Width = *(float*)Tralala::g_iSizeWDisplayAddr;
				pViewports[0].Height = *(float*)Tralala::g_iSizeHDisplayAddr;
				pViewports[0].MinDepth = 0.0f;
				pViewports[0].MaxDepth = 1.0f;

				// Set viewport to context
				pContext->RSSetViewports(1, pViewports);
			}

			// Create a triangle to render
			// Height of triangle
			float posY = 1.0f - ((float)Settings::uLetterBoxThickness / 1000.0f);
			dstHeight = 1.0f - posY;

			// Create constant buffer
			ConstantBuffer cb;
			cb.pos = { 0.0f, srcHeight };

			D3D11_BUFFER_DESC bd{ 0 };
			D3D11_SUBRESOURCE_DATA sr{ 0 };

			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(cb);

			// create the buffer.
			sr.pSysMem = &cb;
			if (FAILED(pDevice->CreateBuffer(&bd, &sr, &pConstantBuffer)))
			{
				CleanupD3D11();
				return *(void**)g_unkStructAddr;
			}

			Vertex pVerts[] =
			{
				{ DirectX::XMFLOAT2A(-1.0f,	1.1f + dstHeight) }, // 0
				{ DirectX::XMFLOAT2A(1.0f,	1.1f + dstHeight) }, // 1
				{ DirectX::XMFLOAT2A(1.0f,	1.0f) }, // 2
				{ DirectX::XMFLOAT2A(-1.0f,	1.0f) }, // 3
				{ DirectX::XMFLOAT2A(-1.0f,	-1.0f) }, // 4
				{ DirectX::XMFLOAT2A(1.0f,	-1.0f) }, // 5
				{ DirectX::XMFLOAT2A(1.0f,	-1.1f - dstHeight) }, // 6
				{ DirectX::XMFLOAT2A(-1.0f,	-1.1f - dstHeight) }  // 7
			};

			// Create a vertex buffer, start by setting up a description.
			ZeroMemory(&bd, sizeof(bd));
			ZeroMemory(&sr, sizeof(sr));

			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(pVerts);
			bd.StructureByteStride = sizeof(pVerts[0]);

			// create the buffer.
			sr.pSysMem = &pVerts;
			if (FAILED(pDevice->CreateBuffer(&bd, &sr, &pVertexBuffer)))
			{
				CleanupD3D11();
				return *(void**)g_unkStructAddr;
			}

			// Create an index buffer
			ZeroMemory(&bd, sizeof(bd));
			ZeroMemory(&sr, sizeof(sr));

			UINT pIndices[] =
			{
				0, 1, 2,
				0, 2, 3,
				4, 5, 6,
				4, 6, 7
			};
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(pIndices); // size of buffer
			bd.StructureByteStride = sizeof(pIndices[0]); // size of element

			sr.pSysMem = &pIndices;
			if (FAILED(pDevice->CreateBuffer(&bd, &sr, &pIndexBuffer)))
			{
				CleanupD3D11();
				return *(void**)g_unkStructAddr;
			}

			speed = Settings::uLetterBoxSpeed / 1000.0f;

			g_bInitialised = true;
		}
		else
		{
			MenuTopicManager* mtm = MenuTopicManager::GetSingleton();
			if (!mtm)
				return *(void**)g_unkStructAddr;

			float step = (speed / 0.016667f) * (*(float*)g_deltaTimeAddr);

			bool isDialogueMenuClose = *(bool*)g_isDialogueMenuCloseAddr;

			UInt32 containerHandle = *(UInt32*)g_containerHandle;
			UInt32 barterHandle = *(UInt32*)g_barterHandle;
			UInt32 activeHandle = containerHandle;
			if (barterHandle)
				activeHandle = barterHandle;

			if (mtm->isInDialogueState && !isDialogueMenuClose && !activeHandle && !g_isTrainingMenu)
			{
				DrawLetterBox(pContext, 12, srcHeight);

				if (srcHeight < dstHeight)
					srcHeight += step;
				else
					srcHeight = dstHeight;
			}
			else
			{
				if (srcHeight > 0 && srcHeight <= dstHeight)
				{
					DrawLetterBox(pContext, 12, srcHeight);

					srcHeight -= step;
				}
				else
				{
					srcHeight = 0.0f;
				}
			}
		}
		return *(void**)g_unkStructAddr;
	}
#endif
}

#include "skse64_common/Utilities.h"
#include "skse64_common/Relocation.h"
#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/SafeWrite.h"
#include "xbyak/xbyak.h"

namespace Graphics
{
	void GetAddresses()
	{
		const std::array<BYTE, 6> unkpattern = { 0x4C, 0x8B, 0x09, 0x8B, 0x50, 0x30 };
		g_unkStructAddr = (uintptr_t)scan_memory_data(unkpattern, 0x7, false, 0x3, 0x7);

		g_presentAddr = (uintptr_t)scan_memory(unkpattern, 0x7, false);

		const std::array<BYTE, 5> cleanPattern = { 0x41, 0x8B, 0xDE, 0x8B, 0xFD };
		g_cleanAddr = (uintptr_t)scan_memory(cleanPattern, 0x6D, false);

#if 0
		g_swapChainAddr = (uintptr_t)scan_memory_data(unkpattern, 0x15, false, 0x3, 0x7);

		const std::array<BYTE, 7> menupattern = { 0x4C, 0x8B, 0xE9, 0x33, 0xC0, 0x8B, 0xF0 };
		g_menuRenderAddr = (uintptr_t)scan_memory(menupattern, 0x7, true);
#endif
	}

	bool InstallHook()
	{
		{
			struct InstallHookPresent_Code : Xbyak::CodeGenerator {
				InstallHookPresent_Code(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;

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

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_presentAddr + 0x5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookPresent_Code code(codeBuf, GetFnAddr(Tralala::PresentHook));
			g_localTrampoline.EndAlloc(code.getCurr());

			if (!g_branchTrampoline.Write5Branch(g_presentAddr, uintptr_t(code.getCode())))
				return false;

			SafeWrite16(g_presentAddr + 0x5, NOP16);
		}

		{
			struct InstallHookDtor_Code : Xbyak::CodeGenerator {
				InstallHookDtor_Code(void* buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;

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

					mov(qword[rsp + 0x20], rsi);

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_cleanAddr + 0x5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookDtor_Code code(codeBuf, GetFnAddr(Tralala::CleanupD3D11));
			g_localTrampoline.EndAlloc(code.getCurr());

			if (!g_branchTrampoline.Write5Branch(g_cleanAddr, uintptr_t(code.getCode())))
				return false;
		}

		return true;
	}
}

