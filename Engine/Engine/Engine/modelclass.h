#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_


#include <D3D11.h>
#include <D3DX10math.h>
#include <fstream>
using namespace std;

#include "textureclass.h"


//model class - representation of a physical object for rendering
class ModelClass
{
public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device*, char*, WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

	void SetPosition(float, float, float);
	void GetPosition(float&, float&, float&);

private:
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR3 normal;
	};
	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	bool LoadModel(char*);
	bool InitializeBuffers(ID3D11Device*);
	bool LoadTexture(ID3D11Device*, WCHAR*);

	void RenderBuffers(ID3D11DeviceContext*);

	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	TextureClass* m_Texture;
	ModelType* m_model;
	float m_positionX, m_positionY, m_positionZ;
};

#endif