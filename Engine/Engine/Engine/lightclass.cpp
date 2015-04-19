#include "lightclass.h"

LightClass::LightClass()
{}
LightClass::LightClass(const LightClass& other)
{}

LightClass::~LightClass()
{}


//setter for the light parameters
void LightClass::SetAmbientColor(float red, float green, float blue, float alpha)
{
	m_ambientColor = D3DXVECTOR4(red, green, blue, alpha);
	return;
}
void LightClass::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = D3DXVECTOR4(red, green, blue, alpha);
	return;
}
void LightClass::SetDirection(float x, float y, float z)
{
	m_direction = D3DXVECTOR3(x, y, z);
	return;
}


//getter functions for the light parameters
D3DXVECTOR4 LightClass::GetAmbientColor()
{
	return m_ambientColor;
}
D3DXVECTOR4 LightClass::GetDiffuseColor()
{
	return m_diffuseColor;
}
D3DXVECTOR3 LightClass::GetDirection()
{
	return m_direction;
}