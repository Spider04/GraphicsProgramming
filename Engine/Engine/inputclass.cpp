#include "inputclass.h"

InputClass::InputClass()
{}

InputClass::InputClass(const InputClass& other)
{}

InputClass::~InputClass()
{}

void InputClass::Initialize()
{
	//set all keys as not pressed
	for(int i = 0; i < 256; i++)
		m_keys[i] = false;

	return;
}

void InputClass::KeyDown(unsigned int input)
{
	//save key state in key array
	m_keys[input] = true;
	return;
}

void InputClass::KeyUp(unsigned int input)
{
	//save key state in key array
	m_keys[input] = false;
	return;
}

bool InputClass::IsKeyDown(unsigned int key)
{
	return m_keys[key];
}