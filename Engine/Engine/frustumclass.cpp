#include "frustumclass.h"

FrustumClass::FrustumClass()
{
}
FrustumClass::FrustumClass(const FrustumClass& other)
{
}

FrustumClass::~FrustumClass()
{
}


void FrustumClass::ConstructFrustum(float screenDepth, D3DXMATRIX projectionMatrix, D3DXMATRIX viewMatrix)
{
	//calc  min z distance
	float zMinimum = 0.0f, r = 0.0f;
	zMinimum = -projectionMatrix._43 / projectionMatrix._33;
	r = screenDepth / (screenDepth - zMinimum);
	projectionMatrix._33 = r;
	projectionMatrix._43 = -r * zMinimum;

	//create frustum matrix from view and projectiion matrix (checks whether sth. is visible or not)
	D3DXMATRIX matrix;
	D3DXMatrixMultiply(&matrix, &viewMatrix, &projectionMatrix);

	//calc near plane
	m_planes[0].a = matrix._14 + matrix._13;
	m_planes[0].b = matrix._24 + matrix._23;
	m_planes[0].c = matrix._34 + matrix._33;
	m_planes[0].d = matrix._44 + matrix._43;
	D3DXPlaneNormalize(&m_planes[0], &m_planes[0]);

	//calc far plane
	m_planes[1].a = matrix._14 - matrix._13; 
	m_planes[1].b = matrix._24 - matrix._23;
	m_planes[1].c = matrix._34 - matrix._33;
	m_planes[1].d = matrix._44 - matrix._43;
	D3DXPlaneNormalize(&m_planes[1], &m_planes[1]);


	//calc left plane
	m_planes[2].a = matrix._14 + matrix._11; 
	m_planes[2].b = matrix._24 + matrix._21;
	m_planes[2].c = matrix._34 + matrix._31;
	m_planes[2].d = matrix._44 + matrix._41;
	D3DXPlaneNormalize(&m_planes[2], &m_planes[2]);

	//calc right plane
	m_planes[3].a = matrix._14 - matrix._11; 
	m_planes[3].b = matrix._24 - matrix._21;
	m_planes[3].c = matrix._34 - matrix._31;
	m_planes[3].d = matrix._44 - matrix._41;
	D3DXPlaneNormalize(&m_planes[3], &m_planes[3]);

	//calc top plane
	m_planes[4].a = matrix._14 - matrix._12; 
	m_planes[4].b = matrix._24 - matrix._22;
	m_planes[4].c = matrix._34 - matrix._32;
	m_planes[4].d = matrix._44 - matrix._42;
	D3DXPlaneNormalize(&m_planes[4], &m_planes[4]);

	//calc bottom plane
	m_planes[5].a = matrix._14 + matrix._12;
	m_planes[5].b = matrix._24 + matrix._22;
	m_planes[5].c = matrix._34 + matrix._32;
	m_planes[5].d = matrix._44 + matrix._42;
	D3DXPlaneNormalize(&m_planes[5], &m_planes[5]);

	return;
}

//checks if any point of the cube is inside the planes or not
bool FrustumClass::CheckCube(float xCenter, float yCenter, float zCenter, float radius)
{
	for(int i=0; i<6; i++) 
	{
		//goes through all corner coordinates
		if(D3DXPlaneDotCoord(&m_planes[i], &D3DXVECTOR3((xCenter - radius), (yCenter - radius), (zCenter - radius))) >= 0.0f)
			continue;
		
		if(D3DXPlaneDotCoord(&m_planes[i], &D3DXVECTOR3((xCenter + radius), (yCenter - radius), (zCenter - radius))) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &D3DXVECTOR3((xCenter - radius), (yCenter + radius), (zCenter - radius))) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &D3DXVECTOR3((xCenter + radius), (yCenter + radius), (zCenter - radius))) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &D3DXVECTOR3((xCenter - radius), (yCenter - radius), (zCenter + radius))) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &D3DXVECTOR3((xCenter + radius), (yCenter - radius), (zCenter + radius))) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &D3DXVECTOR3((xCenter - radius), (yCenter + radius), (zCenter + radius))) >= 0.0f)
			continue;
		
		if(D3DXPlaneDotCoord(&m_planes[i], &D3DXVECTOR3((xCenter + radius), (yCenter + radius), (zCenter + radius))) >= 0.0f)
			continue;

		return false;
	}

	return true;
}