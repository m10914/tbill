/*
=============================================================================
Lights
Small useful library for different types of lights in DirectX

=============================================================================
*/



#ifndef _LIGHTS_H_
#define _LIGHTS_H_

#include <d3dx9.h>
#include <string>


class DXLight
{
protected:
	D3DXCOLOR	color;
	DWORD		shadowsize;
	DWORD		shadowtype;
	bool		needsredraw;
	bool		needsblur;

public:
	enum lighttype
	{
		Static = 0,
		Dynamic = 1
	};

	virtual ~DXLight();

	virtual void CreateShadowMap(LPDIRECT3DDEVICE9 device, DWORD type, DWORD size) = 0;

	inline const D3DXCOLOR& GetColor() const {
		return color;
	}
};



class DXDirectionalLight : public DXLight
{
private:
	LPDIRECT3DTEXTURE9		shadowmap;
	LPDIRECT3DTEXTURE9		blur;
	D3DXVECTOR4				direction;

public:
	DXDirectionalLight(const D3DXCOLOR& color, const D3DXVECTOR4& direction);
	~DXDirectionalLight();

	void DrawShadowMap(LPDIRECT3DDEVICE9 device, LPD3DXEFFECT effect, void (*drawcallback)(LPD3DXEFFECT));
	void BlurShadowMap(LPDIRECT3DDEVICE9 device, LPD3DXEFFECT effect);
	void CreateShadowMap(LPDIRECT3DDEVICE9 device, DWORD type, DWORD size);
	void GetViewProjMatrix(D3DXMATRIX& out, const D3DXVECTOR3& origin);

	inline D3DXVECTOR4& GetDirection() {
		return direction;
	}

	inline LPDIRECT3DTEXTURE9 GetShadowMap() {
		return shadowmap;
	}
};



class DXPointLight : public DXLight
{
private:
	LPDIRECT3DCUBETEXTURE9	shadowmap;
	LPDIRECT3DCUBETEXTURE9	blur;
	D3DXVECTOR3				position;
	float					radius;

public:
	DXPointLight(const D3DXCOLOR& color, const D3DXVECTOR3& pos, float radius);
	~DXPointLight();

	void DrawShadowMap(LPDIRECT3DDEVICE9 device, LPD3DXEFFECT effect, void (*drawcallback)(LPD3DXEFFECT));
	void BlurShadowMap(LPDIRECT3DDEVICE9 device, LPD3DXEFFECT effect);
	void CreateShadowMap(LPDIRECT3DDEVICE9 device, DWORD type, DWORD size);
	void GetScissorRect(RECT& out, const D3DXMATRIX& view, const D3DXMATRIX& proj, int w, int h) const;
	void GetViewProjMatrix(D3DXMATRIX& out, DWORD face);

	inline float GetRadius() const {
		return radius;
	}

	inline D3DXVECTOR3& GetPosition() {
		return position;
	}

	inline LPDIRECT3DCUBETEXTURE9 GetShadowMap() {
		return shadowmap;
	}
};


#endif
