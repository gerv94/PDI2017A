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


CPDIImage::PIXEL& CPDIImage::operator()(int i, int j){
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

	switch (bih.biBitCount)
	{
	case 1:{ // 1bpp Monocromático Paletizadas. (bpp = bit por pixel) 
			 // Tarea	 // Colores: Blanco -> 1 & Negro -> 0
		RGBQUAD Paleta[2];
		int nColors = bih.biClrUsed ? bih.biClrUsed : 2;
		in.read((char*)Paleta, nColors * sizeof(RGBQUAD));

		unsigned long RowLength = bih.biWidth + bih.biBitCount;
		unsigned char* pRow = new unsigned char[RowLength];
		CPDIImage* pImage = Create(bih.biWidth, bih.biHeight);
		for (int j = 0; j < bih.biHeight; j++) {
			in.read((char*)pRow, RowLength);
			for (int i = 0; i < bih.biWidth; i++) {
				PIXEL& Pixel = (*pImage)(i, j);
				if (pRow[i] > 0) {
					RGBQUAD& Color = Paleta[1];
					Pixel.r = Color.rgbRed;
					Pixel.g = Color.rgbGreen;
					Pixel.b = Color.rgbBlue;
					Pixel.a = -1;
				}
				else {
					RGBQUAD& Color = Paleta[0];
					Pixel.r = Color.rgbRed;
					Pixel.g = Color.rgbGreen;
					Pixel.b = Color.rgbBlue;
					Pixel.a = -1;
				}
			}
		}
		delete[] pRow;
		return pImage;
	}
	/*
	- Formato Monocromático: Cada byte representa 8 pixeles.
	- Formato 16 colores: Cada byte representa 2 pixeles.
	- Formato 256 colores/ 8 bits: Cada byte representa un pixel.
	- Formato 24 bits: Cada 3 bytes representan un pixel.
	*/
	case 4: {// 4bpp 16 colores Paletizadas. // Cada byte representa 2 pixeles, es decir, 4 bits son de un pixel y otros 4 bits de otro pixel.
			 // Tarea 
		RGBQUAD Paleta[16];
		int nColors = bih.biClrUsed == 0 ? 16 : bih.biClrUsed;
		in.read((char*)Paleta, nColors * sizeof(RGBQUAD));

		unsigned long RowLength =  4 * ((bih.biWidth * bih.biBitCount + 8) / 16);
		unsigned char* pRow = new unsigned char[RowLength];
		CPDIImage* pImage = Create(bih.biWidth, bih.biHeight);
		for (int j = 0; j < bih.biHeight; j++) {
			in.read((char*)pRow, RowLength);
			for (int i = 0; i < bih.biWidth; i++) {
				PIXEL& Pixel = (*pImage)(i, bih.biHeight - j - 1);
				Pixel.r = Paleta[pRow[i]].rgbRed;
				Pixel.g = Paleta[pRow[i]].rgbGreen;
				Pixel.b = Paleta[pRow[i]].rgbBlue;
				Pixel.a = -1;
			}
		}
		delete[] pRow;
		return pImage; 
	}
	case 8:{ // 8bpp 256 colores Paletizadas
		// En el peor de los casos se rellenan 3 bytes.		
		// Leer paleta, 256 colores máximos.
		RGBQUAD Paleta[256];
		int nColors = bih.biClrUsed == 0 ? 256 : bih.biClrUsed;
		in.read((char*)Paleta, nColors * sizeof(RGBQUAD));

		// Leer pixeles.
		unsigned long RowLength = 4 * ((bih.biWidth * bih.biBitCount + 31) / 32); // Bytes, no pixels. pixel -> bit. Cluster de 32 bits.
		unsigned char* pRow = new unsigned char[RowLength];
		CPDIImage* pImage = Create(bih.biWidth, bih.biHeight);
		// Se lee de abajo hacia arriba.
		for (int j = 0; j < bih.biHeight; j++) {
			in.read((char*)pRow, RowLength);
			for (int i = 0; i < bih.biWidth; i++) {
				PIXEL& Pixel = (*pImage)(i, bih.biHeight - j - 1);
				Pixel.r = Paleta[pRow[i]].rgbRed;
				Pixel.g = Paleta[pRow[i]].rgbGreen;
				Pixel.b = Paleta[pRow[i]].rgbBlue;
				Pixel.a = -1; // es igual que (char)0xff;
			}
		}
		delete[] pRow;
		return pImage;
	}
	case 24: { 
	// 24bpp TrueColor B8G8R8  No paletizados.
	// Cada pixel es B8G8R8, es decir, el primer byte es el color B, el segundo byte es el color G, y el tercero es el color R.
	// Tarea
		/* No requiere paleta, leer byte a byte. */
		// Leer pixeles.
		unsigned long RowLength = (bih.biWidth * bih.biBitCount);
		unsigned char* pRow = new unsigned char[RowLength];
		CPDIImage* pImage = Create(bih.biWidth, bih.biHeight);
		for (int j = 0; j < bih.biHeight; j++) {
			/*in.read((char*)pRow, RowLength);
			char* BGR; memset((char*)BGR, 0, 8);
			for (int i = 0; i < RowLength; i++) {
					
			}*/
		}
		
		return pImage;
	}
			 return NULL;
	}
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