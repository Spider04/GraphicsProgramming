#ifndef _TERRAINCLASS_H_
#define _TERRAINCLASS_H_


#include <D3D11.h>
#include <D3DX10math.h>
#include <stdio.h>

#include "textureclass.h"
#include "dungeon_generator.h"

//defines how often the texture repeats over the grid
const int TEXTURE_REPEAT = 8;


//representation of the terrain
class TerrainClass
{
public:
	TerrainClass();
	TerrainClass(const TerrainClass&);
	~TerrainClass();

	bool Initialize(ID3D11Device*, DungeonGeneratorClass::DungeonData*, WCHAR*, WCHAR*);
	bool LoadDungeonData(DungeonGeneratorClass::DungeonData*);
	void Shutdown();

	//all textures for the game
	ID3D11ShaderResourceView* GetFloorTexture();
	ID3D11ShaderResourceView* GetWallTexture();

	int GetVertexCount();
	void CopyVertexArray(void*);

private:
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	    D3DXVECTOR3 normal;
	};
	struct HeightMapType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};
	struct VectorType
	{
		float x, y, z;
	};


	bool LoadHeightMap(DungeonGeneratorClass::DungeonData*);
	void NormalizeHeightMap();
	bool CalculateNormals();
	void ShutdownHeightMap();

	void CalculateTextureCoordinates();
	bool LoadTextures(ID3D11Device*, WCHAR*, WCHAR*);
	void ReleaseTextures();

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	

	int m_terrainWidth, m_terrainHeight;
	HeightMapType* m_heightMap;
	TextureClass *m_floorTexture, *m_wallTexture;

	int m_vertexCount;
	VertexType* m_vertices;
};

#endif