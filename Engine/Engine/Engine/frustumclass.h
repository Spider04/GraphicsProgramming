#ifndef _FRUSTUMCLASS_H_
#define _FRUSTUMCLASS_H_

#include <D3DX10math.h>


//handles visibility (checks whether something is visible for the camera or not)
class FrustumClass
{
public:
	FrustumClass();
	FrustumClass(const FrustumClass&);
	~FrustumClass();

	void ConstructFrustum(float, D3DXMATRIX, D3DXMATRIX);
	bool CheckCube(float, float, float, float);

private:
	D3DXPLANE m_planes[6];
};

#endif