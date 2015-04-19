#ifndef _POSITIONCLASS_H_
#define _POSITIONCLASS_H_

#include <math.h>


//instance acts as physical representation of player
//--> all physical forces on player are handled here (gravity, jump)
class PositionClass
{
public:
	PositionClass();
	PositionClass(const PositionClass&);
	~PositionClass();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	void GetPosition(float&, float&, float&);
	void GetRotation(float&, float&, float&);

	void Frame(float, float, bool);
	void Jump();

	void MoveForward(bool);
	void MoveBackward(bool);

	void TurnLeft(bool);
	void TurnRight(bool);
	void LookUpward(bool);
	void LookDownward(bool);

	void SetCollisionRadius(float);
	void GetCollisionRadius(float&);
	void SetAllowedUpwardDifference(float);
	void GetAllowedUpwardDifference(float&);

	void SetGravityForce(float);
	void SetJumpForce(float);

private:
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;

	float m_frameTime;

	float m_forwardSpeed, m_backwardSpeed;
	float m_upwardSpeed, m_downwardSpeed;
	float m_leftTurnSpeed, m_rightTurnSpeed;
	float m_lookUpSpeed, m_lookDownSpeed;
	
	//physics functions and variables
	float m_collisionRadius;
	float m_allowedUpwardDifference;

	float m_gravityForce;
	float m_fallSpeed;
	float m_fallSpeedMax;
	float m_mass;

	float m_jumpForce;
	float m_jumpSpeed;
	float m_jumpSpeedMax;

	bool m_inJump;
	bool m_jumpPeakReached;
};

#endif