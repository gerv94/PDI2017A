#include "stdafx.h"
#include "PDIImage.h"
#include <fstream>

using namespace std;


CPDIImage::CPDIImage()
{
	m_nHSize = 0;
	m_nVSize = 0;
	m_pBuffer = 0;
}


CPDIImage::~CPDIImage()
{}


CPDIImage::PIXEL& CPDIImage::operator()(int i, int j) {
	static PIXEL Dummy;
	if (i >= 0 && j >= 0 && i < m_nHSize && j < m_nVSize)
		return m_pBuffer[j*m_nHSize + i];
	return Dummy;
}

CPDIImage* CPDIImage::Create(int nSizeX, int nSizeY) {
	CPDIImage* pImage = new CPDIImage();
	pImage->m_nHSize = nSizeX;
	pImage->m_nVSize = nSizeY;
	pImage->m_pBuffer = (PIXEL*)malloc(sizeof(PIXEL)*nSizeX*nSizeY);
	return pImage;
}

void CPDIImage::Destroy(CPDIImage* pToDestroy) {
	free(pToDestroy->m_pBuffer);
	delete pToDestroy;
}

CPDIImage* CPDIImage::Clone(CPDIImage* pSource) {
	CPDIImage* pClone = Create(pSource->m_nHSize, pSource->m_nVSize);
	memcpy(pClone->m_pBuffer, pSource->m_pBuffer, sizeof(PIXEL)*pSource->m_nHSize*pSource->m_nVSize);
	return pClone;
}

CPDIImage* CPDIImage::SharedCreate(void* pData, int nSizeX, int nSizeY) {
	CPDIImage* pImage = new CPDIImage();
	pImage->m_nHSize = nSizeX;
	pImage->m_nVSize = nSizeY;
	pImage->m_pBuffer = (PIXEL*)pData;
	return pImage;
}

void CPDIImage::SharedDestroy(CPDIImage* pToDestroy) {
	delete pToDestroy;
}

CPDIImage * CPDIImage::CreateFromFile(const char * pszFileName)
{
	// Formato DIB : (Device Independece Bitmap)
	// Utiliza algoritmos sin pérdida como otros archivos como .jpeg

	/*	ETAPAS
		1.- Lectura del encabezado de archivo DIB.
		2.- Lectura de la información de imagen.
		3.- Lectura de los pixeles/índices.
	*/
	// Tipos de codificación de imagen: Píxel o Paletizado (indices)
	// La diferencia entre las codificaciones entre Pixel o Paletizado es que cada pixel guarda el color rgb, mientras que el paletizado utiliza
	// indices para asignar colores a partir de una paleta.

	// 1.- Encabezado DIB
	BITMAPFILEHEADER bfh;
	memset(&bfh, 0, sizeof(bfh));
	fstream in;
	in.open(pszFileName, ios::binary | ios::in);
	if (!in.is_open())
		return NULL;
	// Leer firma digital
	in.read((char*)&bfh.bfType, sizeof(bfh.bfType));
	// Todos los archivos bitmap files inician en BM
	if (bfh.bfType != 'MB')
		return NULL;
	in.read((char*)&bfh.bfSize, sizeof(bfh) - sizeof(bfh.bfType));

	// 2.- Lectura del ecabezado de imagen.
	BITMAPINFOHEADER bih;
	memset(&bih, 0, sizeof(bih));
	in.read((char*)&bih.biSize, sizeof(bih.biSize));
	if (sizeof(BITMAPINFOHEADER) != bih.biSize)
		return NULL;
	in.read((char*)&bih.biWidth, sizeof(bih) - sizeof(bih.biSize));

	CPDIImage* pImage = Create(bih.biWidth, bih.biHeight);
	int nRowLenght = 4 * ((bih.biBitCount*bih.biWidth + 31) / 32);//mide en bits multiplos de 32, el cluster es de 32 bits.
	/*
	- Formato 1bpp 2 colores
	- Formato 4bpp 16 colores
	- Formato 8bpp 256 colores
	- Formato 24bpp no hay paleta
	*/
	//Windows detecta bgr, DirectX detecta rgb
	switch (bih.biBitCount)
	{
	case 1: {
		RGBQUAD paleta[2];
		int nColors = bih.biClrUsed ? bih.biClrUsed : 2;
		in.read((char*) paleta, nColors * sizeof(RGBQUAD));
		unsigned char* pRow = new unsigned char[nRowLenght];
		int block = (bih.biWidth + 7) / 8, totalBits;
		for (int j = bih.biHeight - 1; j >= 0; j--) {
			in.read((char*)pRow, nRowLenght);
			for (int i = 0; i < block; i++) {
				totalBits = ((i + 1) * 8) <= bih.biWidth ? 8 : bih.biWidth % 8;
				for (int x = 0; x < totalBits; x++) {
					PIXEL& pixel = (*pImage)((i * 8) + x, j);
					RGBQUAD& color = paleta[(pRow[i] >> (7 - x)) & 0x01];
					pixel.b = color.rgbRed;
					pixel.g = color.rgbGreen;
					pixel.r = color.rgbBlue;
					pixel.a = 0xff;
				}
			}
		}
		delete[] pRow;//free(pRow);
		break;
	}
	case 4: {
		RGBQUAD paleta[16];
		int nColors = bih.biClrUsed ? bih.biClrUsed : 16;
		in.read((char*) paleta, nColors * sizeof(RGBQUAD));
		int block = (bih.biWidth + 1) / 2;
		unsigned char* pRow = (unsigned char*) malloc(nRowLenght);
		bool flag;
		for (int j = bih.biHeight - 1; j >= 0; j--) {
			in.read((char*)pRow, nRowLenght);
			for (int i = 0; i < block; i++) {
				//Primer pixel
				PIXEL& pixel = (*pImage)(i * 2, j);
				flag = i < block - 1 || bih.biWidth % 2 > 0;
				RGBQUAD& color = paleta[(pRow[i] >> (flag ? 4 : 0)) & 0x0f];
				pixel.b = color.rgbRed;
				pixel.g = color.rgbGreen;
				pixel.r = color.rgbBlue;
				pixel.a = 0xff;
				//Segundo pixel
				if (flag) {
					PIXEL& pixel = (*pImage)(i * 2 + 1, j);
					RGBQUAD& color = paleta[(pRow[i]) & 0x0f];
					pixel.b = color.rgbRed;
					pixel.g = color.rgbGreen;
					pixel.r = color.rgbBlue;
					pixel.a = 0xff;
				}
			}
		}
		delete[] pRow;
		break; 
	}
	case 8: {
		RGBQUAD paleta[256];
		int nColors = bih.biClrUsed ? bih.biClrUsed : 256;
		in.read((char*)paleta, nColors * sizeof(RGBQUAD));
		unsigned char* pRow = new unsigned char[nRowLenght];
		for (int j = bih.biHeight - 1; j >= 0; j--) {
			in.read((char*)pRow, nRowLenght);
			for (int i = 0; i < bih.biWidth; i++) {
				RGBQUAD& color = paleta[pRow[i]];
				PIXEL& pixel = (*pImage)(i, j);
				pixel.b = color.rgbRed;
				pixel.g = color.rgbGreen;
				pixel.r = color.rgbBlue;
				pixel.a = 0xff;
			}
		}
		delete[] pRow;
		break;
	}
	case 24: {
		unsigned char* pRow = new unsigned char[nRowLenght];
		for (int j = bih.biHeight - 1; j >= 0; j--) {
			in.read((char*)pRow, nRowLenght);
			for (int i = 0; i < bih.biWidth; i++){
				PIXEL& pixel = (*pImage)(i, j);
				pixel.r = pRow[i * 3];
				pixel.g = pRow[(i * 3) + 1];
				pixel.b = pRow[(i * 3) + 2];
				pixel.a = 0xff;
			}
		}
		delete[] pRow;
		break;
	}
	case 32: {
		unsigned char* pRow = new unsigned char[nRowLenght];
		for (int j = bih.biHeight - 1; j >= 0; j--) {
			in.read((char*)pRow, nRowLenght);
			for (int i = 0; i < bih.biWidth; i++) {
				PIXEL& pixel = (*pImage)(i, j);
				pixel.r = pRow[i * 4];
				pixel.g = pRow[(i * 4) + 1];
				pixel.b = pRow[(i * 4) + 2];
				pixel.a = pRow[(i * 4) + 3];
			}
		}
		delete[] pRow;
		break;
	}
	default:
		pImage = NULL;
	}
	return pImage;
}

void CPDIImage::Draw(int x, int y, HDC hdc) {
	// Métodos de GDI de hace 30 años (Graphical Digital Interface).
	// HandleDeviceContext : HDC;
	HDC hdcMem = CreateCompatibleDC(hdc); // Manipulación de imagen.
	HBITMAP hbmpMem = CreateCompatibleBitmap(hdc, m_nHSize, m_nVSize); // Volumen de datos.
	SetBitmapBits(hbmpMem, sizeof(PIXEL)*m_nHSize*m_nVSize, m_pBuffer); // Asignarlo a la memoria kernel.
	SelectObject(hdcMem, hbmpMem);
	// BitBlockTransfer
	BitBlt(hdc, x, y, m_nHSize, m_nVSize, hdcMem, 0, 0, SRCCOPY); // Asignar la imagen al GPU y mostrarla.
	DeleteObject(hbmpMem);
	DeleteDC(hdcMem);
}

CPDIImage * CPDIImage::CaptureDesktop()
{
	HDC hdcScreen = CreateDC(L"DISPLAY", 0, 0, 0);
	// Tomar el tamaño.
	int nSizeX = GetDeviceCaps(hdcScreen, HORZRES);  // = GetSystemMetrics(SM_CXSCREEN);
	int nSizeY = GetDeviceCaps(hdcScreen, VERTRES);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);	//Crear dispositivo virtual
	HBITMAP hbmpMem = CreateCompatibleBitmap(hdcScreen, nSizeX, nSizeY); // Crear memoria virtual para el dispositivo virtual.
	SelectObject(hdcMem, hbmpMem);	// Asociar memoria y dispotivo virtuales.
	BitBlt(hdcMem, 0, 0, nSizeX, nSizeY, hdcScreen, 0, 0, SRCCOPY);
	CPDIImage* pCapture = Create(nSizeX, nSizeY);
	GetBitmapBits(hbmpMem, nSizeX*nSizeY * sizeof(PIXEL), pCapture->m_pBuffer);	// Mover memoria de kernel a memoria de aplicación
	// Eliminar dispotivos virtuales y su respectiva memoria:
	DeleteObject(hbmpMem);
	DeleteDC(hdcMem);
	DeleteDC(hdcScreen);
	return pCapture;
}