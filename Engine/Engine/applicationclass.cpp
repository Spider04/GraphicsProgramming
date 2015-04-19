#include "applicationclass.h"

//init all variables at creation
ApplicationClass::ApplicationClass()
{
	//start offf with Intro
	currentGameState = INTRO;

	//reset all pointers to 0 (nno particular order)
	m_Input = 0;
	m_Direct3D = 0;
	m_Camera = 0;
	m_Terrain = 0;

	m_FontShader = 0;
	m_Text = 0;
	m_TerrainShader = 0;
	m_SphereShader = 0;

	m_Light = 0;
	m_Frustum = 0;
	m_QuadTree = 0;
	m_DungeonGenerator = 0;

	m_ColorFilterShader = 0;
	m_RenderTexture = 0;
	m_PostProcessedTexture = 0;
	m_FullScreenWindow = 0;
	m_TextureShader = 0;

	//reset collected points to 0
	m_pointsCollected = 0;

	//since dungeon is not created at this point, reset the boolean variable for it
	m_dungeonRecentlyCreated = false;
}
ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}

ApplicationClass::~ApplicationClass()
{
}

//initialization at start
bool ApplicationClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	bool result = false;

	//--- set up the rendering variables for rednering the loading screen first

	//create and init direct 3D object - handles communication with video card
	m_Direct3D = new D3DClass;
	if (!m_Direct3D)
		return false;

	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize DirectX 11.", L"Error", MB_OK);
		return false;
	}

	//create and init camera object - handles the 3D rendering (gets position from position class)
	m_Camera = new CameraClass;
	if (!m_Camera)
		return false;

	//get base view matrix to handle the 2D rendering properly later
	//otherwise the app tries to render the 2D texture from the side, resulting in weird results
	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(m_baseViewMatrix);

	//create and init init the full screen ortho window object - acts as reference for rendering a 2D texture to the screen
	m_FullScreenWindow = new OrthoWindowClass;
	if (!m_FullScreenWindow)
		return false;

	result = m_FullScreenWindow->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the full screen ortho window object.", L"Error", MB_OK);
		return false;
	}

	//create and init font shader - shader for rendering text
	m_FontShader = new FontShaderClass;
	if (!m_FontShader)
		return false;

	result = m_FontShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
		return false;
	}

	//create and init text object - hold all string in special sentence objects to render them
	m_Text = new TextClass;
	if (!m_Text)
		return false;

	//inits intro and end screen by itself at this point, since no external variables are necessary for them
	result = m_Text->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, m_baseViewMatrix);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
		return false;
	}


	//---- render the intro screen to have something on the screen while loading the rest

	//reset scene (to black screen)
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	//when rendering 2D, we can turn the Z buffer off
	m_Direct3D->TurnZBufferOff();

	//get world and ortho matrix in order to render
	D3DXMATRIX worldMatrix, orthoMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);

	//render the text (takes game state into account to decide what to render)
	result = m_Text->Render(m_Direct3D->GetDeviceContext(), m_FontShader, worldMatrix, orthoMatrix, (int)currentGameState);
	if (!result)
		return false;

	//turn z buffer back on
	m_Direct3D->TurnZBufferOn();

	//end scene + presents rendered result to screen
	m_Direct3D->EndScene();


	//------ creating the dungeon

	//create and init dungeon generator - handles all issues with the dungeon itself (procedural generation, spawning objects, ...)
	m_DungeonGenerator = new DungeonGeneratorClass;
	if(!m_DungeonGenerator)
		return false;

	result = m_DungeonGenerator->Initialize(DUNGEON_WIDTH, DUNGEON_HEIGHT);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the dungeon object.", L"Error", MB_OK);
		return false;
	}

	//create a new dungeon (for now, just for 9 rooms)
	result = m_DungeonGenerator->GenerateNewDungeon(DUNGEON_ROOMS, m_Direct3D);
	if(!result)
	{
		MessageBox(hwnd, L"Could not create a new dungeon.", L"Error", MB_OK);
		return false;
	}


	//----- setting up the rest of the objects for rendering

	//create and init terrain object
	m_Terrain = new TerrainClass;
	if(!m_Terrain)
		return false;

	result = m_Terrain->Initialize(m_Direct3D->GetDevice(), m_DungeonGenerator->GetDungeonData(), L"../Engine/data/dirt01.dds", L"../Engine/data/rock.dds");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the terrain object.", L"Error", MB_OK);
		return false;
	}

	//create and init frustum object - handles visibility issues
	m_Frustum = new FrustumClass;
	if (!m_Frustum)
		return false;

	//create and init quad tree object - breaks terrain down into smaller chunks to only those chunks which are (potentially) visible to the player
	m_QuadTree = new QuadTreeClass;
	if (!m_QuadTree)
		return false;

	result = m_QuadTree->Initialize(m_Terrain, m_Direct3D->GetDevice(), m_DungeonGenerator->GetDungeonData());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the quad tree object.", L"Error", MB_OK);
		return false;
	}

	//create and init light object - directional light to illuminate the scene
	m_Light = new LightClass;
	if (!m_Light)
		return false;

	m_Light->SetAmbientColor(0.05f, 0.05f, 0.05f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(-0.5f, -1.0f, 0.0f);


	//---- init shaders for the terrain and the collectibles

	//create and init terrain shader
	m_TerrainShader = new TerrainShaderClass;
	if(!m_TerrainShader)
		return false;

	result = m_TerrainShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the terrain shader object.", L"Error", MB_OK);
		return false;
	}

	//create and init sphere shader - used for collectible objects
	m_SphereShader = new SphereShaderClass;
	if(!m_SphereShader)
		return false;

	result = m_SphereShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the sphere shader object.", L"Error", MB_OK);
		return false;
	}


	//----- init shaders for post processing

	//create and init texture shader (rendering final texture to screen)
	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader)
		return false;

	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	//create and init color filter shader - for now, it removes the information for the color green
	m_ColorFilterShader = new ColorFilterShaderClass;
	if (!m_ColorFilterShader)
		return false;

	result = m_ColorFilterShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the color filter shader object.", L"Error", MB_OK);
		return false;
	}

	//create and init render to texture object (target for rendering the complete scene at first)
	m_RenderTexture = new RenderTextureClass;
	if (!m_RenderTexture)
		return false;

	result = m_RenderTexture->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the render to texture object.", L"Error", MB_OK);
		return false;
	}
	
	//create and init the final texture for post processing
	m_PostProcessedTexture = new RenderTextureClass;
	if(!m_PostProcessedTexture)
		return false;

	result = m_PostProcessedTexture->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the post processing render to texture object.", L"Error", MB_OK);
		return false;
	}


	//--- set variables for movement and navigation

	//create and init input object - handles input from keyboard and mouse
	m_Input = new InputClass;
	if (!m_Input)
		return false;

	result = m_Input->Initialize(hinstance, hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
		return false;
	}

	//create and init timer object -> returns frame time, e.g. necessary for physics calculation
	m_Timer = new TimerClass;
	if (!m_Timer)
		return false;

	result = m_Timer->Initialize();
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the timer object.", L"Error", MB_OK);
		return false;
	}

	//create and init position object - handles all physical movement in the 3D world (e.g. collision)
	m_Position = new PositionClass;
	if (!m_Position)
		return false;


	//get spawn position of player in dungeon
	float startPosX = 0.0f, startPosY = 0.0f, startPosZ = 0.0f;
	m_DungeonGenerator->GetSpawningCoord(startPosX, startPosY, startPosZ);

	//set initial position of player to this position
	m_Position->SetPosition(startPosX, startPosY, startPosZ);
	
	//add collision radius to ccalculate collision with walls in time
	m_Position->SetCollisionRadius(0.5f);

	//allowed upward(s) difference - used to decide how much slope the char is able to go upwards
	//initialized for the ground only (not 0.0f, since float variables tend to have some calculation issues)
	m_Position->SetAllowedUpwardDifference(0.1f);


	//set y offset for camera - camera acts as head while the position object is the body
	//--> camera needs fixed distance to position
	float cameraOffset = 2.0f;
	m_Camera->SetYOffset(cameraOffset);

	//set the initial position equal to the one from the position object (+ y offset)
	m_Camera->SetPosition(startPosX, startPosY + cameraOffset, startPosZ);


	//set text for collected points
	result = m_Text->SetPoints(m_pointsCollected, m_DungeonGenerator->GetTotalPointCount(), m_Direct3D->GetDeviceContext());
	if (!result)
	{
		MessageBox(hwnd, L"Could not display points.", L"Error", MB_OK);
		return false;
	}


	//set game state to game
	currentGameState = GAME;

	return true;
}

//function to clean up memory when ending the application
void ApplicationClass::Shutdown()
{
	//shut down all objects (if posssible), delete their data and reset all pointers to them
	if(m_FullScreenWindow)
	{
		m_FullScreenWindow->Shutdown();
		delete m_FullScreenWindow;
		m_FullScreenWindow = 0;
	}

	if(m_RenderTexture)
	{
		m_RenderTexture->Shutdown();
		delete m_RenderTexture;
		m_RenderTexture = 0;
	}

	if (m_ColorFilterShader)
	{
		m_ColorFilterShader->Shutdown();
		delete m_ColorFilterShader;
		m_ColorFilterShader = 0;
	}

	if(m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	if(m_DungeonGenerator)
	{
		m_DungeonGenerator->Shutdown();
		delete m_DungeonGenerator;
		m_DungeonGenerator = 0;
	}

	if(m_QuadTree)
	{
		m_QuadTree->Shutdown();
		delete m_QuadTree;
		m_QuadTree = 0;
	}

	if(m_Frustum)
	{
		delete m_Frustum;
		m_Frustum = 0;
	}

	if(m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	if(m_TerrainShader)
	{
		m_TerrainShader->Shutdown();
		delete m_TerrainShader;
		m_TerrainShader = 0;
	}

	if(m_SphereShader)
	{
		m_SphereShader->Shutdown();
		delete m_SphereShader;
		m_SphereShader = 0;
	}

	if(m_Text)
	{
		m_Text->Shutdown();
		delete m_Text;
		m_Text = 0;
	}

	if(m_FontShader)
	{
		m_FontShader->Shutdown();
		delete m_FontShader;
		m_FontShader = 0;
	}

	if(m_Terrain)
	{
		m_Terrain->Shutdown();
		delete m_Terrain;
		m_Terrain = 0;
	}

	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	if(m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}

	if (m_Timer)
	{
		delete m_Timer;
		m_Timer = 0;
	}

	if (m_Position)
	{
		delete m_Position;
		m_Position = 0;
	}

	if(m_Input)
	{
		m_Input->Shutdown();
		delete m_Input;
		m_Input = 0;
	}

	return;
}

//function called each frame
bool ApplicationClass::Frame()
{
	bool result;

	//get user input for this frame
	result = m_Input->Frame();
	if(!result)
		return false;
	
	//end app if the usser clicked escape
	if(m_Input->IsEscapePressed() == true)
		return false;

	//game code  per frame
	if (currentGameState == GAME)
	{
		//update the frame time
		m_Timer->Frame();

		//record old position (necessary for resetting player when stuck into a wall)
		float posX, posY, posZ;
		m_Position->GetPosition(posX, posY, posZ);

		//get height for old position
		bool foundHeight = false;
		foundHeight = m_QuadTree->GetHeightAtPosition(posX, posZ, posY);

		//calculated new position and calculate gravity effect
		m_Position->Frame(m_Timer->GetTime(), posY, foundHeight);

		//overwrite old position with this one
		m_Position->GetPosition(posX, posY, posZ);

		//handle the input
		result = HandleInput();
		if (!result)
			return false;

		//record new position (after calulating the movement)
		float newX = 0.0f, newY = 0.0f, newZ = 0.0f;
		m_Position->GetPosition(newX, newY, newZ);

		//handle collisions with the wall, resets the new position to the old one if collision with wall was detected
		HandleWallCollision(newX, newY, newZ, posX, posY, posZ);
		m_Position->SetPosition(newX, newY, newZ);

		//check if a collectible was collected in this frame
		result = m_DungeonGenerator->CollectibleAtPosition(newX, newY, newZ);
		if (result)
		{
			//update point counter and uppdate the text for it
			m_pointsCollected++;
			result = m_Text->SetPoints(m_pointsCollected, m_DungeonGenerator->GetTotalPointCount(), m_Direct3D->GetDeviceContext());
			if (!result)
				return false;

			//set game state to END if the player got all available points
			if (m_pointsCollected >= m_DungeonGenerator->GetTotalPointCount())
				currentGameState = END;
		}

		//get rotation
		float rotX, rotY, rotZ;
		m_Position->GetRotation(rotX, rotY, rotZ);

		//sync camera's position and rotation with the one from the position object
		m_Camera->SetPosition(newX, newY + m_Camera->GetYOffset(), newZ);
		m_Camera->SetRotation(rotX, rotY, rotZ);

		//finally, render graphics
		result = RenderGraphics();
		if (!result)
			return false;
	}
	//render end screen
	else
	{
		//reset scene (to black screen)
		m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

		//when rendering 2D, we can turn the Z buffer off
		m_Direct3D->TurnZBufferOff();

		//get world and ortho matrix
		D3DXMATRIX worldMatrix, orthoMatrix;
		m_Direct3D->GetWorldMatrix(worldMatrix);
		m_Direct3D->GetOrthoMatrix(orthoMatrix);

		//render text (end screen)
		bool result = m_Text->Render(m_Direct3D->GetDeviceContext(), m_FontShader, worldMatrix, orthoMatrix, (int)currentGameState);
		if (!result)
			return false;

		//turn z buffer back on
		m_Direct3D->TurnZBufferOn();

		//end scene + presents rendered result to screen
		m_Direct3D->EndScene();

		//handle the input for this frame (otherwise we would get stuck in this state)
		result = HandleInput();
		if (!result)
			return false;
	}
	

	return result;
}


//functions depending on input
bool ApplicationClass::HandleInput()
{
	bool result;

	//handle the movement and rotation input only when in the game
	if (currentGameState == GAME)
	{
		//handle movement
		result = m_Input->IsUpPressed();
		m_Position->MoveForward(result);

		result = m_Input->IsDownPressed();
		m_Position->MoveBackward(result);

		//handle rotation
		result = m_Input->IsLeftPressed();
		m_Position->TurnLeft(result);

		result = m_Input->IsRightPressed();
		m_Position->TurnRight(result);

		result = m_Input->IsPgUpPressed();
		m_Position->LookUpward(result);

		result = m_Input->IsPgDownPressed();
		m_Position->LookDownward(result);

		//handle ump
		result = m_Input->IsSpacebarPressed();
		if (result)
			m_Position->Jump();
	}

	//creating a new dungeon
	result = m_Input->IsPPressed();

	//m_dungeonRecentlyCreated - makes sure that the dungein is only recreated once after pressing the button dow
	if (result && !m_dungeonRecentlyCreated)
	{
		//----- render intro screen to see something while loading
		currentGameState = INTRO;

		//reset scene (to black screen)
		m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

		//when rendering 2D, we can turn the Z buffer off
		m_Direct3D->TurnZBufferOff();

		//get world and ortho matrix
		D3DXMATRIX worldMatrix, orthoMatrix;
		m_Direct3D->GetWorldMatrix(worldMatrix);
		m_Direct3D->GetOrthoMatrix(orthoMatrix);

		//render text
		bool result = m_Text->Render(m_Direct3D->GetDeviceContext(), m_FontShader, worldMatrix, orthoMatrix, (int)currentGameState);
		if (!result)
			return false;

		//turn z buffer back on
		m_Direct3D->TurnZBufferOn();

		//end scene + presents rendered result to screen
		m_Direct3D->EndScene();


		//------ create new dungeon (same function as in initialization, but without creating new objects --> no extra function)
		m_DungeonGenerator->GenerateNewDungeon(DUNGEON_ROOMS, m_Direct3D);

		m_Terrain->Shutdown();
		result = m_Terrain->Initialize(m_Direct3D->GetDevice(), m_DungeonGenerator->GetDungeonData(), L"../Engine/data/dirt01.dds", L"../Engine/data/rock.dds");
		if(!result)
			return false;

		m_QuadTree->Shutdown();
		result = m_QuadTree->Initialize(m_Terrain, m_Direct3D->GetDevice(), m_DungeonGenerator->GetDungeonData());
		if(!result)
			return false;

		//reset position and camera with spawning coordinate from dungeon
		float posX, posY, posZ;
		m_DungeonGenerator->GetSpawningCoord(posX, posY, posZ);

		m_Position->SetPosition(posX, posY, posZ);
		m_Camera->SetPosition(posX, posY + m_Camera->GetYOffset(), posZ);
		m_Camera->SetRotation(0.0f, 0.0f, 0.0f);

		//reset points and go to game state
		m_pointsCollected = 0;
		currentGameState = GAME;

		//set the text for points back
		result = m_Text->SetPoints(m_pointsCollected, m_DungeonGenerator->GetTotalPointCount(), m_Direct3D->GetDeviceContext());
		if (!result)
			return false;

		m_dungeonRecentlyCreated = true;
	}
	//reset when releasing the key after regeneration
	else if (!result && m_dungeonRecentlyCreated)
		m_dungeonRecentlyCreated = false;

	return true;
}

//rendering all graphics
bool ApplicationClass::RenderGraphics()
{
	bool result = false;

	//render scene to texture
	result = RenderSceneToTexture();
	if(!result)
		return false;

	//render the post proccessing stuff
	result = RenderPostProcessing();
	if(!result)
		return false;

	//reset scene (to black screen)
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	//when rendering 2D, we can turn the Z buffer off
	m_Direct3D->TurnZBufferOff();

	//get world and ortho matrix
	D3DXMATRIX worldMatrix, orthoMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);


	//set render target to the plane
	m_FullScreenWindow->Render(m_Direct3D->GetDeviceContext());

	//render the last rendered texture via the basic texture shader
	result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_FullScreenWindow->GetIndexCount(),
		worldMatrix, m_baseViewMatrix, orthoMatrix, m_PostProcessedTexture->GetShaderResourceView());

	if(!result)
		return false;


	//use alpha blending to render text over texture without overwriting all of the previous one
	m_Direct3D->TurnOnAlphaBlending();

	//render text
	result = m_Text->Render(m_Direct3D->GetDeviceContext(), m_FontShader, worldMatrix, orthoMatrix, (int)currentGameState);
	if(!result)
		return false;
		
	//turn alpha blending off
	m_Direct3D->TurnOffAlphaBlending();


	//turn z buffer back on
	m_Direct3D->TurnZBufferOn();

	//end scene + presents rendered result to screen
	m_Direct3D->EndScene();

	return true;
}

//renders 3D scene to 2D texture
bool ApplicationClass::RenderSceneToTexture()
{
	//set render target to render to texture object
	m_RenderTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());

	//clear texture
	m_RenderTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();
	
	// Get the world, view and projection matrix from the camera and Direct3D
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	bool result = false;

	//render all collectibles (regardless of visibility)
	std::deque<DungeonGeneratorClass::CollectibleData*> *modelList = m_DungeonGenerator->GetDungeonData()->collectibles;
	DungeonGeneratorClass::CollectibleData* currentModel;
	float posX, posY, posZ;

	//go through complete list
	if(modelList)
	{
		//push 0 as end marker in the back
		modelList->push_back(0);
		currentModel = modelList->front();

		//take always the 1st element from the front and delete it from the lisst
		modelList->pop_front();
		m_Direct3D->GetWorldMatrix(worldMatrix);

		//as long as we do not reach the end
		while(currentModel)
		{
			//use moddels position to get appropriate world matrix
			currentModel->model->GetPosition(posX, posY, posZ);
			D3DXMatrixTranslation(&worldMatrix, posX, posY, posZ);

			//set shader parameters for each model individually (because world matrix changes each time)
			result = m_SphereShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
				m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetDirection(), currentModel->model->GetTexture());
			if (!result)
				return false;

			//render the moel via the sphere shader
			currentModel->model->Render(m_Direct3D->GetDeviceContext());
			m_SphereShader->RenderShader(m_Direct3D->GetDeviceContext(), currentModel->model->GetIndexCount());
			if(!result)
				return false;

			//push object back into list, get the next one and delete it from the list
			modelList->push_back(currentModel);
			currentModel = modelList->front();
			modelList->pop_front();
		}
	}

	//reset world matrix (otherwise we get weird results in later renderings)
	D3DXMatrixTranslation(&worldMatrix, 0.0f, 0.0f, 0.0f);
	
	//construct frustum based on view and projection matrix
	m_Frustum->ConstructFrustum(SCREEN_DEPTH, projectionMatrix, viewMatrix);

	//set terrain shader parameters before rendering the nodes
	result = m_TerrainShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetDirection(), m_Terrain->GetFloorTexture(), m_Terrain->GetWallTexture());

	if(!result)
		return false;

	//render all visible nodes (frustum object for culling, terrain shader for drawing
	m_QuadTree->Render(m_Frustum, m_Direct3D->GetDeviceContext(), m_TerrainShader);
	
	//reset render target to original back buffer and to stop rendering the texture
	m_Direct3D->SetBackBufferRenderTarget();

	//reset viewport back to original
	m_Direct3D->ResetViewport();

	return true;
}

//renders post processing effect onto 2D texture
bool ApplicationClass::RenderPostProcessing()
{
	//set post processing texture as target and clean it
	m_PostProcessedTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());
	m_PostProcessedTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	//get world matrix from Direct3D
	D3DXMATRIX worldMatrix;
	m_Direct3D->GetWorldMatrix(worldMatrix);

	//get ortho matrix from texture, since the dimensions differs to the one fromm Diret3D
	D3DXMATRIX orthoMatrix;
	m_PostProcessedTexture->GetOrthoMatrix(orthoMatrix);

	//prepare 2D rendering by turning the z buffer off
	m_Direct3D->TurnZBufferOff();

	//put the index and vertex buffer of the full screen into the graphics pipeline to draw on it
	m_FullScreenWindow->Render(m_Direct3D->GetDeviceContext());

	//render on full screen window (plane) using the post processing shader
	bool result = false;
	result = m_ColorFilterShader->Render(m_Direct3D->GetDeviceContext(), m_FullScreenWindow->GetIndexCount(),
		worldMatrix, m_baseViewMatrix, orthoMatrix, m_RenderTexture->GetShaderResourceView());

	if(!result)
		return false;

	//2D rendering has completed -> turn z buffer on
	m_Direct3D->TurnZBufferOn();

	//reset rendering target to stop rendering the texture
	m_Direct3D->SetBackBufferRenderTarget();

	//reset viewport back to original
	m_Direct3D->ResetViewport();

	return true;
}


//handles issues with call clipping / collision
void ApplicationClass::HandleWallCollision(float& posX, float& posY, float& posZ, float oldX, float oldY, float oldZ)
{
	//st collision radius and allowed upward(s) difference
	float collisionRadius = 0.0f;
	m_Position->GetCollisionRadius(collisionRadius);

	float allowedUpwardsDifference = 0.0f;
	m_Position->GetAllowedUpwardDifference(allowedUpwardsDifference);

	//check if position has clipped with wall - check this with the collision radius in all 4 corners
	bool clipWithWall = false;
	float tmpX = 0.0f, tmpZ = 0.0f, height = 0.0f;

	for(int i = 0; i < 4; i++)
	{
		tmpX = posX;
		tmpZ = posZ;
		height = 0.0f;

		//check specific corner, depending of iteration cycle
		if(i == 0)
			tmpX += collisionRadius;
		else if(i == 1)
			tmpX -= collisionRadius;
		else if(i == 2)
			tmpZ += collisionRadius;
		else
			tmpZ -= collisionRadius;

		//get height and check if it is higher  than allowed
		bool foundHeight = m_QuadTree->GetHeightAtPosition(tmpX, tmpZ, height);
		if(foundHeight)
		{
			if(height > (oldY + allowedUpwardsDifference))
				clipWithWall = true;
		}
		//outside of mesh = world wall
		else
			clipWithWall = true;

		//break out of loop if clipping with wall was confirmed
		if(clipWithWall)
			break;
	}

	//in case of collision with wall -> set position back to the old one
	if(clipWithWall)
	{
		posX = oldX;
		posY = oldY;
		posZ = oldZ;
	}
}