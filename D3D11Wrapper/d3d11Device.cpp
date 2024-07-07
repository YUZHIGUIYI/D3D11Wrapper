#include "d3d11Device.h"
#include "Globals.h"
#include <string>
#include <vector>
#include <atomic>

std::atomic<uint32_t> badAtomicBufferCounter;

D3D11CustomDevice::D3D11CustomDevice(ID3D11Device* dev, ID3D11Device*** ret)
{
	m_d3dDevice = dev;
	*ret = &m_d3dDevice;
	PostInitialise();
}

D3D11CustomDevice::D3D11CustomDevice(ID3D11Device* dev)
{
	m_d3dDevice = dev;
	PostInitialise();
}

D3D11CustomDevice::D3D11CustomDevice(ID3D11Device* dev, ID3D11Device*** ret, D3DObjectManager *pGlOM)
{
	m_d3dDevice = dev;
	m_pGLOM = pGlOM;
	*ret = &m_d3dDevice;
	PostInitialise();
}

D3D11CustomDevice::D3D11CustomDevice(ID3D11Device* dev, D3DObjectManager *pGlOM)
{
	m_d3dDevice = dev;
	m_pGLOM = pGlOM;
	PostInitialise();
}


void D3D11CustomDevice::PostInitialise()
{
	
}

void D3D11CustomDevice::Notify_Present()
{
	m_pGLOM->Notify_Present();
	if (CustomContext)
	{
		CustomContext->Notify_Present();
	}
}

void D3D11CustomDevice::Link(D3D11CustomContext* devCon)
{
	CustomContext = devCon;
}

void D3D11CustomDevice::LocateSwapchain()
{
	IDXGIDevice2* pDXGIDevice;
	auto hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice2), (void**)&pDXGIDevice);

	IDXGIAdapter* pDXGIAdapter;
	hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter);

	IDXGIFactory2* pIDXGIFactory;
	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&pIDXGIFactory);
}

ID3D11Device* D3D11CustomDevice::RealDevice()
{
	return m_d3dDevice;
}

ID3D11DeviceContext* D3D11CustomDevice::RealContext()
{
	return CustomContext->m_devContext;
}

D3DObjectManager* D3D11CustomDevice::GetGLOM()
{
	return m_pGLOM;
}

HRESULT D3D11CustomDevice::CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer)
{
	bool canCaptureImmediate = false;

	switch(pDesc->Usage)
	{
	case D3D11_USAGE_DEFAULT:
		break;
	case D3D11_USAGE_IMMUTABLE:
		canCaptureImmediate = true;
		break;
	case D3D11_USAGE_DYNAMIC:
		break;
	case D3D11_USAGE_STAGING:
		break;
	default:
		break;
	}

	HRESULT result = m_d3dDevice->CreateBuffer(pDesc, pInitialData, ppBuffer);
	if (result == S_OK)
	{
		if (canCaptureImmediate)
		{
			// Do immediate copy
			m_pGLOM->AddBuffer(*ppBuffer, pInitialData->pSysMem, pDesc->ByteWidth, pDesc->BindFlags, this);
		}
		else
		{
			m_pGLOM->AddBuffer(*ppBuffer, nullptr, 0, pDesc->BindFlags, this);
		}
	}
	
	return result;
}

HRESULT D3D11CustomDevice::CreateTexture1D(const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D)
{
	auto result = m_d3dDevice->CreateTexture1D(pDesc, pInitialData, ppTexture1D);

	// Only on success
	if (result == S_OK && (pDesc->BindFlags & D3D11_BIND_SHADER_RESOURCE))
	{
		// Our texture information
		FTextureInfo fTexInfo{};

		bool canCaptureImmediate = false;
		if (pDesc->Usage == D3D11_USAGE_IMMUTABLE)
		{
			canCaptureImmediate = true;
			if (!pInitialData) 
			{ 
				DEBUG_LOGLINE(m_pGLOM->Event, LOGERR("NULLPTR in immut. Texture")); 
			}
		}

		fTexInfo.uWidth = pDesc->Width;
		fTexInfo.uFormat = pDesc->Format;
		fTexInfo.uCount = pDesc->ArraySize;

		m_pGLOM->AddTexture(*ppTexture1D, pInitialData, fTexInfo, this, canCaptureImmediate);
	}

	return result;
}

HRESULT D3D11CustomDevice::CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D)
{
	auto result = m_d3dDevice->CreateTexture2D(pDesc, pInitialData, ppTexture2D);

	// Only on success
	if (result == S_OK && (pDesc->BindFlags & D3D11_BIND_SHADER_RESOURCE))
	{
		// Our texture information
		FTextureInfo fTexInfo{};

		bool canCaptureImmediate = false;
		if (pDesc->Usage == D3D11_USAGE_IMMUTABLE)
		{
			canCaptureImmediate = true;
			if (!pInitialData) 
			{ 
				DEBUG_LOGLINE(m_pGLOM->Event, LOGERR("NULLPTR in immut. Texture")); 
			}
		}

		fTexInfo.uWidth = pDesc->Width;
		fTexInfo.uHeight = pDesc->Height;
		fTexInfo.uFormat = pDesc->Format;
		fTexInfo.uCount = pDesc->ArraySize;

		m_pGLOM->AddTexture(*ppTexture2D, pInitialData, fTexInfo, this, canCaptureImmediate);
	}

	return result;
}

HRESULT D3D11CustomDevice::CreateTexture3D(const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D)
{
	auto result = m_d3dDevice->CreateTexture3D(pDesc, pInitialData, ppTexture3D);

	// Only on success
	if (result == S_OK && pDesc->BindFlags == D3D11_BIND_SHADER_RESOURCE)
	{
		// Our texture information
		FTextureInfo fTexInfo{};

		bool canCaptureImmediate = false;
		if (pDesc->Usage == D3D11_USAGE_IMMUTABLE)
		{
			canCaptureImmediate = true;
			if (!pInitialData) { DEBUG_LOGLINE(m_pGLOM->Event, LOGERR("NULLPTR in immut. Texture")); }
		}

		fTexInfo.uWidth = pDesc->Width;
		fTexInfo.uHeight = pDesc->Height;
		fTexInfo.uDepth = pDesc->Depth;
		fTexInfo.uFormat = pDesc->Format;

		m_pGLOM->AddTexture(*ppTexture3D, pInitialData, fTexInfo, this, canCaptureImmediate);
	}

	return result;
}

HRESULT D3D11CustomDevice::CreateShaderResourceView(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRView)
{
	auto ret = m_d3dDevice->CreateShaderResourceView(pResource, pDesc, ppSRView);

	if (ret == S_OK)
	{
		// Try all
		int32_t val;
		val = m_pGLOM->QueryBuffer(pResource);
		if (val >= 0)
		{
			m_pGLOM->AddResourceView(*ppSRView, pResource, EBackingType::Buffer);
		}
		
		val = m_pGLOM->QueryTexture(pResource);
		if (val >= 0)
		{
			m_pGLOM->AddResourceView(*ppSRView, pResource, EBackingType::Texture);
		}		
	}

	return ret;
}

HRESULT D3D11CustomDevice::CreateUnorderedAccessView(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView)
{
	return m_d3dDevice->CreateUnorderedAccessView(pResource, pDesc, ppUAView);
}

HRESULT D3D11CustomDevice::CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView)
{
	return m_d3dDevice->CreateRenderTargetView(pResource, pDesc, ppRTView);
}

HRESULT D3D11CustomDevice::CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView)
{
	return m_d3dDevice->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView);
}

HRESULT D3D11CustomDevice::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements, const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength, ID3D11InputLayout** ppInputLayout)
{
	HRESULT result = m_d3dDevice->CreateInputLayout(pInputElementDescs, NumElements, pShaderBytecodeWithInputSignature, BytecodeLength, ppInputLayout);

	if (result == S_OK) 
	{
		m_pGLOM->AddInputLayout(*ppInputLayout, pInputElementDescs, NumElements); 
	}

	return result;
}

HRESULT D3D11CustomDevice::CreateVertexShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader)
{
	HRESULT result = m_d3dDevice->CreateVertexShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader);

	if (result == S_OK) 
	{ 
		m_pGLOM->AddShader(*ppVertexShader, pShaderBytecode, BytecodeLength); 
	}

	return result;
}

HRESULT D3D11CustomDevice::CreateGeometryShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader)
{
	auto ret = m_d3dDevice->CreateGeometryShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppGeometryShader);

	if (ret == S_OK) { m_pGLOM->AddShader(*ppGeometryShader, pShaderBytecode, BytecodeLength); }

	return ret;
}

HRESULT D3D11CustomDevice::CreateGeometryShaderWithStreamOutput(const void* pShaderBytecode, SIZE_T BytecodeLength, const D3D11_SO_DECLARATION_ENTRY* pSODeclaration, UINT NumEntries, const UINT* pBufferStrides, UINT NumStrides, UINT RasterizedStream, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader)
{
	auto ret = m_d3dDevice->CreateGeometryShaderWithStreamOutput(pShaderBytecode, BytecodeLength, pSODeclaration, NumEntries, pBufferStrides, NumStrides, RasterizedStream, pClassLinkage, ppGeometryShader);

	DEBUG_LOGLINE(m_pGLOM->Event, LOG("This shader needs buffer binding: StreamOutput"));

	if (ret == S_OK) { m_pGLOM->AddShader(*ppGeometryShader, pShaderBytecode, BytecodeLength); }

	return ret;
}

HRESULT D3D11CustomDevice::CreatePixelShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppPixelShader)
{
	auto ret = m_d3dDevice->CreatePixelShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppPixelShader);

	if (ret == S_OK) { m_pGLOM->AddShader(*ppPixelShader, pShaderBytecode, BytecodeLength); }

	return ret;
}

HRESULT D3D11CustomDevice::CreateHullShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11HullShader** ppHullShader)
{
	auto ret = m_d3dDevice->CreateHullShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppHullShader);

	if (ret == S_OK) { m_pGLOM->AddShader(*ppHullShader, pShaderBytecode, BytecodeLength); }

	return ret;
}

HRESULT D3D11CustomDevice::CreateDomainShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11DomainShader** ppDomainShader)
{
	auto ret = m_d3dDevice->CreateDomainShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppDomainShader);

	if (ret == S_OK) { m_pGLOM->AddShader(*ppDomainShader, pShaderBytecode, BytecodeLength); }

	return ret;
}

HRESULT D3D11CustomDevice::CreateComputeShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11ComputeShader** ppComputeShader)
{
	auto ret = m_d3dDevice->CreateComputeShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppComputeShader);

	if (ret == S_OK) { m_pGLOM->AddShader(*ppComputeShader, pShaderBytecode, BytecodeLength); }

	return ret;
}

HRESULT D3D11CustomDevice::CreateClassLinkage(ID3D11ClassLinkage** ppLinkage)
{
	return m_d3dDevice->CreateClassLinkage(ppLinkage);
}

HRESULT D3D11CustomDevice::CreateBlendState(const D3D11_BLEND_DESC* pBlendStateDesc, ID3D11BlendState** ppBlendState)
{
	return m_d3dDevice->CreateBlendState(pBlendStateDesc, ppBlendState);
}

HRESULT D3D11CustomDevice::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState)
{
	return m_d3dDevice->CreateDepthStencilState(pDepthStencilDesc, ppDepthStencilState);
}

HRESULT D3D11CustomDevice::CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState)
{
	return m_d3dDevice->CreateRasterizerState(pRasterizerDesc, ppRasterizerState);
}

HRESULT D3D11CustomDevice::CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState)
{
	return m_d3dDevice->CreateSamplerState(pSamplerDesc, ppSamplerState);
}

HRESULT D3D11CustomDevice::CreateQuery(const D3D11_QUERY_DESC* pQueryDesc, ID3D11Query** ppQuery)
{
	return m_d3dDevice->CreateQuery(pQueryDesc, ppQuery);
}

HRESULT D3D11CustomDevice::CreatePredicate(const D3D11_QUERY_DESC* pPredicateDesc, ID3D11Predicate** ppPredicate)
{
	return m_d3dDevice->CreatePredicate(pPredicateDesc, ppPredicate);
}

HRESULT D3D11CustomDevice::CreateCounter(const D3D11_COUNTER_DESC* pCounterDesc, ID3D11Counter** ppCounter)
{
	return m_d3dDevice->CreateCounter(pCounterDesc, ppCounter);
}

HRESULT D3D11CustomDevice::CreateDeferredContext(UINT ContextFlags, ID3D11DeviceContext** ppDeferredContext)
{
	return m_d3dDevice->CreateDeferredContext(ContextFlags, ppDeferredContext);
}

HRESULT D3D11CustomDevice::OpenSharedResource(HANDLE hResource, const IID& ReturnedInterface, void** ppResource)
{
	return m_d3dDevice->OpenSharedResource(hResource, ReturnedInterface, ppResource);
}

HRESULT D3D11CustomDevice::CheckFormatSupport(DXGI_FORMAT Format, UINT* pFormatSupport)
{
	return m_d3dDevice->CheckFormatSupport(Format, pFormatSupport);
}

HRESULT D3D11CustomDevice::CheckMultisampleQualityLevels(DXGI_FORMAT Format, UINT SampleCount, UINT* pNumQualityLevels)
{
	return m_d3dDevice->CheckMultisampleQualityLevels(Format, SampleCount, pNumQualityLevels);
}

void D3D11CustomDevice::CheckCounterInfo(D3D11_COUNTER_INFO* pCounterInfo)
{
	return m_d3dDevice->CheckCounterInfo(pCounterInfo);
}

HRESULT D3D11CustomDevice::CheckCounter(const D3D11_COUNTER_DESC* pDesc, D3D11_COUNTER_TYPE* pType, UINT* pActiveCounters, LPSTR szName, UINT* pNameLength, LPSTR szUnits, UINT* pUnitsLength, LPSTR szDescription, UINT* pDescriptionLength)
{
	return m_d3dDevice->CheckCounter(pDesc, pType, pActiveCounters, szName, pNameLength, szUnits, pUnitsLength, szDescription, pDescriptionLength);
}

HRESULT D3D11CustomDevice::CheckFeatureSupport(D3D11_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize)
{
	return m_d3dDevice->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
}

HRESULT D3D11CustomDevice::GetPrivateData(const GUID& guid, UINT* pDataSize, void* pData)
{
	return m_d3dDevice->GetPrivateData(guid, pDataSize, pData);
}

HRESULT D3D11CustomDevice::SetPrivateData(const GUID& guid, UINT DataSize, const void* pData)
{
	return m_d3dDevice->SetPrivateData(guid, DataSize, pData);
}

HRESULT D3D11CustomDevice::SetPrivateDataInterface(const GUID& guid, const IUnknown* pData)
{
	return m_d3dDevice->SetPrivateDataInterface(guid, pData);
}

D3D_FEATURE_LEVEL D3D11CustomDevice::GetFeatureLevel()
{
	return m_d3dDevice->GetFeatureLevel();
}

UINT D3D11CustomDevice::GetCreationFlags()
{
	return m_d3dDevice->GetCreationFlags();
}

HRESULT D3D11CustomDevice::GetDeviceRemovedReason()
{
	return m_d3dDevice->GetDeviceRemovedReason();
}

void D3D11CustomDevice::GetImmediateContext(ID3D11DeviceContext** ppImmediateContext)
{
	if (CustomContext) *ppImmediateContext = CustomContext;
	else m_d3dDevice->GetImmediateContext(ppImmediateContext);
}

HRESULT D3D11CustomDevice::SetExceptionMode(UINT RaiseFlags)
{
	return m_d3dDevice->SetExceptionMode(RaiseFlags);
}

UINT D3D11CustomDevice::GetExceptionMode()
{
	return m_d3dDevice->GetExceptionMode();
}

HRESULT D3D11CustomDevice::QueryInterface(const IID& riid, void** ppvObject)
{
	DEBUG_LOGLINE(m_pGLOM->Event, "[CF99] QI");

	return m_d3dDevice->QueryInterface(riid, ppvObject);
}


ULONG D3D11CustomDevice::AddRef()
{
	return m_d3dDevice->AddRef();
}

ULONG D3D11CustomDevice::Release()
{
	return m_d3dDevice->Release();
}

