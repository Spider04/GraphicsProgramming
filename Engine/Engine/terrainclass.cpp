////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "terrainclass.h"


TerrainClass::TerrainClass()
	: m_vertexBuffer(0)
	, m_indexBuffer(0)
	, m_heightMap(0)
{}
TerrainClass::TerrainClass(const TerrainClass& other)
{}

TerrainClass::~TerrainClass()
{}

bool TerrainClass::Initialize(ID3D11Device* device, char* heightMapFilename)
{
	//load height map for the terrain
	bool result;
	result = LoadHeightMap(heightMapFilename);
	if(!result)
		return false;

	//normalize the height
	NormalizeHeightMap();

	//calculate shared normals for lighting
	result = CalculateNormals();
	if(!result)
		return false;

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	return result;
}

void TerrainClass::Shutdown()
{
	// Release the vertex and index buffer.
	ShutdownBuffers();

	//release height map
	ShutdownHeightMap();

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


bool TerrainClass::LoadHeightMap(char* filename)
{
	//open height map in binary
	FILE* filePtr;
	int error;

	error = fopen_s(&filePtr, filename, "rb");
	if(error != 0)
		return false;

	//read file header
	unsigned int count;
	BITMAPFILEHEADER bitmapFileHeader;
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if(count != 1)
		return false;

	//read bitmap info header
	BITMAPINFOHEADER bitmapInfoHeader;
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if(count != 1)
		return false;

	//get size of terrain from the image
	m_terrainWidth = bitmapInfoHeader.biWidth;
	m_terrainHeight = bitmapInfoHeader.biHeight;

	//calculate image size
	int imageSize;
	imageSize = m_terrainWidth * m_terrainHeight * 3;

	//allocate memory for bmp image
	unsigned char* bitmapImage;
	bitmapImage = new unsigned char[imageSize];
	if(!bitmapImage)
		return false;

	//move to beginning of bmp data
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	//read image data
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if(count != imageSize)
		return false;

	//close file
	error = fclose(filePtr);
	if(error != 0)
		return false;
	


	//create structure for height map data
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if(!m_heightMap)
		return false;

	int k = 0;
	int index;
	unsigned char height;

	for(int j = 0; j < m_terrainHeight; j++)
	{
		for(int i = 0; i < m_terrainWidth; i++)
		{
			height = bitmapImage[k];
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = (float)height;
			m_heightMap[index].z = (float)j;

			//added by three, since we just need one of the color values for the grey scale
			k += 3;
		}
	}

	//release bitmap image data
	delete [] bitmapImage;
	bitmapImage = 0;
	
	return true;
}

void TerrainClass::NormalizeHeightMap()
{
	for(int j = 0; j < m_terrainHeight; j++)
	{
		for(int i = 0; i < m_terrainWidth; i++)
		{
			m_heightMap[(m_terrainHeight * j) + i].y /= 15.0f;
		}
	}

	return;
}

bool TerrainClass::CalculateNormals()
{
	//goes thorugh each vertex and takes an average of the normal for each face that the vertex is part of
	//afterwards each vertex will be averaged to have less abrupt changes in light direction
	
	//temporary array to hold un-normalized normal vectors
	VectorType* normals;
	normals = new VectorType[(m_terrainHeight -1) * (m_terrainWidth - 1)];
	if(!normals)
		return false;
	
	//calculate normals for all faces in the mesh
	int index, count = 0;
	int index1, index2, index3, index4 = 0;
	float vertex1[3], vertex2[3], vertex3[3];
	float vector1[3], vector2[3];
	
	for(int j = 0; j < (m_terrainHeight - 1); j++)
	{
		for(int i = 0; i < (m_terrainWidth - 1); i++)
		{
			index1 = (j * m_terrainHeight) + i;
			index2 = (j * m_terrainHeight) + i + 1;
			index3 = ((j + 1) * m_terrainHeight) + i;

			//get vertices from face
			vertex1[0] = m_heightMap[index1].x;
			vertex1[1] = m_heightMap[index1].y;
			vertex1[2] = m_heightMap[index1].z;
		
			vertex2[0] = m_heightMap[index2].x;
			vertex2[1] = m_heightMap[index2].y;
			vertex2[2] = m_heightMap[index2].z;
		
			vertex3[0] = m_heightMap[index3].x;
			vertex3[1] = m_heightMap[index3].y;
			vertex3[2] = m_heightMap[index3].z;

			//calculate two vectors for the face
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (m_terrainHeight-1)) + i;

			//calculate the cross product of those two vectors to get the un-normalized value for this face normal
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
		}
	}

	//take an average of each face normal that the vertex touches in order to get the averaged normal for that vertex
	float sum[3], length;
	for(int j = 0; j < m_terrainHeight; j++)
	{
		for(int i = 0; i < m_terrainWidth; i++)
		{
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Initialize the count.
			count = 0;

			// Bottom left face.
			if(((i-1) >= 0) && ((j-1) >= 0))
			{
				index = ((j-1) * (m_terrainHeight-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Bottom right face.
			if((i < (m_terrainWidth-1)) && ((j-1) >= 0))
			{
				index = ((j-1) * (m_terrainHeight-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper left face.
			if(((i-1) >= 0) && (j < (m_terrainHeight-1)))
			{
				index = (j * (m_terrainHeight-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper right face.
			if((i < (m_terrainWidth-1)) && (j < (m_terrainHeight-1)))
			{
				index = (j * (m_terrainHeight-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}
			
			// Take the average of the faces touching this vertex.
			sum[0] = (sum[0] / (float)count);
			sum[1] = (sum[1] / (float)count);
			sum[2] = (sum[2] / (float)count);

			// Calculate the length of this normal.
			length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));
			
			// Get an index to the vertex location in the height map array.
			index = (j * m_terrainHeight) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			m_heightMap[index].nx = (sum[0] / length);
			m_heightMap[index].ny = (sum[1] / length);
			m_heightMap[index].nz = (sum[2] / length);
		}
	}

	//release temporary normals
	delete [] normals;
	normals = 0;

	return true;
}

void TerrainClass::ShutdownHeightMap()
{
	if(m_heightMap)
	{
		delete [] m_heightMap;
		m_heightMap = 0;
	}

	return;
}


bool TerrainClass::InitializeBuffers(ID3D11Device* device)
{
	// Calculate the number of vertices in the terrain mesh.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;

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
	int index = 0;
	int index1, index2, index3, index4 = 0;

	// Load the vertex and index arrays with the terrain data.
	for(int j = 0; j < (m_terrainHeight-1); j++)
	{
		for(int i = 0; i < (m_terrainWidth-1); i++)
		{
			index1 = (m_terrainHeight * j) + i; //bottom left
			index2 = (m_terrainHeight * j) + i + 1; //bottom right
			index3 = (m_terrainHeight * (j+1)) + i; //upper left
			index4 = (m_terrainHeight * (j+1)) + i + 1; //upper right

			// Upper left.
			vertices[index].position = D3DXVECTOR3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
			vertices[index].normal = D3DXVECTOR3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
			indices[index] = index;
			index++;

			// Upper right.
			vertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			vertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			indices[index] = index;
			index++;

			// Bottom left.
			vertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			vertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			indices[index] = index;
			index++;

			// Bottom left.
			vertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			vertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			indices[index] = index;
			index++;

			// Upper right.
			vertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			vertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			indices[index] = index;
			index++;

			// Bottom right.
			vertices[index].position = D3DXVECTOR3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
			vertices[index].normal = D3DXVECTOR3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
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
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}