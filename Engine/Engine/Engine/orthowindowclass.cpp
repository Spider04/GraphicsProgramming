#include "orthowindowclass.h"

OrthoWindowClass::OrthoWindowClass()
	: m_vertexBuffer(0)
	, m_indexBuffer(0)
{}
OrthoWindowClass::OrthoWindowClass(const OrthoWindowClass& other)
{}

OrthoWindowClass::~OrthoWindowClass()
{}


bool OrthoWindowClass::Initialize(ID3D11Device* device, int windowWidth, int windowHeight)
{
	//init vertex and index buffer
	bool result = false;
	result = InitializeBuffers(device, windowWidth, windowHeight);

	return result;
}

void OrthoWindowClass::Shutdown()
{
	//release buffers
	ShutdownBuffers();

	return;
}

void OrthoWindowClass::Render(ID3D11DeviceContext* deviceContext)
{
	//put buffers on graphic pipeline for rendering
	RenderBuffers(deviceContext);

	return;
}

int OrthoWindowClass::GetIndexCount()
{
	return m_indexCount;
}


bool OrthoWindowClass::InitializeBuffers(ID3D11Device* device, int windowWidth, int windowHeight)
{
	//calc left and right screen coordinate of window
	float left = (float)((windowWidth / 2) * -1);
	float right = left + (float)windowWidth;

	//calc top and bottom screen coordinate of window
	float top = (float)(windowHeight / 2);
	float bottom = top - (float)windowHeight;
	
	//set number of vertexes and index count to 6
	m_vertexCount = 6;
	m_indexCount = m_vertexCount;

	
	//create vertex array
	VertexType* vertices;
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
		return false;

	//create the index array
	unsigned long* indices;
	indices = new unsigned long[m_indexCount];
	if(!indices)
		return false;

	//load vertex array
	//1st triangle, top left
	vertices[0].position = D3DXVECTOR3(left, top, 0.0f);
	vertices[0].texture = D3DXVECTOR2(0.0f, 0.0f);

	//1st triangle, bottom right
	vertices[1].position = D3DXVECTOR3(right, bottom, 0.0f);
	vertices[1].texture = D3DXVECTOR2(1.0f, 1.0f);

	//1st triangle, bottom left
	vertices[2].position = D3DXVECTOR3(left, bottom, 0.0f);
	vertices[2].texture = D3DXVECTOR2(0.0f, 1.0f);


	//2nd triangle, top left
	vertices[3].position = D3DXVECTOR3(left, top, 0.0f);
	vertices[3].texture = D3DXVECTOR2(0.0f, 0.0f);

	//2nd triangle, top right
	vertices[4].position = D3DXVECTOR3(right, top, 0.0f);
	vertices[4].texture = D3DXVECTOR2(1.0f, 0.0f);

	//2nd triangle, bottom right
	vertices[5].position = D3DXVECTOR3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].texture = D3DXVECTOR2(1.0f, 1.0f);

	//load index array
	for(int i=0; i<m_indexCount; i++)
	{
		indices[i] = i;
	}


	//set up vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	//create vertex buffer
	HRESULT result;
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
		return false;


	//set up index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	//create index buffer
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
		return false;


	//release temporary buffers
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}

void OrthoWindowClass::ShutdownBuffers()
{
	//release all pointers
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}

void OrthoWindowClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	//set stride and offset for vertex buffer
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;
    
	//set vertex and index buffer active
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    //set primitive to triangle
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}