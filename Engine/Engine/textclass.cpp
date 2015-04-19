#include "textclass.h"

TextClass::TextClass()
	: m_Font(0)
	, m_pointsSentence(0)
	, m_introSentence0(0)
	, m_introSentence1(0)
	, m_introSentence2(0)
	, m_endSentence0(0)
	, m_endSentence1(0)
{}
TextClass::TextClass(const TextClass& other)
{
}

TextClass::~TextClass()
{
}


bool TextClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight, 
						   D3DXMATRIX baseViewMatrix)
{
	//record screen width, height and the base view matrix for rendering
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_baseViewMatrix = baseViewMatrix;

	//init font object
	m_Font = new FontClass;
	if(!m_Font)
		return false;

	bool result = false;
	result = m_Font->Initialize(device, "../Engine/data/fontdata.txt", L"../Engine/data/font.dds");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the font object.", L"Error", MB_OK);
		return false;
	}

	//init sentence object for intro
	result = InitIntroSentence(device, deviceContext);
	if (!result)
		return false;

	//init sentence object for displaying the points
	result = InitializeSentence(&m_pointsSentence, 32, device);
	if(!result)
		return false;

	//init sentence for end sentence
	result = InitEndSentence(device, deviceContext);
	if (!result)
		return false;


	return true;
}

bool TextClass::InitIntroSentence(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	bool result = InitializeSentence(&m_introSentence0, 64, device);
	if (!result)
		return false;

	//setup point string
	char renderString0[64];
	strcpy_s(renderString0, "Collect all  orbs to finish the game.");

	//update sentence vertex buffer
	result = UpdateSentence(m_introSentence0, renderString0, (m_screenWidth / 2) - 100, (m_screenHeight / 2) - 10, 1.0f, 0.0f, 0.0f, deviceContext);
	if (!result)
		return false;


	result = InitializeSentence(&m_introSentence1, 64, device);
	if (!result)
		return false;

	//setup point string
	char renderString1[64];
	strcpy_s(renderString1, "Use arrow keys for moving and [Space] for jump.");

	//update sentence vertex buffer
	result = UpdateSentence(m_introSentence1, renderString1, (m_screenWidth / 2) - 135, (m_screenHeight / 2) + 10, 1.0f, 0.0f, 0.0f, deviceContext);
	if (!result)
		return false;


	result = InitializeSentence(&m_introSentence2, 32, device);
	if (!result)
		return false;

	//setup point string
	char renderString2[32];
	strcpy_s(renderString2, "Loading Dungeon ...");

	//update sentence vertex buffer
	result = UpdateSentence(m_introSentence2, renderString2, (m_screenWidth / 2) - 70, (m_screenHeight / 2) + 50, 1.0f, 0.0f, 0.0f, deviceContext);
	if (!result)
		return false;

	return true;
}

bool TextClass::InitEndSentence(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	bool result = InitializeSentence(&m_endSentence0, 32, device);
	if (!result)
		return false;

	//setup point string
	char renderString0[32];
	strcpy_s(renderString0, "Thanks for playing.");

	//update sentence vertex buffer
	result = UpdateSentence(m_endSentence0, renderString0, (m_screenWidth / 2) - 75, (m_screenHeight / 2) - 10, 1.0f, 0.0f, 0.0f, deviceContext);
	if (!result)
		return false;


	result = InitializeSentence(&m_endSentence1, 64, device);
	if (!result)
		return false;

	//setup point string
	char renderString1[64];
	strcpy_s(renderString1, "Press [Esc] to quit or [P] to generate the dungeon again");

	//update sentence vertex buffer
	result = UpdateSentence(m_endSentence1, renderString1, (m_screenWidth / 2) - 170, (m_screenHeight / 2) + 40, 1.0f, 0.0f, 0.0f, deviceContext);
	if (!result)
		return false;

	return true;
}

bool TextClass::InitializeSentence(SentenceType** sentence, int maxLength, ID3D11Device* device)
{
	//create sentence object
	*sentence = new SentenceType;
	if (!*sentence)
		return false;

	//init buffers
	(*sentence)->vertexBuffer = 0;
	(*sentence)->indexBuffer = 0;

	//set max length and number of vertices
	(*sentence)->maxLength = maxLength;
	(*sentence)->vertexCount = 6 * maxLength;
	(*sentence)->indexCount = (*sentence)->vertexCount;

	//create vertex array
	VertexType* vertices;
	vertices = new VertexType[(*sentence)->vertexCount];
	if (!vertices)
		return false;

	//create index array
	unsigned long* indices;
	indices = new unsigned long[(*sentence)->indexCount];
	if (!indices)
		return false;

	//init vertex and index array
	memset(vertices, 0, (sizeof(VertexType) * (*sentence)->vertexCount));
	for (int i = 0; i<(*sentence)->indexCount; i++)
	{
		indices[i] = i;
	}


	//set up desc of vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * (*sentence)->vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	//create vertex data
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	//create vertex buffer
	HRESULT result;
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(*sentence)->vertexBuffer);
	if (FAILED(result))
		return false;


	//set up desc of static index buffer.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * (*sentence)->indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	//create index data
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	//create index buffer
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &(*sentence)->indexBuffer);
	if (FAILED(result))
		return false;

	//release vertex and index array
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}


void TextClass::Shutdown()
{
	//release font object
	if(m_Font)
	{
		m_Font->Shutdown();
		delete m_Font;
		m_Font = 0;
	}

	//release sentence object
	ReleaseSentence(&m_pointsSentence);

	return;
}

void TextClass::ReleaseSentence(SentenceType** sentence)
{
	if (*sentence)
	{
		//release vertex buffer
		if ((*sentence)->vertexBuffer)
		{
			(*sentence)->vertexBuffer->Release();
			(*sentence)->vertexBuffer = 0;
		}

		//release index buffer
		if ((*sentence)->indexBuffer)
		{
			(*sentence)->indexBuffer->Release();
			(*sentence)->indexBuffer = 0;
		}

		//release pointer
		delete *sentence;
		*sentence = 0;
	}

	return;
}


bool TextClass::Render(ID3D11DeviceContext* deviceContext, FontShaderClass* FontShader, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix
	, int gameStateIndex)
{
	//render the sentence
	bool result = false;
	if (gameStateIndex == 1)
		result = RenderSentence(m_pointsSentence, deviceContext, FontShader, worldMatrix, orthoMatrix);

	else if (gameStateIndex == 0)
	{
		result = RenderSentence(m_introSentence0, deviceContext, FontShader, worldMatrix, orthoMatrix);
		if (!result)
			return false;

		result = RenderSentence(m_introSentence1, deviceContext, FontShader, worldMatrix, orthoMatrix);
		if (!result)
			return false;

		result = RenderSentence(m_introSentence2, deviceContext, FontShader, worldMatrix, orthoMatrix);
	}
	else
	{
		result = RenderSentence(m_endSentence0, deviceContext, FontShader, worldMatrix, orthoMatrix);
		if (!result)
			return false;

		result = RenderSentence(m_endSentence1, deviceContext, FontShader, worldMatrix, orthoMatrix);
	}

	return result;
}

bool TextClass::RenderSentence(SentenceType* sentence, ID3D11DeviceContext* deviceContext, FontShaderClass* FontShader, D3DXMATRIX worldMatrix,
	D3DXMATRIX orthoMatrix)
{
	//set stride and offset for vertex
	unsigned int stride, offset;
	stride = sizeof(VertexType);
	offset = 0;

	//set vertex buffer to active to enable rendering it
	deviceContext->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);

	//do the same for the indx buffer
	deviceContext->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//set primitives to triangles
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//get pixel color from sentence object
	D3DXVECTOR4 pixelColor;
	pixelColor = D3DXVECTOR4(sentence->red, sentence->green, sentence->blue, 1.0f);

	//sart rendering via the font shader
	bool result = false;
	result = FontShader->Render(deviceContext, sentence->indexCount, worldMatrix, m_baseViewMatrix, orthoMatrix, m_Font->GetTexture(), pixelColor);

	return result;
}


bool TextClass::UpdateSentence(SentenceType* sentence, char* text, int positionX, int positionY, float red, float green, float blue,
							   ID3D11DeviceContext* deviceContext)
{
	//record color
	sentence->red = red;
	sentence->green = green;
	sentence->blue = blue;

	//record number of letters
	int numLetters = 0;
	numLetters = (int)strlen(text);

	//check for buffer overflow
	if(numLetters > sentence->maxLength)
		return false;


	//create verte array
	VertexType* vertices;
	vertices = new VertexType[sentence->vertexCount];
	if(!vertices)
		return false;

	//init vertex array
	memset(vertices, 0, (sizeof(VertexType) * sentence->vertexCount));

	//calc start position
	float drawX, drawY;
	drawX = (float)(((m_screenWidth / 2) * -1) + positionX);
	drawY = (float)((m_screenHeight / 2) - positionY);

	//build vertex array
	m_Font->BuildVertexArray((void*)vertices, text, drawX, drawY);

	//lock vertex buffer
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	result = deviceContext->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
		return false;


	//get pointer to vertex buffer data
	VertexType* verticesPtr;
	verticesPtr = (VertexType*)mappedResource.pData;

	//copy the data and unlock vertex buffer
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence->vertexCount));
	deviceContext->Unmap(sentence->vertexBuffer, 0);

	//release vertex array
	delete [] vertices;
	vertices = 0;

	return true;
}

//set text for displaying the points
bool TextClass::SetPoints(int achievedPoints, int allPoints, ID3D11DeviceContext* deviceContext)
{
	//truncate point count to prevent buffer overflow
	if(achievedPoints > 99)
		achievedPoints = 99;

	//convert achieved point int to string
	char tempString[10];
	_itoa_s(achievedPoints, tempString, 10);
	
	//truncate point count to prevent buffer overflow
	if(allPoints > 99)
		allPoints = 99;

	//convert all point int to string
	char tempString2[10];
	_itoa_s(allPoints, tempString2, 10);

	//setup point string
	char renderString[32];
	strcpy_s(renderString, "Points: ");
	strcat_s(renderString, tempString);
	strcat_s(renderString, " / ");
	strcat_s(renderString, tempString2);

	//update sentence vertex buffer
	bool result = false;
	result = UpdateSentence(m_pointsSentence, renderString, 10, 50, 0.0f, 1.0f, 0.0f, deviceContext);

	return result;
}