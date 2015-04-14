#include "textureclass.h"

TextureClass::TextureClass()
	: m_texture(0)
{}
TextureClass::TextureClass(const TextureClass&)
{}

TextureClass::~TextureClass()
{}

//load texture from given file
bool TextureClass::Initialize(ID3D11Device* device, WCHAR* filename)
{
	HRESULT result;
	result = D3DX11CreateShaderResourceViewFromFile(device, filename, NULL, NULL, &m_texture, NULL);
	if(FAILED(result))
		return false;

	return true;
}

void TextureClass::Shutdown()
{
	if(m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}

	return;
}

//for rendering the texture via shaders
ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_texture;
}