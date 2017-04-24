#include "stdafx.h"
#include "DXManager.h"


IDXGIAdapter* CDXManager::ChooseAdapter(HWND hwnd) {
	IDXGIAdapter* pAdapter = NULL; //Adapatador
	IDXGIFactory* pFactory = NULL; //El menumerador de dispositivos.
	CreateDXGIFactory(IID_IDXGIFactory, (void**)&pFactory);
	int iAdapter = 0;
	while (true) 
	{
		pFactory->EnumAdapters(iAdapter, &pAdapter);
		if (!pAdapter)
			return NULL;
		DXGI_ADAPTER_DESC dad;
		pAdapter->GetDesc(&dad);
		wchar_t szMessage[1024];
		wsprintf(szMessage, L"¿Desea utilizar éste adaptador:\r\nDescripción: %s\r\n", dad.Description);
		switch (MessageBox(hwnd, szMessage, L"Seleccione dispositivo", MB_ICONQUESTION|MB_YESNOCANCEL))
		{
		case IDYES:
			pFactory->Release();
			return pAdapter;
		case IDNO:
			pAdapter->Release();
			iAdapter++;
			break;
		case IDCANCEL:
			pAdapter->Release();
			pFactory->Release();
			return NULL;
		}
	}
	pFactory->Release();
	return NULL;
}

bool CDXManager::Initialize(IDXGIAdapter * pAdapter, HWND hWnd, bool bUseCPU)
{
	DXGI_SWAP_CHAIN_DESC dscd;
	memset(&dscd, 0, sizeof(dscd)); //INICIALIZACIÓN DE BYTES.
	dscd.BufferCount = 2;
	dscd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	RECT rc;
	GetClientRect(hWnd, &rc);
	dscd.BufferDesc.Width = rc.right;
	dscd.BufferDesc.Height = rc.bottom;
	dscd.BufferDesc.RefreshRate.Numerator = 0; // 0 para auto-refresh.
	dscd.BufferDesc.RefreshRate.Denominator = 0;
	dscd.OutputWindow = hWnd;
	dscd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //Sincronizable verticalmente, es el más rápido y no guarda el front-buffer.
	dscd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS; // Para Direct3D para renderizar en 3D | Para salida como DirectCompute;
	dscd.SampleDesc.Count = 1; // Equivale a un pixel de salida y entrada.
	dscd.SampleDesc.Quality = 0; // En DirectCompute se procesa unidad por unidad. Filtro Gaussiano, mejor calidad.
	/* Media, Cubica, Gaussania. */
	dscd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED; // Si la salida es más pequeña que la entrada, la entrada se encoge.
	// Líneas de rastreo:
	// 1080i tiene ventaja sobre 1080p en el ancho de banda.
	// Son dos transferencias. Se usan líneas pares y líneas nones.
	// Interlive agrega un 1/60 de segundo, es deicr, de latencia.
	dscd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE; // Progresivo lanza todo el frame en una sola pieza.
	//Protocolo de comunicación: IIC ó (I^2)C en cables HDMI que puede servir para controlar otros dispositivos.
	dscd.Windowed = true; // Si es false, se manda a pantalla completa.
	D3D_FEATURE_LEVEL Requested[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 }, Detected;
	if (bUseCPU) {
		// WARP (Windows Advance Raterization Platform) interpreta a codigo ensamblado en tiempo real.
		HRESULT hr = 
			D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, 0, Requested, 1, D3D11_SDK_VERSION, &dscd, &m_pSwapChain, &m_pDevice, &Detected, &m_pContext);
		if (FAILED(hr)) return false;
	}
	else {
		HRESULT hr =
			D3D11CreateDeviceAndSwapChain(pAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, Requested, 1, D3D11_SDK_VERSION, &dscd, &m_pSwapChain, &m_pDevice, &Detected, &m_pContext);
		if (FAILED(hr)) return false;
	}
	return true;
}


ID3D11ComputeShader * CDXManager::Compile(wchar_t * pszFileName, char * pszEntryPoint)
{
	// HLSL -> DXIL
	ID3DBlob* pCode = 0; //Blob = Binary Large Object  "Byte Arrray"
	ID3DBlob* pErrors = 0; // ASCII De los errore sde compilación \0
	D3DCompileFromFile(pszFileName, NULL, 
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // Es un decrypter, es para proteger archivos Shaders
		pszEntryPoint, 
		"cs_5_0", // Se debe escribir tal cual.
// Compilación condicional		
#ifndef _DEBUG
		// Activar warnings u otras opciones de compilación.
		D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ENABLE_STRICTNESS,
#else
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
#endif
		0, 
		&pCode, &pErrors);
	if (pErrors) // Es distinto de NULL cuando hay errores o warnings.
	{
		MessageBoxA(NULL, (char*)pErrors->GetBufferPointer(), "Errores y avisos", MB_ICONEXCLAMATION);
		pErrors->Release();
	}
	if (pCode) // Si se ha generado DXIL, es un bytecode.
	{
		ID3D11ComputeShader* pCS = 0;
		m_pDevice->CreateComputeShader(pCode->GetBufferPointer(),
			pCode->GetBufferSize(), NULL, &pCS);
		pCode->Release();
		return pCS;
	}
	return NULL;
}

CDXManager::CDXManager()
{
	m_pContext = NULL;
	m_pDevice = NULL;
	m_pSwapChain = NULL;
}


CDXManager::~CDXManager()
{
}