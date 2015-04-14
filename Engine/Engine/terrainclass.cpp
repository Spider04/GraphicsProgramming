////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "terrainclass.h"


TerrainClass::TerrainClass()
	: m_vertexBuffer(0)
	, m_indexBuffer(0)
{}
TerrainClass::TerrainClass(const TerrainClass& other)
{}

TerrainClass::~TerrainClass()
{}

bool TerrainClass::Initialize(ID3D11Device* device)
{
	// Manually set the width and height of the terrain.
	m_terrainWidth = 100;
	m_terrainHeight = 100;

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	bool result;
	result = InitializeBuffers(device);
	if(!result)
		return false;

	return true;
}

void TerrainClass::Shutdown()
{
	// Release the vertex and index buffer.
	ShutdownBuffers();

	return;
}

void TerrainClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}

int TerrainClass::GetIndexCount()
{
	return m_indexCount;
}

bool TerrainClass::InitializeBuffers(ID3D11Device* device)
{
	// Calculate the number of vertices in the terrain mesh.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 8;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	VertexType* vertices;
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
		return false;

	// Create the index array.
	unsigned long* indices;
	indices = new unsigned long[m_indexCount];
	if(!indices)
		return false;

	// Initialize the index to the vertex array.
	int index;
	index = 0;

	float positionX, positionZ;

	// Load the vertex and index arrays with the terrain data.
	for(int j=0; j<(m_terrainHeight-1); j++)
	{
		for(int i=0; i<(m_terrainWidth-1); i++)
		{
			// LINE 1
			// Upper left.
			positionX = (float)i;
			positionZ = (float)(j+1);

			vertices[index].position = D3DXVECTOR3(positionX, 0.0f, positionZ);
			vertices[index].color = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// Upper right.
			positionX = (float)(i+1);
			positionZ = (float)(j+1);

			vertices[index].position = D3DXVECTOR3(positionX, 0.0f, positionZ);
			vertices[index].color = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// LINE 2
			// Upper right.
			positionX = (float)(i+1);
			positionZ = (float)(j+1);

			vertices[index].position = D3DXVECTOR3(positionX, 0.0f, positionZ);
			vertices[index].color = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// Bottom right.
			positionX = (float)(i+1);
			positionZ = (float)j;

			vertices[index].position = D3DXVECTOR3(positionX, 0.0f, positionZ);
			vertices[index].color = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// LINE 3
			// Bottom right.
			positionX = (float)(i+1);
			positionZ = (float)j;

			vertices[index].position = D3DXVECTOR3(positionX, 0.0f, positionZ);
			vertices[index].color = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// Bottom left.
			positionX = (float)i;
			positionZ = (float)j;

			vertices[index].position = D3DXVECTOR3(positionX, 0.0f, positionZ);
			vertices[index].color = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// LINE 4
			// Bottom left.
			positionX = (float)i;
			positionZ = (float)j;

			vertices[index].position = D3DXVECTOR3(positionX, 0.0f, positionZ);
			vertices[index].color = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// Upper left.
			positionX = (float)i;
			positionZ = (float)(j+1);

			vertices[index].position = D3DXVECTOR3(positionX, 0.0f, positionZ);
			vertices[index].color = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;			
		}
	}

	// Set up the description of the static vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	HRESULT result;
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
		return false;


	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
		return false;

	// Release the arrays now that the buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}


void TerrainClass::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


void TerrainClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	// Set vertex buffer stride and offset.
	unsigned int stride;
	stride = sizeof(VertexType);

	unsigned int offset;
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case a line list.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	return;
}