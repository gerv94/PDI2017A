#pragma once
class CPDIImage
{
public:
	struct PIXEL
	{
		unsigned char r, g, b, a; //Agregué unsigned
	};
protected:
	PIXEL* m_pBuffer; // Direccion del primer pixel de la imagen.
	// Resolución de la imagen:
	int    m_nHSize;  // Número de pixeles horizontales.
	int    m_nVSize;  // Número de pixeles verticales.
public:
	static CPDIImage* Create(int nSizeX, int nSizeY);
	static void Destroy(CPDIImage* pToDestroy);
	static CPDIImage* Clone(CPDIImage* pSource);
	static CPDIImage* SharedCreate(void* pData, int nSizeX, int nSizeY);
	static void SharedDestroy(CPDIImage* pToDestroy);
	static CPDIImage* CreateFromFile(const char* pszFileName);
	static CPDIImage* CaptureDesktop();
	void Draw(int x, int y, HDC hdc);
	PIXEL& operator()(int i, int j);
	inline int GetSizeX() { return m_nHSize; }
	inline int GetSizeY() { return m_nVSize; }
protected:
	CPDIImage();
	~CPDIImage();
};

