#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

//handles the input from the player - saves all key states (although we need just a few)
class InputClass
{
public:
	InputClass();
	InputClass(const InputClass&);
	~InputClass();

	bool Initialize(HINSTANCE, HWND);
	void Shutdown();
	bool Frame();

	bool IsEscapePressed();
	bool IsSpacebarPressed();
	bool IsPPressed();

	bool IsLeftPressed();
	bool IsRightPressed();
	bool IsUpPressed();
	bool IsDownPressed();

	bool IsPgUpPressed();
	bool IsPgDownPressed();

private:
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_keyboard;

	unsigned char m_keyboardState[256];
};

#endif