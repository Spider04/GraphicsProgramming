#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_

#include <D3D11.h>
#include <D3DX11tex.h>

class TextureClass
{
public:
	TextureClass();
	TextureClass(const TextureClass&);
	~TextureClass();

	//load texture from given file
	bool Initialize(ID3D11Device*, WCHAR*);
	void Shutdown();

	//for rendering the texture via shaders
	ID3D11ShaderResourceView* GetTexture();

private:
	ID3D11ShaderResourceView* m_texture;
};


#endif