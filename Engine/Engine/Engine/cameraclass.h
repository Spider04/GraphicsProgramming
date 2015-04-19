#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_


#include <D3DX10math.h>


//the camera class which is part of the 3D rendering process
class CameraClass
{
public:
	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	//getter and setter for position (3D)
	D3DXVECTOR3 GetPosition(); 
	void SetPosition(float, float, float);
	
	//getter and setter for rotation
	D3DXVECTOR3 GetRotation();
	void SetRotation(float, float, float);

	//getter and setter for y offset
	void SetYOffset(float);
	float GetYOffset();

	void Render();
	void GetViewMatrix(D3DXMATRIX&);

private:
	//3D position and rotation variables
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;

	//view matrix
	D3DXMATRIX m_viewMatrix;

	//y offset (to physical position)
	float m_yOffset;
};

#endif