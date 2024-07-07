#include "stdafx.h"
#include "d3d11ObjectManager.h"
#include "utils.h"
#include "d3d11Device.h"
#include "d3d11DeviceContext.h"
#include "Globals.h"
#include "../DXGIWrapper/dxgiSwapchain2.h"


#include <processenv.h>

// Global Class
D3DObjectManager *GlOM = new D3DObjectManager();


using PFN_D3D11_CREATE_DEVICE = HRESULT(WINAPI *)(__in_opt IDXGIAdapter*,
	D3D_DRIVER_TYPE, HMODULE, UINT,
	__in_ecount_opt(FeatureLevels) CONST D3D_FEATURE_LEVEL*,
	UINT FeatureLevels, UINT, __out_opt ID3D11Device**,
	__out_opt D3D_FEATURE_LEVEL*, __out_opt ID3D11DeviceContext**);


HRESULT WINAPI D3D11CreateDevice(
	__in_opt IDXGIAdapter* pAdapter,
	D3D_DRIVER_TYPE DriverType,
	HMODULE Software,
	UINT Flags,
	__in_ecount_opt(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT FeatureLevels,
	UINT SDKVersion,
	__out_opt ID3D11Device** ppDevice,
	__out_opt D3D_FEATURE_LEVEL* pFeatureLevel,
	__out_opt ID3D11DeviceContext** ppImmediateContext)
{
	// Log
	DEBUG_LOGLINE(GlOM->Event, LOG("D3D11CreateDevice intercepted"));
	PFN_D3D11_CREATE_DEVICE pfnCreateD3D11Device = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(GlOM->getDLL(), "D3D11CreateDevice");
	if (pfnCreateD3D11Device == nullptr)
	{
		DEBUG_LOGLINE(GlOM->Event, LOGERR("Cannot find function D3D11CreateDevice in DLL"));
		return S_FALSE;
	}

	// Use the real DLL's function
	HRESULT result = pfnCreateD3D11Device(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
	if (ppDevice)
	{
		DEBUG_LOGLINE(GlOM->Event, LOG("Device Created. Registering."));

		D3D11CustomDevice *customDevice = new D3D11CustomDevice(*ppDevice, GlOM);
		*ppDevice = customDevice;

		// Check if the optional immediate is being used
		if (ppImmediateContext)
		{
			DEBUG_LOGLINE(GlOM->Event, LOG("Caught Immediate Context"));
			ID3D11DeviceContext* customDeviceCtx = new D3D11CustomContext(*ppImmediateContext, customDevice, GlOM);
			*ppImmediateContext = customDeviceCtx;
		}
	} else
	{
		DEBUG_LOGLINE(GlOM->Event, LOGWARN("Failed to get device from D3D11"));
	}

	return result;
}

using PFN_D3D11ON12_CREATE_DEVICE = HRESULT(WINAPI *)(_In_ IUnknown*, UINT,
	_In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL*, UINT FeatureLevels,
	_In_reads_opt_(NumQueues) IUnknown* CONST*, UINT NumQueues,
	UINT, _COM_Outptr_opt_ ID3D11Device**, _COM_Outptr_opt_ ID3D11DeviceContext**,
	_Out_opt_ D3D_FEATURE_LEVEL*);

HRESULT WINAPI D3D11On12CreateDevice(
	_In_ IUnknown* pDevice,
	UINT Flags,
	_In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT FeatureLevels,
	_In_reads_opt_(NumQueues) IUnknown* CONST* ppCommandQueues,
	UINT NumQueues,
	UINT NodeMask,
	_COM_Outptr_opt_ ID3D11Device** ppDevice,
	_COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext,
	_Out_opt_ D3D_FEATURE_LEVEL* pChosenFeatureLevel)
{
	DEBUG_LOGLINE(GlOM->Event, LOG("Intercepted D3D11On12CreateDevice call"));
	PFN_D3D11ON12_CREATE_DEVICE pfnCreateD3D11On12Device = (PFN_D3D11ON12_CREATE_DEVICE)GetProcAddress(GlOM->getDLL(), "D3D11On12CreateDevice");
	if (!pfnCreateD3D11On12Device)
	{
		DEBUG_LOGLINE(GlOM->Event, LOGERR("Cannot find function D3D11On12CreateDevice in DLL"));
		return S_FALSE;
	}

	HRESULT result = pfnCreateD3D11On12Device(pDevice, Flags, pFeatureLevels, FeatureLevels, ppCommandQueues, NumQueues, NodeMask, ppDevice, ppImmediateContext, pChosenFeatureLevel);
	if (ppDevice == nullptr)
	{
		DEBUG_LOGLINE(GlOM->Event, LOGWARN("Failed to get device from D3D11"));
	}
	return result;
}

HRESULT WINAPI D3D11CreateDeviceForD3D12() { return NULL; }

using PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN =  HRESULT(WINAPI *)(__in_opt IDXGIAdapter*,
	D3D_DRIVER_TYPE, HMODULE, UINT,
	__in_ecount_opt(FeatureLevels) CONST D3D_FEATURE_LEVEL*,
	UINT FeatureLevels, UINT, __in_opt CONST DXGI_SWAP_CHAIN_DESC*,
	__out_opt IDXGISwapChain**, __out_opt ID3D11Device**,
	__out_opt D3D_FEATURE_LEVEL*, __out_opt ID3D11DeviceContext**);

HRESULT WINAPI D3D11CreateDeviceAndSwapChain(
	__in_opt IDXGIAdapter* pAdapter,
	D3D_DRIVER_TYPE DriverType,
	HMODULE Software,
	UINT Flags,
	__in_ecount_opt(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT FeatureLevels,
	UINT SDKVersion,
	__in_opt CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
	__out_opt IDXGISwapChain** ppSwapChain,
	__out_opt ID3D11Device** ppDevice,
	__out_opt D3D_FEATURE_LEVEL* pFeatureLevel,
	__out_opt ID3D11DeviceContext** ppImmediateContext)
{
	DEBUG_LOGLINE(GlOM->Event, LOG("D3D11CreateDeviceAndSwapChain intercepted"));
	PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN pfnCreateDeviceAndSwapChain = 
											(PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN)GetProcAddress(GlOM->getDLL(), "D3D11CreateDeviceAndSwapChain");
	if (!pfnCreateDeviceAndSwapChain)
	{
		return S_FALSE;
	}

	HRESULT result = pfnCreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion,pSwapChainDesc,ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
	if (result == S_OK)
	{
		DEBUG_LOGLINE(GlOM->Event, LOG("CDSC Success"));

		if (ppDevice)
		{
			DEBUG_LOGLINE(GlOM->Event, LOG("CDSC Has Device"));
			D3D11CustomDevice *customDevice = new D3D11CustomDevice(*ppDevice, GlOM);
			*ppDevice = customDevice;

			if (ppImmediateContext)
			{
				DEBUG_LOGLINE(GlOM->Event, LOG("CDSC Device + IM"));
				ID3D11DeviceContext* constomDeviceCtx = new D3D11CustomContext(*ppImmediateContext, customDevice, GlOM);
				*ppImmediateContext = constomDeviceCtx;
			}
			if (ppSwapChain)
			{
				DEBUG_LOGLINE(GlOM->Event, LOG("CDSC Device + SC"));
				IDXGISwapChain *customSwapChain = new DXGICustomSwapChain(*ppSwapChain, customDevice, GlOM);
				*ppSwapChain = customSwapChain;
			}
		}
		else
		{
			if (ppImmediateContext)
			{
				DEBUG_LOGLINE(GlOM->Event, LOG("CDSC IM"));
				ID3D11DeviceContext* customDeviceCtx = new D3D11CustomContext(*ppImmediateContext, GlOM);
				*ppImmediateContext = customDeviceCtx;
			}
			if (ppSwapChain)
			{
				DEBUG_LOGLINE(GlOM->Event, LOG("CDSC SC"));
				IDXGISwapChain *customSwapChain = new DXGICustomSwapChain(*ppSwapChain, nullptr, GlOM);
				*ppSwapChain = customSwapChain;
			}
		}
	}
	return result;
}

using DXGIFAC = HRESULT(WINAPI *)(REFIID, void **);

HRESULT WINAPI CreateDXGIFactory(REFIID riid, void **ppFactory)
{
	DXGIFAC pfnCreateDXGIFactory = (DXGIFAC)GetProcAddress(GlOM->getDLL(), "CreateDXGIFactory");
	if (!pfnCreateDXGIFactory)
	{
		return S_FALSE;
	}

	HRESULT result = pfnCreateDXGIFactory(riid, ppFactory);

	return result;
}

HRESULT WINAPI CreateDXGIFactory1(REFIID riid, void **ppFactory)
{
	DEBUG_LOGLINE(GlOM->Event, LOG("CreateDXGIFactory1 intercepted"));
	DXGIFAC pfnCreateDXGIFactory = (DXGIFAC)GetProcAddress(GlOM->getDLL(), "CreateDXGIFactory1");
	if (!pfnCreateDXGIFactory)
	{
		return S_FALSE;
	}

	HRESULT result = pfnCreateDXGIFactory(riid, ppFactory);

	return result;
}

HRESULT WINAPI CreateDXGIFactory2(REFIID riid, void **ppFactory)
{
	DEBUG_LOGLINE(GlOM->Event, LOG("CreateDXGIFactory2 intercepted"));
	DXGIFAC pfnCreateDXGIFactory = (DXGIFAC)GetProcAddress(GlOM->getDLL(), "CreateDXGIFactory2");
	if (!pfnCreateDXGIFactory)
	{
		return S_FALSE;
	}

	HRESULT result = pfnCreateDXGIFactory(riid, ppFactory);

	return result;
}

int WINAPI D3DPerformance_BeginEvent(DWORD col, LPCWSTR wszName)
{
	return 0;
}

int WINAPI D3DPerformance_EndEvent()
{
	return 0;
}

DWORD WINAPI D3DPerformance_GetStatus()
{
	return 0;
}

void WINAPI D3DPerformance_SetMarker()
{
	
}
