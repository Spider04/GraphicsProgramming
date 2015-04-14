#include "cameraclass.h"

CameraClass::CameraClass()
	: m_positionX(0.0f)
	, m_positionY(0.0f)
	, m_positionZ(0.0f)
	, m_rotationX(0.0f)
	, m_rotationY(0.0f)
	, m_rotationZ(0.0f)
{}
CameraClass::CameraClass(const CameraClass& other)
{}

CameraClass::~CameraClass()
{}

void CameraClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}
void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}

D3DXVECTOR3 CameraClass::GetPosition()
{
	return D3DXVECTOR3(m_positionX, m_positionY, m_positionZ);
}
D3DXVECTOR3 CameraClass::GetRotation()
{
	return D3DXVECTOR3(m_rotationX, m_rotationY, m_rotationZ);
}

//build and update view matrix
void CameraClass::Render()
{
	//creat upwards vector
	D3DXVECTOR3 up;
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	//set position
	D3DXVECTOR3 position;
	position.x = m_positionX;
	position.y = m_positionY;
	position.z = m_positionZ;

	//set default looking direction
	D3DXVECTOR3 lookAt;
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	//set rotation in radians
	float pitch, yaw, roll;
	pitch = m_rotationX * 0.0174532925f; //x axis
	yaw = m_rotationX * 0.0174532925f; //y axis
	roll = m_rotationX * 0.0174532925f; //z axis

	//create roation matrix
	D3DXMATRIX rotationMatrix;
	D3DXMatrixRotationYawPitchRoll(&rotationMatrix, yaw, pitch, roll);

	//transform lookAt and up vector according to rotation matrix
	D3DXVec3TransformCoord(&lookAt, &lookAt, &rotationMatrix);
	D3DXVec3TransformCoord(&up, &up, &rotationMatrix);

	//translated rotated position to position of viewer
	lookAt = position + lookAt;

	//create view matrix
	D3DXMatrixLookAtLH(&m_viewMatrix, &position, &lookAt, &up);

	return;
}

void CameraClass::GetViewMatrix(D3DXMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}