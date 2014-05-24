/*
==============================================================
Special class for easy orbit camera.


==============================================================
*/


#ifndef _CCAMERA_H_
#define _CCAMERA_H_
#pragma once


class CCamera
{
public:
	CCamera(): distance(10), angPhi(0), angTheta(0), bLMBDown(false) {};
	~CCamera() {};

	void Update(float fElapsedTime);
	void HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	D3DXMATRIXA16* GetViewMatrix() { return &matView; };
	D3DXMATRIXA16* GetProjMatrix() { return &matProj; };
	D3DXVECTOR3* GetEyePt() { return &eyePos; } 
	void SetProjParams( FLOAT fFOV, FLOAT fAspect, FLOAT fNearPlane, FLOAT fFarPlane );

	void SetTarget(D3DXVECTOR3 target) { this->atPos = target; }
	void SetDistance(float distance) { this->distance = distance; }

private:

	float distance;
	float angPhi;
	float angTheta;

	bool bLMBDown;
	int oMousePosX, oMousePosY;

	D3DXMATRIXA16 matView;
	D3DXMATRIXA16 matProj;

	D3DXVECTOR3 eyePos;
	D3DXVECTOR3 atPos;
};



#endif