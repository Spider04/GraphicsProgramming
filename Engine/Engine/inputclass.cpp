#include "inputclass.h"

InputClass::InputClass()
	: m_directInput(0)
	, m_keyboard(0)
{}
InputClass::InputClass(const InputClass& other)
{}

InputClass::~InputClass()
{}


bool InputClass::Initialize(HINSTANCE hinstance, HWND hwnd)
{
	//init main input interface
	HRESULT result;
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if(FAILED(result))
		return false;

	//init keyboard interface
	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if(FAILED(result))
		return false;

	//set data format for keyboard
	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if(FAILED(result))
		return false;

	//set cooperative level with keyboard
	result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if(FAILED(result))
		return false;

	//acquire keyboard link
	result = m_keyboard->Acquire();
	if(FAILED(result))
		return false;

	return true;
}

void InputClass::Shutdown()
{
	//release keyboard
	if(m_keyboard)
	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = 0;
	}

	//release main input interface
	if(m_directInput)
	{
		m_directInput->Release();
		m_directInput = 0;
	}

	return;
}

bool InputClass::Frame()
{
	//read current state of keyboard
	HRESULT result;
	result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
	if (FAILED(result))
	{
		//get keyboard back if link was somehow lost
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
			m_keyboard->Acquire();

		else
			return false;
	}

	return true;
}


//check if escape was pressed
bool InputClass::IsEscapePressed()
{
	if(m_keyboardState[DIK_ESCAPE] & 0x80)
		return true;

	return false;
}

//check if spacebar is pressed
bool InputClass::IsSpacebarPressed()
{
	if (m_keyboardState[DIK_SPACE] & 0x80)
		return true;

	return false;
}

//check if the P key is pressed
bool InputClass::IsPPressed()
{
	if (m_keyboardState[DIK_P] & 0x80)
		return true;

	return false;
}


//check if left arrow key is pressed
bool InputClass::IsLeftPressed()
{
	if(m_keyboardState[DIK_LEFT] & 0x80)
		return true;

	return false;
}

//check if right arrow key is pressed
bool InputClass::IsRightPressed()
{
	if(m_keyboardState[DIK_RIGHT] & 0x80)
		return true;

	return false;
}

//check if up arrow key is pressed
bool InputClass::IsUpPressed()
{
	if(m_keyboardState[DIK_UP] & 0x80)
		return true;

	return false;
}

//check if down arrow key is pressed
bool InputClass::IsDownPressed()
{
	if(m_keyboardState[DIK_DOWN] & 0x80)
		return true;

	return false;
}


//check if page up key is pressed
bool InputClass::IsPgUpPressed()
{
	if(m_keyboardState[DIK_PGUP] & 0x80)
		return true;

	return false;
}

//check if page down key is pressed
bool InputClass::IsPgDownPressed()
{
	if(m_keyboardState[DIK_PGDN] & 0x80)
		return true;

	return false;
}