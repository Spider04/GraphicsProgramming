#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include <D3D11.h>
#include <D3DX10math.h>


class ModelClass
{
public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device*);
	void Shutdown();
	//puts model geometry in video card to prepare it for drawing the color shader
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	
private:
	//must match the layout in ColorShaderClass
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR4 color;
	};

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	int m_vertexCount, m_indexCount;
};

#endif