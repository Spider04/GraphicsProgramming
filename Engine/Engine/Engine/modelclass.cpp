#include "modelclass.h"

ModelClass::ModelClass()
	: m_vertexBuffer(0)
	, m_indexBuffer(0)
	, m_Texture(0)
	, m_model(0)
{}
ModelClass::ModelClass(const ModelClass& other)
{}

ModelClass::~ModelClass()
{
}


bool ModelClass::Initialize(ID3D11Device* device, char* modelFilename, WCHAR* textureFilename)
{
	//load model data
	bool result = false;
	result = LoadModel(modelFilename);
	if(!result)
		return false;

	//init vertex and index buffer
	result = InitializeBuffers(device);
	if(!result)
		return false;

	//load texture for model
	result = LoadTexture(device, textureFilename);
	if(!result)
		return false;

	return true;
}

//load model from text file
bool ModelClass::LoadModel(char* filename)
{
	//open txt file
	ifstream fin;
	fin.open(filename);

	//if opening the file failed, break
	if (fin.fail())
		return false;

	//read vertex count
	char input;
	fin.get(input);

	while (input != ':')
	{
		fin.get(input);
	}
	fin >> m_vertexCount;

	//index count = number of vertices
	m_indexCount = m_vertexCount;

	//create model
	m_model = new ModelType[m_vertexCount];
	if (!m_model)
		return false;

	//read data
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	//read vertex data
	for (int i = 0; i < m_vertexCount; i++)
	{
		fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
		fin >> m_model[i].tu >> m_model[i].tv;
		fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
	}

	//close model file
	fin.close();

	return true;
}

//init vertex and index buffer
bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	//create vertex array
	VertexType* vertices;
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
		return false;

	//create index array
	unsigned long* indices = 0;
	indices = new unsigned long[m_indexCount];
	if (!indices)
		return false;

	//load vertex and index array
	for (int i = 0; i<m_vertexCount; i++)
	{
		vertices[i].position = D3DXVECTOR3(m_model[i].x, m_model[i].y, m_model[i].z);
		vertices[i].texture = D3DXVECTOR2(m_model[i].tu, m_model[i].tv);
		vertices[i].normal = D3DXVECTOR3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

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
	if (FAILED(result))
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
	if (FAILED(result))
		return false;

	//release temporary arrays
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

//load texture for model
bool ModelClass::LoadTexture(ID3D11Device* device, WCHAR* filename)
{
	//create texture object
	m_Texture = new TextureClass;
	if (!m_Texture)
		return false;

	//init texture object
	bool result = false;
	result = m_Texture->Initialize(device, filename);
	if (!result)
		return false;

	return true;
}


void ModelClass::Shutdown()
{
	//release all pointers
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}

	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	if (m_model)
	{
		delete[] m_model;
		m_model = 0;
	}

	return;
}

void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	//put vertex and index buffer on graphic pipeline
	RenderBuffers(deviceContext);

	return;
}

void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	//set stride and ffset for vertex buffer
	unsigned int stride;
	stride = sizeof(VertexType);

	unsigned int offset;
	offset = 0;

	//set vertex and index buffer to active
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//set primitives to triangles
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_Texture->GetTexture();
}

void ModelClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}

void ModelClass::GetPosition(float& x, float& y, float& z)
{
	x = m_positionX;
	y = m_positionY;
	z = m_positionZ;
	return;
}