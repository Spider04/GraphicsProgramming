#include "terrainclass.h"


TerrainClass::TerrainClass()
	: m_heightMap(0)
	, m_floorTexture(0)
	, m_wallTexture(0)
	, m_vertices(0)
{}
TerrainClass::TerrainClass(const TerrainClass& other)
{}

TerrainClass::~TerrainClass()
{}

bool TerrainClass::Initialize(ID3D11Device* device, DungeonGeneratorClass::DungeonData* dungeonArray, WCHAR* floorTextureFilename, WCHAR* wallTextureFilename)
{
	//load height map for the terrain
	bool result;
	result = LoadDungeonData(dungeonArray);
	if(!result)
		return false;

	//normalize the height
	NormalizeHeightMap();

	//calculate shared normals for lighting
	result = CalculateNormals();
	if(!result)
		return false;

	//calculate texture coordinates and load texture
	CalculateTextureCoordinates();
	result = LoadTextures(device, floorTextureFilename, wallTextureFilename);
	if(!result)
		return false;

	// Initialize vertex array that will hold the geometry
	result = InitializeBuffers(device);
	return result;
}

bool TerrainClass::LoadDungeonData(DungeonGeneratorClass::DungeonData* dungeonArray)
{
	bool result;
	result = LoadHeightMap(dungeonArray);
	
	return result;
}

void TerrainClass::Shutdown()
{
	//release textures
	ReleaseTextures();

	// Release vertex array
	ShutdownBuffers();

	//release height map
	ShutdownHeightMap();

	return;
}


ID3D11ShaderResourceView* TerrainClass::GetFloorTexture()
{
	return m_floorTexture->GetTexture();
}
ID3D11ShaderResourceView* TerrainClass::GetWallTexture()
{
	return m_wallTexture->GetTexture();
}

int TerrainClass::GetVertexCount()
{
	return m_vertexCount;
}

void TerrainClass::CopyVertexArray(void* vertexList)
{
	memcpy(vertexList, m_vertices, sizeof(VertexType) * m_vertexCount);
}

//load height map via dungeon array
bool TerrainClass::LoadHeightMap(DungeonGeneratorClass::DungeonData* dungeonData)
{
	//get size of terrain from the image
	m_terrainWidth = dungeonData->dungeonWidth;
	m_terrainHeight = dungeonData->dungeonHeight;

	//create structure for height map data
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if(!m_heightMap)
		return false;

	int index = 0;
	for(int j = 0; j < m_terrainHeight; j++)
	{
		for(int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = (float)dungeonData->dungeonArray[index];
			m_heightMap[index].z = (float)j;
		}
	}
	
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


//generating texture coordinates for grid
void TerrainClass::CalculateTextureCoordinates()
{
	//calculate how much to increment the texture coordinates by
	float incrementValue;
	incrementValue = (float)TEXTURE_REPEAT / (float)m_terrainWidth;

	//how many time to repeat the texture
	int incrementCount;
	incrementCount = m_terrainWidth / TEXTURE_REPEAT;


	//init tu and tv indexes and values
	int tuCount = 0, tvCount = 0;
	float tuCoordinate = 0.0f;
	float tvCoordinate = 1.0f;


	//calculate tu and tv texture coordinate for each vertex
	for(int j = 0; j < m_terrainHeight; j++)
	{
		for(int i = 0; i < m_terrainWidth; i++)
		{
			//store texture coordinate in height map
			m_heightMap[(m_terrainHeight * j) + i].tu = tuCoordinate;
			m_heightMap[(m_terrainHeight * j) + i].tv = tvCoordinate;

			//increment tu value and index
			tuCoordinate += incrementValue;
			tuCount++;

			//reset value when the end of the texture is reached
			if(tuCount == incrementCount)
			{
				tuCoordinate = 0.0f;
				tuCount = 0;
			}
		}

		tvCoordinate -= incrementValue;
		tvCount++;

		if(tvCount == incrementCount)
		{
			tvCoordinate = 1.0f;
			tvCount = 0;
		}
	}

	return;
}

//load texture resource into texture object
bool TerrainClass::LoadTextures(ID3D11Device* device, WCHAR* floorFilename, WCHAR* wallFilename)
{
	//load the floor texture
	m_floorTexture = new TextureClass;
	if(!m_floorTexture)
		return false;

	bool result;
	result = m_floorTexture->Initialize(device, floorFilename);
	if(!result)
		return false;

	//load the wall texture
	m_wallTexture = new TextureClass;
	if(!m_wallTexture)
		return false;

	result = m_wallTexture->Initialize(device, wallFilename);

	return result;
}

void TerrainClass::ReleaseTextures()
{
	if(m_floorTexture)
	{
		m_floorTexture->Shutdown();
		delete m_floorTexture;
		m_floorTexture = 0;
	}

	if(m_wallTexture)
	{
		m_wallTexture->Shutdown();
		delete m_wallTexture;
		m_wallTexture = 0;
	}

	return;
}


bool TerrainClass::InitializeBuffers(ID3D11Device* device)
{
	// Calculate the number of vertices in the terrain mesh.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;

	// Create the vertex array.
	m_vertices = new VertexType[m_vertexCount];
	if(!m_vertices)
		return false;

	// Initialize the index to the vertex array.
	int index = 0;
	int index1 = 0, index2 = 0, index3 = 0, index4 = 0;
	float tu, tv = 0.0f;

	// Load the vertex and index arrays with the terrain data.
	for(int j = 0; j < (m_terrainHeight-1); j++)
	{
		for(int i = 0; i < (m_terrainWidth-1); i++)
		{
			index1 = (m_terrainHeight * j) + i;          // Bottom left.
			index2 = (m_terrainHeight * j) + i + 1;      // Bottom right.
			index3 = (m_terrainHeight * (j + 1)) + i;      // Upper left.
			index4 = (m_terrainHeight * (j + 1)) + i + 1;  // Upper right.

			// Upper left.
			tv = m_heightMap[index3].tv;

			// Modify the texture coordinates to cover the top edge.
			if(tv == 1.0f)
				tv = 0.0f;

			m_vertices[index].position = D3DXVECTOR3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
			m_vertices[index].texture = D3DXVECTOR2(m_heightMap[index3].tu, tv);
			m_vertices[index].normal = D3DXVECTOR3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
			index++;

			// Upper right.
			tu = m_heightMap[index4].tu;
			tv = m_heightMap[index4].tv;

			// Modify the texture coordinates to cover the top and right edge.
			if(tu == 0.0f)
				tu = 1.0f;

			if(tv == 1.0f)
				tv = 0.0f;

			m_vertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			m_vertices[index].texture = D3DXVECTOR2(tu, tv);
			m_vertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			index++;

			// Bottom left.
			m_vertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			m_vertices[index].texture = D3DXVECTOR2(m_heightMap[index1].tu, m_heightMap[index1].tv);
			m_vertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			index++;

			// Bottom left.
			m_vertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			m_vertices[index].texture = D3DXVECTOR2(m_heightMap[index1].tu, m_heightMap[index1].tv);
			m_vertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			index++;

			// Upper right.
			tu = m_heightMap[index4].tu;
			tv = m_heightMap[index4].tv;

			// Modify the texture coordinates to cover the top and right edge.
			if(tu == 0.0f)
				tu = 1.0f;

			if(tv == 1.0f)
				tv = 0.0f;

			m_vertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			m_vertices[index].texture = D3DXVECTOR2(tu, tv);
			m_vertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			index++;

			// Bottom right.
			tu = m_heightMap[index2].tu;

			// Modify the texture coordinates to cover the right edge.
			if(tu == 0.0f)
				tu = 1.0f;

			m_vertices[index].position = D3DXVECTOR3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
			m_vertices[index].texture = D3DXVECTOR2(tu, m_heightMap[index2].tv);
			m_vertices[index].normal = D3DXVECTOR3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
			index++;
		}
	}

	return true;
}

void TerrainClass::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_vertices)
	{
		delete [] m_vertices;
		m_vertices = 0;
	}

	return;
}