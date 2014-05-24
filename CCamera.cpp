/*
========================================================

========================================================
*/


#include "DXUT.h"
#include "SDKmisc.h"
#include "CCamera.h"
using namespace std;


/*
========================================================
Camera implementation

========================================================
*/

void CCamera::Update(float fElapsedSeconds)
{
	D3DXVECTOR3 vUp(0,1,0);

	//clamp theta
	if(angTheta < 0.1f) angTheta = 0.1f;
	else if(angTheta > D3DX_PI/2.f) angTheta = D3DX_PI/2.f;

	//clamp height
	if(distance < 20) distance = 20;
	else if(distance > 200) distance = 200;

	// calculate position
	eyePos.x = atPos.x + distance*cos(angPhi)*sin(angTheta);
	eyePos.z = atPos.z + distance*sin(angPhi)*sin(angTheta);
	eyePos.y = atPos.y + distance*cos(angTheta);

	//calculate matrix
	D3DXMatrixLookAtLH(&matView, &eyePos, &atPos, &vUp);
}


void CCamera::SetProjParams(FLOAT fFOV, FLOAT fAspect, FLOAT fNearPlane, FLOAT fFarPlane)
{
	//create perspective projection matrix
	D3DXMatrixPerspectiveFovLH(&matProj, fFOV, fAspect, fNearPlane, fFarPlane);
}


void CCamera::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_LBUTTONDOWN:
		{
			bLMBDown = true;

			oMousePosX = ( int )( short )LOWORD( lParam );
			oMousePosY = ( int )( short )HIWORD( lParam );
			return;
		}

		case WM_MOUSEMOVE:
		{         
			if(bLMBDown)
			{
				int iMouseX = ( int )( short )LOWORD( lParam );
				int iMouseY = ( int )( short )HIWORD( lParam );

				int dx = iMouseX - oMousePosX;
				int dy = iMouseY - oMousePosY;

				angPhi -= 0.01f * (float)dx;
				angTheta -= 0.01f * (float)dy;

				oMousePosX = iMouseX;
				oMousePosY = iMouseY;
			}
			return;
		}

		case WM_LBUTTONUP:
		{
			bLMBDown = false;
			return;
		}

		case WM_MOUSEWHEEL:
            // Update member var state
			distance -= ( short )HIWORD( wParam ) * 0.07f;
            break;
	}
}