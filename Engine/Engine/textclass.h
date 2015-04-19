#ifndef _TEXTCLASS_H_
#define _TEXTCLASS_H_


#include "fontclass.h"
#include "fontshaderclass.h"


//handles all texts for the screen
class TextClass
{
public:
	TextClass();
	TextClass(const TextClass&);
	~TextClass();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, HWND, int, int, D3DXMATRIX);
	void Shutdown();

	bool Render(ID3D11DeviceContext*, FontShaderClass*, D3DXMATRIX, D3DXMATRIX, int);
	bool SetPoints(int, int, ID3D11DeviceContext*);

private:
	struct SentenceType
	{
		ID3D11Buffer *vertexBuffer, *indexBuffer;
		int vertexCount, indexCount, maxLength;
		float red, green, blue;
	};
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};


	bool InitIntroSentence(ID3D11Device*, ID3D11DeviceContext*);
	bool InitEndSentence(ID3D11Device*, ID3D11DeviceContext*);

	bool InitializeSentence(SentenceType**, int, ID3D11Device*);
	bool UpdateSentence(SentenceType*, char*, int, int, float, float, float, ID3D11DeviceContext*);
	void ReleaseSentence(SentenceType**);
	bool RenderSentence(SentenceType*, ID3D11DeviceContext*, FontShaderClass*, D3DXMATRIX, D3DXMATRIX);


	int m_screenWidth, m_screenHeight;
	D3DXMATRIX m_baseViewMatrix;
	FontClass* m_Font;
	SentenceType *m_pointsSentence, *m_introSentence0, *m_introSentence1, *m_introSentence2, *m_endSentence0, *m_endSentence1;
};

#endif