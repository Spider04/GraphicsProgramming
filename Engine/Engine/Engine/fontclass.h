#ifndef _FONTCLASS_H_
#define _FONTCLASS_H_


#include <D3D11.h>
#include <D3DX10math.h>
#include <fstream>
using namespace std;

#include "textureclass.h"


//handles the rendering of font on screen
class FontClass
{
public:
	FontClass();
	FontClass(const FontClass&);
	~FontClass();

	bool Initialize(ID3D11Device*, char*, WCHAR*);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();
	void BuildVertexArray(void*, char*, float, float);

private:
	bool LoadFontData(char*);
	bool LoadTexture(ID3D11Device*, WCHAR*);


	struct FontType
	{
		float left, right;
		int size;
	};
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};

	FontType* m_Font;
	TextureClass* m_Texture;
};

#endif