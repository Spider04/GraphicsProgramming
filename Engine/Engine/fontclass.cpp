#include "fontclass.h"

FontClass::FontClass()
	: m_Font(0)
	, m_Texture(0)
{}
FontClass::FontClass(const FontClass& other)
{}

FontClass::~FontClass()
{}


bool FontClass::Initialize(ID3D11Device* device, char* fontFilename, WCHAR* textureFilename)
{
	//load font data from text file
	bool result = false;
	result = LoadFontData(fontFilename);
	if(!result)
		return false;

	//load font character texture
	result = LoadTexture(device, textureFilename);
	return result;
}

void FontClass::Shutdown()
{
	//release and delete texture
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}

	//release and delete font data
	if (m_Font)
	{
		delete[] m_Font;
		m_Font = 0;
	}

	return;
}

bool FontClass::LoadFontData(char* filename)
{
	//create font space
	m_Font = new FontType[95];
	if(!m_Font)
		return false;

	//read in font data file
	ifstream fin;
	fin.open(filename);
	if(fin.fail())
		return false;

	//read all 95 ASCII chars in
	char temp;
	for(int i=0; i<95; i++)
	{
		fin.get(temp);
		while(temp != ' ')
		{
			fin.get(temp);
		}

		fin.get(temp);
		while(temp != ' ')
		{
			fin.get(temp);
		}

		fin >> m_Font[i].left;
		fin >> m_Font[i].right;
		fin >> m_Font[i].size;
	}

	//close file
	fin.close();

	return true;
}

bool FontClass::LoadTexture(ID3D11Device* device, WCHAR* filename)
{
	//create and init texture object
	m_Texture = new TextureClass;
	if(!m_Texture)
		return false;

	bool result = false;
	result = m_Texture->Initialize(device, filename);

	return result;
}


ID3D11ShaderResourceView* FontClass::GetTexture()
{
	return m_Texture->GetTexture();
}

void FontClass::BuildVertexArray(void* vertices, char* sentence, float drawX, float drawY)
{
	//create ointer to vertices
	VertexType* vertexPtr = 0;
	vertexPtr = (VertexType*)vertices;

	//get string length
	int numLetters = (int)strlen(sentence);

	//init index for vertex array and the letter index
	int index = 0;
	int letter = 0;

	//draw each letter
	for(int i=0; i < numLetters; i++)
	{
		//get letter index
		letter = ((int)sentence[i]) - 32;

		//in case of a space, move coordinate 3 (letter) positions
		if(letter == 0)
			drawX = drawX + 3.0f;

		else
		{
			//---- 1st triangle in quad
			//top left
			vertexPtr[index].position = D3DXVECTOR3(drawX, drawY, 0.0f);
			vertexPtr[index].texture = D3DXVECTOR2(m_Font[letter].left, 0.0f);
			index++;

			//bottom right
			vertexPtr[index].position = D3DXVECTOR3((drawX + m_Font[letter].size), (drawY - 16), 0.0f);
			vertexPtr[index].texture = D3DXVECTOR2(m_Font[letter].right, 1.0f);
			index++;

			//bottom left
			vertexPtr[index].position = D3DXVECTOR3(drawX, (drawY - 16), 0.0f);
			vertexPtr[index].texture = D3DXVECTOR2(m_Font[letter].left, 1.0f);
			index++;

			//---- 2nd triangle
			//top left
			vertexPtr[index].position = D3DXVECTOR3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = D3DXVECTOR2(m_Font[letter].left, 0.0f);
			index++;

			//top right
			vertexPtr[index].position = D3DXVECTOR3(drawX + m_Font[letter].size, drawY, 0.0f);
			vertexPtr[index].texture = D3DXVECTOR2(m_Font[letter].right, 0.0f);
			index++;

			//bottom right
			vertexPtr[index].position = D3DXVECTOR3((drawX + m_Font[letter].size), (drawY - 16), 0.0f);
			vertexPtr[index].texture = D3DXVECTOR2(m_Font[letter].right, 1.0f);
			index++;

			//update x location with the letter size + 1px
			drawX = drawX + m_Font[letter].size + 1.0f;
		}
	}

	return;
}