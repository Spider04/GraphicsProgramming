#ifndef _TERRAINCLASS_H_
#define _TERRAINCLASS_H_

#include <d3d11.h>
#include <d3dx10math.h>
#include <stdio.h>

#include "textureclass.h"
#include "dungeon_generator.h"

//defines how often the texture repeats over the grid
const int TEXTURE_REPEAT = 8;

class TerrainClass
{
public:
	TerrainClass();
	TerrainClass(const TerrainClass&);
	~TerrainClass();

	bool Initialize(ID3D11Device*, char*, WCHAR*, WCHAR*, WCHAR*);
	bool Initialize(ID3D11Device*, DungeonGeneratorClass::DungeonData*, WCHAR*, WCHAR*, WCHAR*);
	bool LoadDungeonData(DungeonGeneratorClass::DungeonData*);
	void Shutdown();

	//all textures for the game
	ID3D11ShaderResourceView* GetFloorTexture();
	ID3D11ShaderResourceView* GetWallTexture();
	ID3D11ShaderResourceView* GetSphereTexture();

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

	bool LoadHeightMap(char*);
	bool LoadHeightMap(DungeonGeneratorClass::DungeonData*);

	void NormalizeHeightMap();
	bool CalculateNormals();
	void ShutdownHeightMap();

	void CalculateTextureCoordinates();
	bool LoadTextures(ID3D11Device*, WCHAR*, WCHAR*, WCHAR*);
	void ReleaseTextures();

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	
	int m_terrainWidth, m_terrainHeight;
	HeightMapType* m_heightMap;
	TextureClass *m_floorTexture, *m_wallTexture, *m_sphereTexture;

	int m_vertexCount;
	VertexType* m_vertices;
};

#endif