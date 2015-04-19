#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_


const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;

const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

const int DUNGEON_WIDTH = 256;
const int DUNGEON_HEIGHT = 256;

//includes for building and rendering the dungeon
#include "terrainclass.h"
#include "dungeon_generator.h"

#include "d3dclass.h"
#include "frustumclass.h"
#include "quadtreeclass.h"

#include "textureshaderclass.h"
#include "terrainshaderclass.h"
#include "sphereshaderclass.h"
#include "lightclass.h"

//movement and physics
#include "inputclass.h"
#include "positionclass.h"
#include "timerclass.h"
#include "cameraclass.h"

//classes for displaying text
#include "fontshaderclass.h"
#include "textclass.h"

//post-processing classes
#include "rendertextureclass.h"
#include "orthowindowclass.h"
#include "colorfiltershaderclass.h"


//the applicatiion in a nutshell
class ApplicationClass
{
public:
	ApplicationClass();
	ApplicationClass(const ApplicationClass&);
	~ApplicationClass();

	bool Initialize(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool Frame();

private:
	//these functions will be executed each frame
	bool HandleInput();
	bool RenderGraphics();

	//various rendering functions
	bool RenderIntroScreen();
	bool RenderSceneToTexture();
	bool RenderPostProcessing();
	bool RenderEndScreen();

	//wall collision
	void HandleWallCollision(float&, float&, float&, float, float, float);

	//game state
	enum GameState{INTRO, GAME, END};
	GameState currentGameState;

private:
	//variables for building and rendering the dungeon
	D3DClass* m_Direct3D;
	TerrainClass* m_Terrain;

	FrustumClass* m_Frustum;
	QuadTreeClass* m_QuadTree;
	DungeonGeneratorClass* m_DungeonGenerator;
	bool m_dungeonRecentlyCreated;

	TerrainShaderClass* m_TerrainShader;
	SphereShaderClass* m_SphereShader;
	LightClass* m_Light;
	TextureShaderClass* m_TextureShader;

	//handling movement and physics
	InputClass* m_Input;
	PositionClass* m_Position;
	TimerClass* m_Timer;
	CameraClass* m_Camera;
	
	//variables for displaying text
	FontShaderClass* m_FontShader;
	TextClass* m_Text;
	//amount of points which the player has currently collected
	int m_pointsCollected;

	//post processing variables
	ColorFilterShaderClass* m_ColorFilterShader;
	RenderTextureClass *m_RenderTexture, *m_PostProcessedTexture;

	//2D rendering plane for 2D textures (like post proccessed 2D texture from scene)
	OrthoWindowClass *m_FullScreenWindow;
	D3DXMATRIX m_baseViewMatrix;
};

#endif