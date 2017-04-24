#pragma once
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <Windows.h>
class CDXManager
{
protected:
	IDXGISwapChain* m_pSwapChain; // Front buffer y el back buffer.
	ID3D11Device*   m_pDevice;	  // Fabrica de recursos.	
	ID3D11DeviceContext* m_pContext; // Comandos sobre los recursos.
public:
	IDXGIAdapter* ChooseAdapter(HWND hwnd);
	bool Initialize(IDXGIAdapter* pAdapter, HWND hWnd, bool bUseCPU = false);
	IDXGISwapChain* GetSwapChain() { return m_pSwapChain; }
	ID3D11Device* GetDevice() { return m_pDevice; }
	ID3D11DeviceContext* GetContext() { return m_pContext; }
	ID3D11ComputeShader* Compile(wchar_t* pszFileName, char* pszEntryPoint);

	CDXManager();
	~CDXManager();
};

