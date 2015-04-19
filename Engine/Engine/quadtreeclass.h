#ifndef _QUADTREECLASS_H_
#define _QUADTREECLASS_H_


//condition for quad tree to stop splitting
const int MAX_TRIANGLES = 10000;

#include "terrainclass.h"
#include "frustumclass.h"
#include "terrainshaderclass.h"
#include "dungeon_generator.h"


//splits the terrain up in multiple nodes and stores them in a tree-like structure
class QuadTreeClass
{
public:
	QuadTreeClass();
	QuadTreeClass(const QuadTreeClass&);
	~QuadTreeClass();

	bool Initialize(TerrainClass*, ID3D11Device*, DungeonGeneratorClass::DungeonData*);
	void Shutdown();
	void Render(FrustumClass*, ID3D11DeviceContext*, TerrainShaderClass*);

	int GetDrawCount();
	//return height at specific x,z position
	bool GetHeightAtPosition(float, float, float&);

private:
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR3 normal;
	};

	//position information - used in node for its vertices
	struct VectorType
	{
		float x, y, z;
	};

	struct NodeType
	{
		float positionX, positionZ, width;
		int triangleCount;
		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* indexBuffer;
		VectorType* vertexArray;
		NodeType* nodes[4];
	};

	void CalculateMeshDimensions(int, float&, float&, float&);
	void CreateTreeNode(NodeType*, float, float, float, ID3D11Device*);
	int CountTriangles(float, float, float);

	bool IsTriangleContained(int, float, float, float);
	void ReleaseNode(NodeType*);
	void RenderNode(NodeType*, FrustumClass*, ID3D11DeviceContext*, TerrainShaderClass*);

	//returns node at position
	void FindNode(NodeType*, float, float, float&);

	//determines which triangle inside the node is located above
	bool CheckHeightOfTriangle(float, float, float&, float[3], float[3], float[3]);

	int m_triangleCount, m_drawCount;
	VertexType* m_vertexList;
	NodeType* m_parentNode; //root node

	DungeonGeneratorClass::DungeonData* m_dungeonData;
};

#endif