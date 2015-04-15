#include "quadtreeclass.h"

QuadTreeClass::QuadTreeClass()
	: m_vertexList(0)
	, m_parentNode(0)
{}
QuadTreeClass::QuadTreeClass(const QuadTreeClass& other)
{}

QuadTreeClass::~QuadTreeClass()
{}

bool QuadTreeClass::Initialize(TerrainClass* terrain, ID3D11Device* device)
{
	//get number of vertices and the total triangle count from the terrain vertex array
	int vertexCount;
	vertexCount = terrain->GetVertexCount();
	m_triangleCount = vertexCount / 3;

	m_vertexList = new VertexType[vertexCount];
	if(!m_vertexList)
		return false;

	//copy terrain vertices into vertex list
	terrain->CopyVertexArray((void*)m_vertexList);


	//get center coordinate (2D) and width of mesh
	float centerX = 0.0f, centerZ = 0.0f, width = 0.0f;
	CalculateMeshDimensions(vertexCount, centerX, centerZ, width);

	//create parent node
	m_parentNode = new NodeType;
	if(!m_parentNode)
		return false;

	//create tree (recursive)
	CreateTreeNode(m_parentNode, centerX, centerZ, width, device);

	//release vertex list, since quad tree has vertices in each node
	if(m_vertexList)
	{
		delete [] m_vertexList;
		m_vertexList = 0;
	}

	return true;
}

void QuadTreeClass::Shutdown()
{
	//recursively release quad tree data
	if(m_parentNode)
	{
		ReleaseNode(m_parentNode);
		delete m_parentNode;
		m_parentNode = 0;
	}

	return;
}

void QuadTreeClass::Render(FrustumClass* frustum, ID3D11DeviceContext* deviceContext, TerrainShaderClass* shader)
{
	//reset number of triangles drawn in this frame
	m_drawCount = 0;

	//render each visible node (use frustum to check) + moving down tree
	RenderNode(m_parentNode, frustum, deviceContext, shader);

	return;
}

//returns number of drawn triangles in previous Render call
int QuadTreeClass::GetDrawCount()
{
	return m_drawCount;
}

//determines phzsical quad size of parent node
//return 2D coordinate from center + mesh width
void QuadTreeClass::CalculateMeshDimensions(int vertexCount, float& centerX, float& centerZ, float& meshWidth)
{
	//init center with 0
	centerX = 0;
	centerZ = 0;

	//sum all vertices
	for(int i = 0; i < vertexCount; i++)
	{
		centerX += m_vertexList[i].position.x;
		centerZ += m_vertexList[i].position.z;
	}

	//get midpoint by dividing the sum with the number of vertices
	centerX = centerX / (float)vertexCount;
	centerZ = centerZ / (float)vertexCount;

	//init max and min size + depth
	float maxWidth = 0.0f;
	float minWidth = fabsf(m_vertexList[0].position.x - centerX);

	float maxDepth = 0.0f;
	float minDepth = fabsf(m_vertexList[0].position.z - centerZ);

	//find min and max width + depth
	float width = 0.0f, depth = 0.0f;
	for(int i = 0; i < vertexCount; i++)
	{
		width = fabsf(m_vertexList[i].position.x - centerX);
		depth = fabsf(m_vertexList[i].position.z - centerZ);

		if(width > maxWidth)
			maxWidth = width;
		if(width < minWidth)
			minWidth = width;

		if(depth > maxDepth)
			maxDepth = depth;
		if(depth < minDepth)
			minDepth = depth;
	}

	//get absolute min and max values for width and depth
	float maxX = 0.0f, maxZ = 0.0f;
	maxX = (float)max(fabs(minWidth), fabs(maxWidth));
	maxZ = (float)max(fabs(minDepth), fabs(maxDepth));

	//calc maximum diameter of mesh
	meshWidth = max(maxX, maxZ) * 2.0f;

	return;

}

void QuadTreeClass::CreateTreeNode(NodeType* node, float positionX, float positionZ, float width, ID3D11Device* device)
{
	//store node position and size
	node->positionX = positionX;
	node->positionZ = positionZ;
	node->width = width;

	//init rest of node
	node->triangleCount = 0;
	node->vertexBuffer = 0;
	node->indexBuffer = 0;

	node->nodes[0] = 0;
	node->nodes[1] = 0;
	node->nodes[2] = 0;
	node->nodes[3] = 0;

	//count triangles in this node
	int numTriangles = 0;
	numTriangles = CountTriangles(positionX, positionZ, width);

	//no triangles -> complete
	if(numTriangles == 0)
		return;
	
	//too many triangles -> split into 4
	float offsetX = 0.0f, offsetZ = 0.0f;
	int count = 0;
	if(numTriangles > MAX_TRIANGLES)
	{
		for(int i = 0; i < 4; i++)
		{
			//calc pos offset for new child node
			offsetX = (((i % 2) < 1) ? -1.0f : 1.0f) * (width / 4.0f);
			offsetZ = (((i % 4) < 2) ? -1.0f : 1.0f) * (width / 4.0f);

			//check for triangles in new node
			count = CountTriangles((positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));
			if(count > 0)
			{
				//create new node and extend tree
				node->nodes[i] = new NodeType;
				CreateTreeNode(node->nodes[i], (positionX + offsetX), (positionZ + offsetZ), (width / 2.0f), device);
			}

			count = 0;
		}

		return;
	}

	//triangles are less than maximum -> create list of triangles to store them
	node->triangleCount = numTriangles;

	//create vertex array
	int vertexCount = 0;
	vertexCount = numTriangles * 3;

	VertexType* vertices;
	vertices = new VertexType[vertexCount];

	//create index array
	unsigned long* indices;
	indices = new unsigned long[vertexCount];
	
	int index = 0, vertexIndex = 0;
	bool result = false;
	for(int i = 0; i < m_triangleCount; i++)
	{
		//triangle is inside node -> add to vertex array
		result = IsTriangleContained(i, positionX, positionZ, width);
		if(result)
		{
			vertexIndex = i * 3;

			//get the three vertices from the triangle
			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			indices[index] = index;
			index++;

			vertexIndex++;
			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			indices[index] = index;
			index++;
			
			vertexIndex++;
			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			indices[index] = index;
			index++;
		}
	}
	
	// Set up the description of the vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &node->vertexBuffer);

	// Set up the description of the index buffer.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * vertexCount;
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
	device->CreateBuffer(&indexBufferDesc, &indexData, &node->indexBuffer);

	// Release the vertex and index arrays now that the data is stored in the buffers in the node.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return;
}

int QuadTreeClass::CountTriangles(float positionX, float positionZ, float width)
{
	int count = 0;
	bool result = false;

	for(int i = 0; i < m_triangleCount; i++)
	{
		result = IsTriangleContained(i, positionX, positionZ, width);
		if(result)
			count++;
	}

	return count;
}

//check if given triangle is inside cube dimensions
bool QuadTreeClass::IsTriangleContained(int index, float positionX, float positionZ, float width)
{
	//calculate radius of node
	float radius = 0.0f;
	radius = width * 0.5f;

	//get the three vertices at the index
	int vertexIndex = 0;
	vertexIndex = index * 3;

	float x1 = 0.0f, z1 = 0.0f;
	x1 = m_vertexList[vertexIndex].position.x;
	z1 = m_vertexList[vertexIndex].position.z;
	vertexIndex++;

	float x2 = 0.0f, z2 = 0.0f;
	x2 = m_vertexList[vertexIndex].position.x;
	z2 = m_vertexList[vertexIndex].position.z;
	vertexIndex++;

	float x3 = 0.0f, z3 = 0.0f;
	x3 = m_vertexList[vertexIndex].position.x;
	z3 = m_vertexList[vertexIndex].position.z;

	//check if minimum x of triangle is inside node
	float minimumX = 0.0f;
	minimumX = min(x1, min(x2, x3));
	if(minimumX > (positionX + radius))
		return false;

	//do the same for maxX, minZ and maxZ
	float maximumX = 0.0f;
	maximumX = max(x1, max(x2, x3));
	if(maximumX < (positionX - radius))
		return false;

	float minimumZ = 0.0f;
	minimumZ = min(z1, min(z2, z3));
	if(minimumZ > (positionZ + radius))
		return false;

	float maximumZ = 0.0f;
	maximumZ = max(z1, max(z2, z3));
	if(maximumZ < (positionZ - radius))
		return false;


	return true;
}


//recursive function which releases all nodes and their data throughout the tree
void QuadTreeClass::ReleaseNode(NodeType* node)
{
	for(int i = 0; i < 4; i++)
	{
		if(node->nodes[i] != 0)
			ReleaseNode(node->nodes[i]);
	}

	//delete the data from this node
	if(node->vertexBuffer)
	{
		node->vertexBuffer->Release();
		node->vertexBuffer = 0;
	}

	if(node->indexBuffer)
	{
		node->indexBuffer->Release();
		node->indexBuffer = 0;
	}

	//release the four node childs
	for(int i = 0; i < 4; i++)
	{
		if(node->nodes[i] != 0)
		{
			delete node->nodes[i];
			node->nodes[i] = 0;
		}
	}
}

//render all visible nodes
void QuadTreeClass::RenderNode(NodeType* node, FrustumClass* frustum, ID3D11DeviceContext* deviceContext, TerrainShaderClass* shader)
{
	//do a frustum check in order to check the visibility of the node
	bool result = false;
	result = frustum->CheckCube(node->positionX, 0.0f, node->positionZ, (node->width * 0.5f));
	if(!result)
		return;

	//recursively call function for all child nodes
	int count = 0;
	for(int i = 0; i < 4; i++)
	{
		if(node->nodes[i] != 0)
		{
			count++;
			RenderNode(node->nodes[i], frustum, deviceContext, shader);
		}
	}

	//if children exist, there is no need to render the parent
	if(count != 0)
		return;

	//otherwise, render the parent
	unsigned int stride = 0, offset = 0;
	stride = sizeof(VertexType);

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &node->vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(node->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case a line list.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	//render polygons in this node
	int indexCount = 0;
	indexCount = node->triangleCount * 3;

	shader->RenderShader(deviceContext, indexCount);
	m_drawCount += node->triangleCount;

	return;
}