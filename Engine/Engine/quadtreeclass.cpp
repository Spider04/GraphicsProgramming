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

	//init vertex array
	node->vertexArray = 0;

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

	//create vertex array
	node->vertexArray = new VectorType[vertexCount];
	
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

			//store vertex position in vertex array
			node->vertexArray[index].x = m_vertexList[vertexIndex].position.x;
			node->vertexArray[index].y = m_vertexList[vertexIndex].position.y;
			node->vertexArray[index].z = m_vertexList[vertexIndex].position.z;

			index++;
			vertexIndex++;

			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			indices[index] = index;

			node->vertexArray[index].x = m_vertexList[vertexIndex].position.x;
			node->vertexArray[index].y = m_vertexList[vertexIndex].position.y;
			node->vertexArray[index].z = m_vertexList[vertexIndex].position.z;

			index++;
			vertexIndex++;

			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			indices[index] = index;

			node->vertexArray[index].x = m_vertexList[vertexIndex].position.x;
			node->vertexArray[index].y = m_vertexList[vertexIndex].position.y;
			node->vertexArray[index].z = m_vertexList[vertexIndex].position.z;

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

	//release vertex array
	if(node->vertexArray)
	{
		delete [] node->vertexArray;
		node->vertexArray = 0;
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

//return height at x,z position
bool QuadTreeClass::GetHeightAtPosition(float positionX, float positionZ, float& height)
{
	//checks if position is actually on mesh
	float meshMinX = 0.0f, meshMaxX = 0.0f, meshMinZ = 0.0f, meshMaxZ = 0.0f;

	meshMinX = m_parentNode->positionX - (m_parentNode->width * 0.5f);
	meshMaxX = m_parentNode->positionX + (m_parentNode->width * 0.5f);

	meshMinZ = m_parentNode->positionZ - (m_parentNode->width * 0.5f);
	meshMaxZ = m_parentNode->positionZ + (m_parentNode->width * 0.5f);

	if(positionX < meshMinX || positionX > meshMaxX || positionZ < meshMinZ || positionZ > meshMaxZ)
		return false;

	//find node and return height
	FindNode(m_parentNode, positionX, positionZ, height);

	return true;
}

void QuadTreeClass::FindNode(NodeType* node, float x, float z, float& height)
{

	//calculate dimensions of node
	float xMin = 0.0f, xMax = 0.0f, zMin = 0.0f, zMax = 0.0f;
	xMin = node->positionX - (node->width * 0.5f);
	xMax = node->positionX + (node->width * 0.5f);

	zMin = node->positionZ - (node->width * 0.5f);
	zMax = node->positionZ + (node->width * 0.5f);

	//check if position is valid
	if(x < xMin || x > xMax || z < zMin || z > zMax)
		return;

	//check if children exist
	int count = 0;
	for(int i = 0; i < 4; i++)
	{
		if(node->nodes[i] != 0)
		{
			count++;
			FindNode(node->nodes[i], x, z, height);
		}
	}

	//return if children exist -> polygon will be on one of the children
	if(count > 0)
		return;

	//check all polygons of this node
	int index = 0;
	float vertex1[3], vertex2[3], vertex3[3];
	bool foundHeight = false;

	for(int i = 0; i < node->triangleCount; i++)
	{
		index = i * 3;
		vertex1[0] = node->vertexArray[index].x;
		vertex1[1] = node->vertexArray[index].y;
		vertex1[2] = node->vertexArray[index].z;

		index++;
		vertex2[0] = node->vertexArray[index].x;
		vertex2[1] = node->vertexArray[index].y;
		vertex2[2] = node->vertexArray[index].z;

		index++;
		vertex3[0] = node->vertexArray[index].x;
		vertex3[1] = node->vertexArray[index].y;
		vertex3[2] = node->vertexArray[index].z;

		foundHeight = CheckHeightOfTriangle(x, z, height, vertex1, vertex2, vertex3);

		if(foundHeight)
			return;
	}

	return;
}

//create 3 lines form triangle and check if position is inside all 3 lines
bool QuadTreeClass::CheckHeightOfTriangle(float x, float z, float& height, float v0[3], float v1[3], float v2[3])
{
	//start coordinate for raycast
	float startVector[3];
	startVector[0] = x;
	startVector[1] = 0.0f;
	startVector[2] = z;

	//direction vector for raycast
	float directionVector[3];
	directionVector[0] = 0.0f;
	directionVector[1] = 1.0f;
	directionVector[2] = 0.0f;

	//calculate then2 edges from the three points
	float edge1[3];
	edge1[0] = v1[0] - v0[0];
	edge1[1] = v1[1] - v0[1];
	edge1[2] = v1[2] - v0[2];

	float edge2[3];
	edge2[0] = v2[0] - v0[0];
	edge2[1] = v2[1] - v0[1];
	edge2[2] = v2[2] - v0[2];

	//calculate normal of triangle
	float normal[3];
	normal[0] = (edge1[1] * edge2[2]) - (edge1[2] * edge2[1]);
	normal[1] = (edge1[2] * edge2[0]) - (edge1[0] * edge2[2]);
	normal[2] = (edge1[0] * edge2[1]) - (edge1[1] * edge2[0]);

	float magnitude = 0.0f;
	magnitude = (float)sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));
	normal[0] = normal[0] / magnitude;
	normal[1] = normal[1] / magnitude;
	normal[2] = normal[2] / magnitude;

	// Find the distance from the origin to the plane.
	float D = 0.0f;
	D = ((-normal[0] * v0[0]) + (-normal[1] * v0[1]) + (-normal[2] * v0[2]));

	// Get the denominator of the equation.
	float denominator = 0.0f;
	denominator = ((normal[0] * directionVector[0]) + (normal[1] * directionVector[1]) + (normal[2] * directionVector[2]));

	//prevent division by zero
	if(fabs(denominator) < 0.0001f)
		return false;

	//get numerator
	float numerator = 0.0f;
	numerator = -1.0f * (((normal[0] * startVector[0]) + (normal[1] * startVector[1]) + (normal[2] * startVector[2])) + D);

	// Calculate where we intersect the triangle.
	float t = 0.0f;
	t = numerator / denominator;

	// Find the intersection vector.
	float Q[3];
	Q[0] = startVector[0] + (directionVector[0] * t);
	Q[1] = startVector[1] + (directionVector[1] * t);
	Q[2] = startVector[2] + (directionVector[2] * t);

	// Find the three edges of the triangle.
	float e1[3];
	e1[0] = v1[0] - v0[0];
	e1[1] = v1[1] - v0[1];
	e1[2] = v1[2] - v0[2];

	float e2[3];
	e2[0] = v2[0] - v1[0];
	e2[1] = v2[1] - v1[1];
	e2[2] = v2[2] - v1[2];

	float e3[3];
	e3[0] = v0[0] - v2[0];
	e3[1] = v0[1] - v2[1];
	e3[2] = v0[2] - v2[2];

	// Calculate the normal for the first edge.
	float edgeNormal[3];
	edgeNormal[0] = (e1[1] * normal[2]) - (e1[2] * normal[1]);
	edgeNormal[1] = (e1[2] * normal[0]) - (e1[0] * normal[2]);
	edgeNormal[2] = (e1[0] * normal[1]) - (e1[1] * normal[0]);

	// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
	float temp[3];
	temp[0] = Q[0] - v0[0];
	temp[1] = Q[1] - v0[1];
	temp[2] = Q[2] - v0[2];

	float determinant = 0.0f;
	determinant = ((edgeNormal[0] * temp[0]) + (edgeNormal[1] * temp[1]) + (edgeNormal[2] * temp[2]));

	// Check if it is outside.
	if(determinant > 0.001f)
		return false;

	// Calculate the normal for the second edge.
	edgeNormal[0] = (e2[1] * normal[2]) - (e2[2] * normal[1]);
	edgeNormal[1] = (e2[2] * normal[0]) - (e2[0] * normal[2]);
	edgeNormal[2] = (e2[0] * normal[1]) - (e2[1] * normal[0]);

	// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
	temp[0] = Q[0] - v1[0];
	temp[1] = Q[1] - v1[1];
	temp[2] = Q[2] - v1[2];

	determinant = ((edgeNormal[0] * temp[0]) + (edgeNormal[1] * temp[1]) + (edgeNormal[2] * temp[2]));

	// Check if it is outside.
	if(determinant > 0.001f)
		return false;

	// Calculate the normal for the third edge.
	edgeNormal[0] = (e3[1] * normal[2]) - (e3[2] * normal[1]);
	edgeNormal[1] = (e3[2] * normal[0]) - (e3[0] * normal[2]);
	edgeNormal[2] = (e3[0] * normal[1]) - (e3[1] * normal[0]);

	// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
	temp[0] = Q[0] - v2[0];
	temp[1] = Q[1] - v2[1];
	temp[2] = Q[2] - v2[2];

	determinant = ((edgeNormal[0] * temp[0]) + (edgeNormal[1] * temp[1]) + (edgeNormal[2] * temp[2]));

	// Check if it is outside.
	if(determinant > 0.001f)
		return false;

	// Now we have our height.
	height = Q[1];

	return true;
}