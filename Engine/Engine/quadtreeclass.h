#ifndef _QUADTREECLASS_H_
#define _QUADTREECLASS_H_


//condition for quad tree to stop splitting
const int MAX_TRIANGLES = 10000;

#include "terrainclass.h"
#include "frustumclass.h"
#include "terrainshaderclass.h"

class QuadTreeClass
{
public:
	QuadTreeClass();
	QuadTreeClass(const QuadTreeClass&);
	~QuadTreeClass();

	bool Initialize(TerrainClass*, ID3D11Device*);
	void Shutdown();
	void Render(FrustumClass*, ID3D11DeviceContext*, TerrainShaderClass*);

	int GetDrawCount();

private:
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR3 normal;
	};

	struct NodeType
	{
		float positionX, positionZ, width;
		int triangleCount;
		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* indexBuffer;
		NodeType* nodes[4];
	};

	void CalculateMeshDimensions(int, float&, float&, float&);
	void CreateTreeNode(NodeType*, float, float, float, ID3D11Device*);
	int CountTriangles(float, float, float);

	bool IsTriangleContained(int, float, float, float);
	void ReleaseNode(NodeType*);
	void RenderNode(NodeType*, FrustumClass*, ID3D11DeviceContext*, TerrainShaderClass*);

	int m_triangleCount, m_drawCount;
	VertexType* m_vertexList;

	//root node
	NodeType* m_parentNode;
};

#endif