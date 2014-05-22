/*
=======================================================================

BilliardBalls

Based on MeshFromOBJ sample from DirectX SDK 2009.

(c) Damon Wall
=======================================================================
*/


#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include <fstream>
#pragma warning(disable: 4995)
#include "resource.h"

#include "Formats.h"
#include "CPlane.h"
#include "CSphere.h"
#pragma warning(default: 4995)

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 



//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*                  g_pFont = NULL;          // Font for drawing text
ID3DXSprite*                g_pTextSprite = NULL;    // Sprite for batching draw text calls
ID3DXEffect*                g_pEffect = NULL;        // D3DX effect interface
CModelViewerCamera          g_Camera;                // A model viewing camera
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTDialog                 g_HUD;                   // dialog for standard controls
CDXUTDialog                 g_SampleUI;              // dialog for sample specific controls
LPDIRECT3DDEVICE9			g_pd3dDevice = NULL;

WCHAR g_strFileSaveMessage[MAX_PATH] = {0}; // Text indicating file write success/failure

D3DXVECTOR2 screenSize;

// SCENE
CPlanePrimitive g_PlaneObj;
CSpherePrimitive g_SphereObj;
CPlanePrimitive g_OneWall;

IDirect3DTexture9* g_PlaneTexture;
IDirect3DTexture9* g_SphereTexture;

IDirect3DTexture9** g_SkyBoxTextures;
IDirect3DCubeTexture9* g_CubeMapTexture;



//--------------------------------------------------------------------------------------
// Effect parameter handles
//--------------------------------------------------------------------------------------

//matrices
D3DXHANDLE                  g_hWorld = NULL;
D3DXHANDLE                  g_hViewProjection = NULL;
D3DXHANDLE                  g_hView = NULL;
D3DXHANDLE                  g_hWorldView = NULL;

//vars
D3DXHANDLE					g_hEyePosition = NULL;
D3DXHANDLE                  g_hLightPosition = NULL;

//textures
D3DXHANDLE                  g_hDiffTexture = NULL;
D3DXHANDLE                  g_hSpecTexture = NULL;
D3DXHANDLE                  g_hBumpTexture = NULL;

//techniques
D3DXHANDLE					g_DefaultTechnique;
D3DXHANDLE					g_EnvMapTechnique;
D3DXHANDLE					g_PlainTechnique;


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_STATIC              -1
#define IDC_EDITBOX1			1



//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------


// actual application
void MyCreateScene();

//rendering

void MyRenderSceneIntoCubeMap();
void MyRenderText();
void MyRenderScene();

void MyRenderRoom();
void MyRenderPlane();
void MyRenderSphere();

void MyRenderOneWall();

void MyRenderFullscreen();


//fm
void MyFrameMove(float fElapsedTime);

//destroy
void MyDestroyScene();



// framework stuff
void InitApp();
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed,
                                  void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnLostDevice( void* pUserContext );
void CALLBACK OnDestroyDevice( void* pUserContext );






/*
==============================================================================================================================
A P P

Consists of basic 3d program pipeline: Create -> FrameMove -> Render -> Destroy
==============================================================================================================================
*/

void MyCreateScene()
{
	HRESULT hr;

	//create plane
	g_PlaneObj.Create(g_pd3dDevice, D3DXVECTOR3(0,-10,0), D3DXVECTOR2(150,150));
	g_SphereObj.Create(g_pd3dDevice, D3DXVECTOR3(0,0,0), 10, 100, 100);

	g_OneWall.Create(g_pd3dDevice, D3DXVECTOR3(0,0,0), D3DXVECTOR2(2,2));

	//create textures
	V( D3DXCreateTextureFromFile( g_pd3dDevice, L"Assets/ball_albedo.png", &g_SphereTexture ) );
	V( D3DXCreateTextureFromFile( g_pd3dDevice, L"Assets/pooltable.png", &g_PlaneTexture ) );	

	g_SkyBoxTextures = new LPDIRECT3DTEXTURE9[6];
	V( D3DXCreateTextureFromFile( g_pd3dDevice, L"Assets/skybox/front.jpg", &(g_SkyBoxTextures[0]) ) );
	V( D3DXCreateTextureFromFile( g_pd3dDevice, L"Assets/skybox/top.jpg", &(g_SkyBoxTextures[1]) ) );
	V( D3DXCreateTextureFromFile( g_pd3dDevice, L"Assets/skybox/back.jpg", &(g_SkyBoxTextures[2]) ) );
	V( D3DXCreateTextureFromFile( g_pd3dDevice, L"Assets/skybox/left.jpg", &(g_SkyBoxTextures[3]) ) );
	V( D3DXCreateTextureFromFile( g_pd3dDevice, L"Assets/skybox/right.jpg", &(g_SkyBoxTextures[4]) ) );
	V( D3DXCreateTextureFromFile( g_pd3dDevice, L"Assets/skybox/bottom.jpg", &(g_SkyBoxTextures[5]) ) );

	// Create the cube textures
	V( g_pd3dDevice->CreateCubeTexture( 512,
                                        1,
                                        D3DUSAGE_RENDERTARGET,
                                        D3DFMT_A16B16G16R16F,
                                        D3DPOOL_DEFAULT,
										&g_CubeMapTexture,
                                        NULL ) );
}



/*
======================================================================
Main rendering function

Launches all render in proper way
======================================================================
*/
void MyRenderScene()
{
	HRESULT hr;
	
	//----------------------------------
	// first render scene into cubemap texture

	// get matrices from camera, setup
	D3DXMATRIXA16 mWorld;
	D3DXMatrixIdentity(&mWorld);


	D3DXMATRIXA16 mView = *g_Camera.GetViewMatrix();
    D3DXMATRIXA16 mViewProjection = mView * *g_Camera.GetProjMatrix();
	D3DXMATRIXA16 mWorldView = mWorld * mView;

	//setup some common variables
	V( g_pEffect->SetMatrix( g_hViewProjection, &mViewProjection ) );
	V( g_pEffect->SetMatrix( g_hView, &mView ) );
	V( g_pEffect->SetMatrix( g_hWorldView, &mWorldView));
  
	V( g_pEffect->SetValue( g_hEyePosition, g_Camera.GetEyePt(), sizeof( D3DXVECTOR3 ) ) );
	V( g_pEffect->SetValue( g_hLightPosition, D3DXVECTOR3(100,100,-100), sizeof( D3DXVECTOR3 ) ) );
	

	//---------------------------------
	// Render!

	// room
	V( g_pEffect->SetTechnique( g_PlainTechnique ) );
	MyRenderRoom();

	// render ball
	V( g_pEffect->SetTechnique( g_EnvMapTechnique ) );
	MyRenderSphere();

	// render floor
	V( g_pEffect->SetTechnique( g_PlainTechnique ) );
	MyRenderPlane();


	//TODO: remove
	MyRenderFullscreen();

	return;
}

void MyRenderPlane()
{
	HRESULT hr;
	UINT cPasses, iPass;
	D3DXMATRIXA16 mWorld;
	D3DXMatrixIdentity(&mWorld);

	DWORD oldVal;
	g_pd3dDevice->GetRenderState(D3DRS_CULLMODE, &oldVal);
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	V( g_pEffect->SetTexture( g_hDiffTexture, g_PlaneTexture));
	V( g_pEffect->SetMatrix( g_hWorld, &mWorld));

	V( g_pEffect->Begin( &cPasses, 0 ) );
	V( g_pEffect->BeginPass( 0 ) );
	
	g_PlaneObj.DrawPrimitive( g_pd3dDevice );

	V( g_pEffect->EndPass() );	
	V( g_pEffect->End() );

	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, oldVal);
}

void MyRenderSphere()
{
	HRESULT hr;
	UINT cPasses, iPass;
	D3DXMATRIXA16 mWorld;
	D3DXMatrixIdentity(&mWorld);

	V( g_pEffect->SetTexture( g_hDiffTexture, g_CubeMapTexture));
	V( g_pEffect->SetTexture( g_hBumpTexture, g_SphereTexture));
	V( g_pEffect->SetMatrix( g_hWorld, &mWorld));

	V( g_pEffect->Begin( &cPasses, 0 ) );
	V( g_pEffect->BeginPass( 0 ) );
	
	g_SphereObj.DrawPrimitive( g_pd3dDevice );

	V( g_pEffect->EndPass() );	
	V( g_pEffect->End() );

}


void MyRenderRoom()
{
	HRESULT hr;
	D3DXMATRIXA16 trans, rot, scale, temp;
	D3DXMATRIXA16 worldMatrix;
	float size = 200;


	D3DXMatrixScaling(&scale, size, size, size);
	D3DXMatrixTranslation(&trans, 0, -size, 0);

	//1st wall
	D3DXMatrixRotationY(&temp, -D3DX_PI/2.f);
	worldMatrix = scale * trans * temp;
	V( g_pEffect->SetMatrix( g_hWorld, &worldMatrix ) );
	V( g_pEffect->SetTexture( g_hDiffTexture, g_SkyBoxTextures[5]));

	MyRenderOneWall();

	//2nd wall
	D3DXMatrixRotationX(&temp, D3DX_PI/2.f);
	D3DXMatrixRotationZ(&rot, D3DX_PI/2.f);
	worldMatrix = scale * trans * rot * temp;
	V( g_pEffect->SetMatrix( g_hWorld, &worldMatrix ) );
	V( g_pEffect->SetTexture( g_hDiffTexture, g_SkyBoxTextures[0]));

	MyRenderOneWall();

	//3rd wall
	D3DXMatrixRotationZ(&rot, D3DX_PI/2.f);
	worldMatrix *= rot;
	V( g_pEffect->SetMatrix( g_hWorld, &worldMatrix ) );
	V( g_pEffect->SetTexture( g_hDiffTexture, g_SkyBoxTextures[1]));

	MyRenderOneWall();

	//4th wall
	D3DXMatrixRotationZ(&rot, D3DX_PI/2.f);
	D3DXMatrixRotationX(&temp, D3DX_PI);
	worldMatrix = worldMatrix * rot * temp;
	V( g_pEffect->SetMatrix( g_hWorld, &worldMatrix ) );
	V( g_pEffect->SetTexture( g_hDiffTexture, g_SkyBoxTextures[2]));

	MyRenderOneWall();

	//5th wall
	D3DXMatrixRotationX(&rot, D3DX_PI/2.f);
	worldMatrix = scale * trans * rot;
	V( g_pEffect->SetMatrix( g_hWorld, &worldMatrix ) );
	V( g_pEffect->SetTexture( g_hDiffTexture, g_SkyBoxTextures[3]));

	MyRenderOneWall();

	//6th wall
	D3DXMatrixRotationX(&rot, D3DX_PI);
	D3DXMatrixRotationZ(&temp, D3DX_PI);
	worldMatrix = worldMatrix * rot * temp;
	V( g_pEffect->SetMatrix( g_hWorld, &worldMatrix ) );
	V( g_pEffect->SetTexture( g_hDiffTexture, g_SkyBoxTextures[4]));

	MyRenderOneWall();

	return;
}

void MyRenderOneWall()
{
	HRESULT hr;

	g_pEffect->Begin(NULL, 0);
	g_pEffect->BeginPass(0);
	
	// draw scene objects
	g_OneWall.DrawPrimitive( g_pd3dDevice );

	g_pEffect->EndPass();	
	g_pEffect->End();
}



void MyRenderSceneIntoCubeMap()
{
    HRESULT hr;
	UINT cPasses, iPass;

    // The projection matrix has a FOV of 90 degrees and asp ratio of 1
	D3DXMATRIXA16 mWorld;
	D3DXMatrixIdentity(&mWorld);

    D3DXMATRIXA16 mProj;
    D3DXMatrixPerspectiveFovLH( &mProj, D3DX_PI * 0.5f, 1.0f, 0.01f, 100000.0f );

    D3DXMATRIXA16 mViewDir( *g_Camera.GetViewMatrix() );
    mViewDir._41 = mViewDir._42 = mViewDir._43 = 0.0f;

    LPDIRECT3DSURFACE9 pRTOld = NULL;
	V( g_pd3dDevice->GetRenderTarget( 0, &pRTOld ) );


    for( int nFace = 0; nFace < 6; ++nFace )
    {
        LPDIRECT3DSURFACE9 pSurf;

		V( g_CubeMapTexture->GetCubeMapSurface( ( D3DCUBEMAP_FACES )nFace, 0, &pSurf ) );
        V( g_pd3dDevice->SetRenderTarget( 0, pSurf ) );
        SAFE_RELEASE( pSurf );

        D3DXMATRIXA16 mView = DXUTGetCubeMapViewMatrix( nFace );
        D3DXMatrixMultiply( &mView, &mViewDir, &mView );

		V( g_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L ) );		

        // Begin the scene
        if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
        {
			//render everything except sphere
			D3DXMATRIXA16 mViewProjection = mView * mProj;

			V( g_pEffect->SetMatrix( g_hViewProjection, &mViewProjection ) );
			V( g_pEffect->SetMatrix( g_hView, &mView ) );
			V( g_pEffect->SetMatrix( g_hWorld, &mWorld ) );

			//render
			V( g_pEffect->SetTechnique( g_PlainTechnique ) );
			MyRenderRoom();

			MyRenderPlane();
			

            // End the scene.
            g_pd3dDevice->EndScene();
        }
    }

    V( g_pd3dDevice->SetRenderTarget( 0, pRTOld ) );
    SAFE_RELEASE( pRTOld );
}


void MyRenderFullscreen()
{
	float quadvertices[36] =
	{
							-0.5f,						-0.5f, 0, 1,	0, 0,
		(float)screenSize.x - 0.5f,						-0.5f, 0, 1,	1, 0,
							-0.5f, (float)screenSize.y - 0.5f, 0, 1,	0, 1,

							-0.5f, (float)screenSize.y - 0.5f, 0, 1,	0, 1,
		(float)screenSize.x - 0.5f,						-0.5f, 0, 1,	1, 0,
		(float)screenSize.x - 0.5f, (float)screenSize.y - 0.5f, 0, 1,	1, 1
	};

	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0);
	g_pd3dDevice->SetFVF(D3DFVF_XYZRHW|D3DFVF_TEX1);
	g_pd3dDevice->SetTexture(0, g_PlaneTexture);
	g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, quadvertices, 6 * sizeof(float));

	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
}



void MyRenderText()
{
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

    // Output statistics
	/*
	txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
	WCHAR* str = new WCHAR[256];
	swprintf(str, L"Areas: %f - %f", g_MeshLoader.MinArea, g_MeshLoader.MaxArea);
	txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    txtHelper.DrawTextLine( str );
	delete[] str;

    txtHelper.End();
	*/
}


void MyFrameMove(float fElapsedTime)
{
	g_Camera.FrameMove( fElapsedTime );
}


void MyDestroyScene()
{
	g_PlaneObj.Destroy();
	g_SphereObj.Destroy();
	g_OneWall.Destroy();

	SAFE_RELEASE( g_PlaneTexture );
	SAFE_RELEASE( g_SphereTexture );

	for(int i=0; i < 6; i++)
		SAFE_RELEASE(g_SkyBoxTextures[i]);
	delete[] g_SkyBoxTextures;

	SAFE_RELEASE(g_CubeMapTexture);
}










/*
==============================================================================================================================
F R A M E W O R K    P A R T

Consists of standart code, which is reusable for any simple DX demo
==============================================================================================================================
*/


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Set the callback functions. These functions allow DXUT to notify
    // the application about device changes, user input, and windows messages.  The 
    // callbacks are optional so you need only set callbacks for events you're interested 
    // in. However, if you don't handle the device reset/lost callbacks then the sample 
    // framework won't be able to reset your device since the application must first 
    // release all device resources before resetting.  Likewise, if you don't handle the 
    // device created/destroyed callbacks then DXUT won't be able to 
    // recreate your device resources.
    DXUTSetCallbackD3D9DeviceAcceptable( IsDeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnFrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp();

    // Initialize DXUT and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true ); // Parse the command line and show msgboxes
    DXUTSetHotkeyHandling( true, true, true );  // handle the defaul hotkeys
    DXUTCreateWindow( L"BilliardBalls" );
    DXUTCreateDevice( true, 1280, 768 );

    // Pass control to DXUT for handling the message pump and 
    // dispatching render calls. DXUT will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.
    DXUTMainLoop();

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here.

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

	
	g_SampleUI.AddEditBox( IDC_EDITBOX1, L"", 5, 20,
                           100, 32 );


    g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;

    // Title font for comboboxes
    g_SampleUI.SetFont( 1, L"Arial", 14, FW_BOLD );
    CDXUTElement* pElement = g_SampleUI.GetDefaultElement( DXUT_CONTROL_STATIC, 0 );
    if( pElement )
    {
        pElement->iFont = 1;
        pElement->dwTextFormat = DT_LEFT | DT_BOTTOM;
    }

}





//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // No fallback defined by this app, so reject any device that 
    // doesn't support at least ps2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
        return false;

    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// DXUT will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver );

    HRESULT hr;
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    D3DCAPS9 caps;

    V( pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal,
                            pDeviceSettings->d3d9.DeviceType,
                            &caps ) );

    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
    // then switch to SWVP.
    if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
        caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
    {
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

    // Debugging vertex shaders requires either REF or software vertex processing 
    // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
    if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif

    // Enable anti-aliasing for HAL devices which support it
    CD3D9Enumeration* pEnum = DXUTGetD3D9Enumeration();
    CD3D9EnumDeviceSettingsCombo* pCombo = pEnum->GetDeviceSettingsCombo( &pDeviceSettings->d3d9 );

    if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_HAL &&
        pCombo->multiSampleTypeList.Contains( D3DMULTISAMPLE_4_SAMPLES ) )
    {
        pDeviceSettings->d3d9.pp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
        pDeviceSettings->d3d9.pp.MultiSampleQuality = 0;
    }
    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
    }

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext )
{
    HRESULT hr;
    WCHAR str[MAX_PATH];

	g_pd3dDevice = pd3dDevice;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );

    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                              L"Arial", &g_pFont ) );


	MyCreateScene();

	// load stuff
	//WCHAR areaStr[256];
	//swprintf(areaStr, L"%f", g_MeshLoader.CurrentArea);
	g_SampleUI.GetEditBox( IDC_EDITBOX1 )->SetText( L"REMOVE THIS!" );


	// create effect
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"effect.fx" ) );

	ID3DXBuffer* pBuff = NULL;
    hr = D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags,
                                        NULL, &g_pEffect, &pBuff );
	if(hr == E_FAIL)
	{
		std::wofstream file("shader_compile_errors.txt");
		file << ((char*)pBuff->GetBufferPointer());
		file.close();
		return E_FAIL;
		// look for shader errors in file
	}


    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye( 12.2f, 10.5f, 0.0f );
    D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
    g_Camera.SetViewParams( &vecEye, &vecAt );

    return S_OK;
}





//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice,
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );
    if( g_pEffect )
        V_RETURN( g_pEffect->OnResetDevice() );

    // Store the correct technique handles for each material
	g_DefaultTechnique = g_pEffect->GetTechniqueByName( "Default" );
	g_EnvMapTechnique = g_pEffect->GetTechniqueByName( "EnvMap" );
	g_PlainTechnique = g_pEffect->GetTechniqueByName( "Plain" );

	g_hDiffTexture = g_pEffect->GetParameterBySemantic( 0, "Texture" );
	g_hBumpTexture = g_pEffect->GetParameterBySemantic( 0, "BumpTexture" );
	g_hSpecTexture = g_pEffect->GetParameterBySemantic( 0, "SpecTexture" );
	g_hEyePosition = g_pEffect->GetParameterBySemantic( 0, "EyePosition" );
    g_hLightPosition = g_pEffect->GetParameterBySemantic( 0, "LightPosition" );
    g_hWorld = g_pEffect->GetParameterBySemantic( 0, "World" );
    g_hViewProjection = g_pEffect->GetParameterBySemantic( 0, "ViewProjection" );
    g_hView = g_pEffect->GetParameterBySemantic( 0, "View" );
	g_hWorldView = g_pEffect->GetParameterBySemantic( 0, "WorldView" );


    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	screenSize = D3DXVECTOR2( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );


    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_HUD.Refresh();

    g_SampleUI.SetLocation( 5, 5 );
	g_SampleUI.SetSize( pBackBufferSurfaceDesc->Width-10, pBackBufferSurfaceDesc->Height-10 );
    g_SampleUI.Refresh();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	MyFrameMove(fElapsedTime);
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, DXUT will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

    HRESULT hr;

	//first - cubemap
	MyRenderSceneIntoCubeMap();

	//D3DXSaveTextureToFile(L"out.jpg", D3DXIFF_JPG, g_CubeMapTexture, NULL);


    // Clear the render target and the zbuffer
	//D3DCOLOR_ARGB( 0, 141, 153, 191 )
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 0, 0, 0 ), 1.0f, 0 ) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {

		// custom render functions
		MyRenderScene();
        MyRenderText();

		// built-in renders
        V( g_HUD.OnRender( fElapsedTime ) );
        V( g_SampleUI.OnRender( fElapsedTime ) );

        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	WCHAR wszOutput[1024];

    switch( nControlID )
    {
		case IDC_EDITBOX1:
			switch( nEvent )
            {
                case EVENT_EDITBOX_STRING:
                {
					auto res = ( ( CDXUTEditBox* )pControl )->GetText();

					//g_MeshLoader.SetCurrentArea(_wtof(res));

                    break;
                }
            }
            break;
    }
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();
    if( g_pFont )
        g_pFont->OnLostDevice();
    if( g_pEffect )
        g_pEffect->OnLostDevice();

    SAFE_RELEASE( g_pTextSprite );
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();
    SAFE_RELEASE( g_pEffect );
    SAFE_RELEASE( g_pFont );

	MyDestroyScene();
}



