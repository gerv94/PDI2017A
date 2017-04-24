// PDI2017A.cpp: define el punto de entrada de la aplicación.
//

#include "stdafx.h"
#include "PDI2017A.h"
#include "PDIImage.h"
#include <math.h>
#include "DXManager.h"
#include <commdlg.h> // Dialogos comunes de Windows

#define MAX_LOADSTRING 100

// Variables globales:
HINSTANCE hInst;                                // Instancia actual
WCHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // nombre de clase de la ventana principal
CDXManager g_Manager;
ID3D11ComputeShader* g_pCSDefault;

// Declaraciones de funciones adelantadas incluidas en este módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: colocar código aquí.

    // Inicializar cadenas globales
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PDI2017A, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Realizar la inicialización de la aplicación:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PDI2017A));

    MSG msg;

    // Bucle principal de mensajes:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCIÓN: MyRegisterClass()
//
//  PROPÓSITO: registrar la clase de ventana.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PDI2017A));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground  = 0;//(HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PDI2017A);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCIÓN: InitInstance(HINSTANCE, int)
//
//   PROPÓSITO: guardar el identificador de instancia y crear la ventana principal
//
//   COMENTARIOS:
//
//        En esta función, se guarda el identificador de instancia en una variable común y
//        se crea y muestra la ventana principal del programa.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Almacenar identificador de instancia en una variable global

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   auto pAdapter = g_Manager.ChooseAdapter(NULL);
   g_Manager.Initialize(pAdapter, hWnd, pAdapter ? false : true);
   g_pCSDefault = g_Manager.Compile(L"..\\Shaders\\Default.hlsl", "main");
   DXGI_ADAPTER_DESC dad;
   if (pAdapter) {
	   pAdapter->GetDesc(&dad);
	   SetWindowText(hWnd, dad.Description);
	   pAdapter->Release();
   }
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCIÓN: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PROPÓSITO:  procesar mensajes de la ventana principal.
//
//  WM_COMMAND  - procesar el menú de aplicaciones
//  WM_PAINT    - Pintar la ventana principal
//  WM_DESTROY  - publicar un mensaje de salida y volver
//
//

CPDIImage* g_pImgSource;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'C':
			if (g_pImgSource) CPDIImage::Destroy(g_pImgSource);
			g_pImgSource = NULL;
			g_pImgSource = CPDIImage::CaptureDesktop();
			InvalidateRect(hWnd, 0, 0);
			break;
		case 'F':
			OPENFILENAMEA ofn;
			char szFileName[1024] = "";
			memset(&ofn, 0, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFile = szFileName;
			ofn.lpstrFilter = "Archivos de imágenes BMP (*.bmp)\0*.bmp\0"; // Multinull string
			ofn.nMaxFile = 1023;
			if (GetOpenFileNameA(&ofn)) {
				if (g_pImgSource)
					CPDIImage::Destroy(g_pImgSource);
				g_pImgSource = CPDIImage::CreateFromFile(szFileName);
				InvalidateRect(hWnd, 0, 0);
			}
			break;
		}
		break;
	case WM_CREATE:
		//SetTimer(hWnd, 1, 100, NULL);
		return 0;
	case WM_TIMER:
		/*switch (wParam)
		{
			case 1:
				InvalidateRect(hWnd, NULL, false);
		}*/
		return 0;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Analizar las selecciones de menú:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
		/*
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Agregar cualquier código de dibujo que use hDC aquí...
			/*CPDIImage* pImg = CPDIImage::Create(640, 480);
			
			static float t = 0.0f;
			for (int j = 0; j < 480; j++)
				for (int i = 0; i < 640; i++) {
					CPDIImage::PIXEL p = { 127+127*cos(8*t+i*3.141592/180), 127+127*sin(5*t+j*3.141592/180), t*i*j, 1 };
					(*pImg)(i, j) = p;
				}

			t += 0.01;
			pImg->Draw(0, 0, hdc);
			CPDIImage::Destroy(pImg);
			*/

		// Algoritmo no destructivo
		/*if (g_pImgSource) {
				auto Clone = CPDIImage::Clone(g_pImgSource);
				for (int j = 0; j < g_pImgSource->GetSizeY(); j++)
					for (int i = 0; i < g_pImgSource->GetSizeX(); i++) {
						CPDIImage::PIXEL A = (*g_pImgSource)(i, j);
						CPDIImage::PIXEL B = (*g_pImgSource)(i + 1, j);
						CPDIImage::PIXEL P;
						P.r = 127 + ((int)A.r - B.r) / 2;
						P.g = 127 + ((int)A.g - B.g) / 2;
						P.b = 127 + ((int)A.b - B.b) / 2;
						//(*g_pImgSource)(i, j) = P;
						(*Clone)(i, j) = P;
					}
				//g_pImgSource->Draw(0, 0, hdc);
				Clone->Draw(0, 0, hdc);
				CPDIImage::Destroy(Clone);
				*//*
				g_pImgSource->Draw(0, 0, hdc);
			}
            EndPaint(hWnd, &ps);*/
		{
			// 1.- Obtener el recurso destino (salida)
			ID3D11Texture2D* pBackBuffer = 0;
			g_Manager.GetSwapChain()->GetBuffer(0, 
				IID_ID3D11Texture2D, (void**)&pBackBuffer);
			// 2.- Crear una UAV del recurso destino.
			ID3D11UnorderedAccessView* pUAV = 0;
			g_Manager.GetDevice()->CreateUnorderedAccessView(pBackBuffer, NULL, &pUAV);
			// 3.- Conectar la UAV del recurso destiiino como saluda de la tubería de comp.
			g_Manager.GetContext()->CSSetUnorderedAccessViews(0, 1, &pUAV, NULL);
			// 4.- Ejecución
			g_Manager.GetContext()->CSSetShader(g_pCSDefault, 0, 0);
			g_Manager.GetContext()->Dispatch(200, 100, 1);
			g_Manager.GetSwapChain()->Present(1, 0);
			// Liberar recursos utilizados.
			pUAV->Release();
			pBackBuffer->Release();
        }
		// Le dice a Windows que ya pintó.
		ValidateRect(hWnd, NULL);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Controlador de mensajes del cuadro Acerca de.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}