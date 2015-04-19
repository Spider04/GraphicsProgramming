#include "positionclass.h"

PositionClass::PositionClass()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;
	
	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;

	m_frameTime = 0.0f;

	m_forwardSpeed   = 0.0f;
	m_backwardSpeed  = 0.0f;
	m_upwardSpeed    = 0.0f;
	m_downwardSpeed  = 0.0f;
	m_leftTurnSpeed  = 0.0f;
	m_rightTurnSpeed = 0.0f;
	m_lookUpSpeed    = 0.0f;
	m_lookDownSpeed  = 0.0f;

	m_collisionRadius = 0.0f;
	m_allowedUpwardDifference = 0.0f;
	m_gravityForce = 30.0f;

	//variables for physical values are set to standard values from the beginning
	m_jumpForce = 0.0f;
	m_jumpSpeed = -1.0f;
	m_jumpSpeedMax = 10.0f;

	m_fallSpeed = 0.0f;
	m_fallSpeedMax = 100.0f;
	m_mass = 1.0f;
	m_inJump = false;
	m_jumpPeakReached = false;
}
PositionClass::PositionClass(const PositionClass& other)
{
}

PositionClass::~PositionClass()
{
}


void PositionClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}

void PositionClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}

void PositionClass::GetPosition(float& x, float& y, float& z)
{
	x = m_positionX;
	y = m_positionY;
	z = m_positionZ;
	return;
}

void PositionClass::GetRotation(float& x, float& y, float& z)
{
	x = m_rotationX;
	y = m_rotationY;
	z = m_rotationZ;
	return;
}


void PositionClass::Frame(float time, float floorHeight, bool onMesh)
{
	//reccord frame time
	m_frameTime = time;

	//only calculate physical forces when on mesh
	if(onMesh)
	{
		//jump if necessary
		if(m_inJump && m_jumpSpeed > 0.0f)
		{
			m_jumpSpeed -= m_gravityForce * m_mass * time * 0.001f;
			if(m_jumpSpeed > 0.0f)
				m_positionY += m_jumpSpeed * time * 0.001f;
			else
				m_jumpSpeed = -1.0f;
		}
		else if(m_positionY > (floorHeight + 0.01f))
		{
			//drag the character down
			if(m_fallSpeed < m_fallSpeedMax)
				m_fallSpeed += m_gravityForce * m_mass * time * 0.001f;

			m_positionY -= m_fallSpeed * time * 0.001f;

			if(m_positionY < (floorHeight + 0.01f)){
				m_positionY = floorHeight;
				m_fallSpeed = 0.0f;
				m_inJump = false;
			}
		}

	}else if (m_fallSpeed > 0.0f)
		m_fallSpeed = 0.0f;
		
}


void PositionClass::MoveForward(bool keydown)
{
	//update forward speed (has momentum)
	if(keydown)
	{
		m_forwardSpeed += m_frameTime * 0.001f;

		if(m_forwardSpeed > (m_frameTime * 0.03f))
			m_forwardSpeed = m_frameTime * 0.03f;
	}
	else
	{
		m_forwardSpeed -= m_frameTime * 0.0007f;
		if(m_forwardSpeed < 0.0f)
			m_forwardSpeed = 0.0f;
	}

	//calculate new position based on current forward speed
	float radians;
	radians = m_rotationY * 0.0174532925f;

	m_positionX += sinf(radians) * m_forwardSpeed;
	m_positionZ += cosf(radians) * m_forwardSpeed;

	return;
}

void PositionClass::MoveBackward(bool keydown)
{
	//update backward speed (has momentum)
	if(keydown)
	{
		m_backwardSpeed += m_frameTime * 0.001f;

		if(m_backwardSpeed > (m_frameTime * 0.03f))
			m_backwardSpeed = m_frameTime * 0.03f;
	}
	else
	{
		m_backwardSpeed -= m_frameTime * 0.0007f;
		if(m_backwardSpeed < 0.0f)
			m_backwardSpeed = 0.0f;
	}

	//calculate new position based on current backward speed
	float radians;
	radians = m_rotationY * 0.0174532925f;

	m_positionX -= sinf(radians) * m_backwardSpeed;
	m_positionZ -= cosf(radians) * m_backwardSpeed;

	return;
}


void PositionClass::TurnLeft(bool keydown)
{
	//calc left turn speed
	if(keydown)
	{
		m_leftTurnSpeed += m_frameTime * 0.01f;

		if(m_leftTurnSpeed > (m_frameTime * 0.15f))
			m_leftTurnSpeed = m_frameTime * 0.15f;
	}
	else
	{
		m_leftTurnSpeed -= m_frameTime* 0.005f;
		if(m_leftTurnSpeed < 0.0f)
			m_leftTurnSpeed = 0.0f;
	}

	//update rotation based on the current left turn speed
	m_rotationY -= m_leftTurnSpeed;

	//keep rotation in 360 degrees range
	if(m_rotationY < 0.0f)
		m_rotationY += 360.0f;

	return;
}

void PositionClass::TurnRight(bool keydown)
{
	//calc right turn speed
	if(keydown)
	{
		m_rightTurnSpeed += m_frameTime * 0.01f;

		if(m_rightTurnSpeed > (m_frameTime * 0.15f))
			m_rightTurnSpeed = m_frameTime * 0.15f;
	}
	else
	{
		m_rightTurnSpeed -= m_frameTime* 0.005f;
		if(m_rightTurnSpeed < 0.0f)
			m_rightTurnSpeed = 0.0f;
	}

	//update rotation based on current right turn speed
	m_rotationY += m_rightTurnSpeed;

	//keep rotation in 360 degrees range
	if(m_rotationY > 360.0f)
		m_rotationY -= 360.0f;

	return;
}

void PositionClass::LookUpward(bool keydown)
{
	//update upward rotation
	if(keydown)
	{
		m_lookUpSpeed += m_frameTime * 0.01f;

		if(m_lookUpSpeed > (m_frameTime * 0.15f))
			m_lookUpSpeed = m_frameTime * 0.15f;
	}
	else
	{
		m_lookUpSpeed -= m_frameTime* 0.005f;
		if(m_lookUpSpeed < 0.0f)
			m_lookUpSpeed = 0.0f;
	}

	//update rotation based on the current upward rotation speed
	m_rotationX -= m_lookUpSpeed;

	//keep rotation in 90 degrees range
	if (m_rotationX > 90.0f)
		m_rotationX = 90.0f;

	
	return;
}

void PositionClass::LookDownward(bool keydown)
{
	//update downward rotation
	if(keydown)
	{
		m_lookDownSpeed += m_frameTime * 0.01f;

		if(m_lookDownSpeed > (m_frameTime * 0.15f))
			m_lookDownSpeed = m_frameTime * 0.15f;
	}
	else
	{
		m_lookDownSpeed -= m_frameTime* 0.005f;
		if(m_lookDownSpeed < 0.0f)
			m_lookDownSpeed = 0.0f;
	}

	//update rotation based on the current downward rotation speed
	m_rotationX += m_lookDownSpeed;

	//keep rotation in 90 degrees range
	if (m_rotationX < -90.0f)
		m_rotationX = -90.0f;

	return;
}


void PositionClass::SetCollisionRadius(float newValue)
{
	m_collisionRadius = newValue;
}
void PositionClass::GetCollisionRadius(float& value)
{
	value = m_collisionRadius;
}

void PositionClass::SetAllowedUpwardDifference(float newValue)
{
	m_allowedUpwardDifference = newValue;
}
void PositionClass::GetAllowedUpwardDifference(float& value)
{
	value = m_allowedUpwardDifference;
}


void PositionClass::SetGravityForce(float value)
{
	m_gravityForce = value;
}
void PositionClass::SetJumpForce(float value)
{
	m_jumpForce = value;
}

void PositionClass::Jump()
{
	//only calculate new jump if the character is not currently in a jump
	if(!m_inJump){
		m_inJump = true;
		m_jumpSpeed = m_jumpSpeedMax;
	}
}