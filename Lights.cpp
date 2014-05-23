
#include "Lights.h"



LPDIRECT3DVERTEXDECLARATION9	blurdeclforpoint = 0;
LPDIRECT3DVERTEXDECLARATION9	blurdeclforpointfordirectional = 0;

static const D3DXVECTOR3 DXCubeForward[6] =
{
	D3DXVECTOR3(1, 0, 0),
	D3DXVECTOR3(-1, 0, 0),
	D3DXVECTOR3(0, 1, 0),
	D3DXVECTOR3(0, -1, 0),
	D3DXVECTOR3(0, 0, 1),
	D3DXVECTOR3(0, 0, -1),
};

static const D3DXVECTOR3 DXCubeUp[6] =
{
	D3DXVECTOR3(0, 1, 0),
	D3DXVECTOR3(0, 1, 0),
	D3DXVECTOR3(0, 0, -1),
	D3DXVECTOR3(0, 0, 1),
	D3DXVECTOR3(0, 1, 0),
	D3DXVECTOR3(0, 1, 0),
};





/*
=============================================================================
DXLight

implementation
=============================================================================
*/
DXLight::~DXLight()
{
}

void DXLight::CreateShadowMap(LPDIRECT3DDEVICE9 device, DWORD type, DWORD size)
{
	shadowtype = type;
	shadowsize = size;
}





/*
=============================================================================
DXDirectionalLight

implementation
=============================================================================
*/
DXDirectionalLight::DXDirectionalLight(const D3DXCOLOR& color, const D3DXVECTOR4& direction)
{
	this->color = color;
	this->direction = direction;

	shadowmap = 0;
	blur = 0;
	needsredraw = true;
	needsblur = true;
}

DXDirectionalLight::~DXDirectionalLight()
{
	if( shadowmap )
	{
		shadowmap->Release();
		shadowmap = 0;
	}

	if( blur )
	{
		blur->Release();
		blur = 0;
	}

	if( blurdeclforpointfordirectional )
	{
		if( 0 == blurdeclforpointfordirectional->Release() )
			blurdeclforpointfordirectional = 0;
	}
}

void DXDirectionalLight::CreateShadowMap(LPDIRECT3DDEVICE9 device, DWORD type, DWORD size)
{
	HRESULT hr;

	DXLight::CreateShadowMap(device, type, size);
	hr = device->CreateTexture(size, size, 1, D3DUSAGE_RENDERTARGET, D3DFMT_G32R32F, D3DPOOL_DEFAULT, &shadowmap, NULL);

	if( SUCCEEDED(hr) )
		hr = device->CreateTexture(size, size, 1, D3DUSAGE_RENDERTARGET, D3DFMT_G32R32F, D3DPOOL_DEFAULT, &blur, NULL);

	if( FAILED(hr) )
	{
		if( shadowmap )
			shadowmap->Release();

		shadowmap = blur = NULL;
	}

	if( !blurdeclforpointfordirectional )
	{
		D3DVERTEXELEMENT9 elem[] =
		{
			{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
			{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			D3DDECL_END()
		};

		device->CreateVertexDeclaration(elem, &blurdeclforpointfordirectional);
	}
	else
		blurdeclforpointfordirectional->AddRef();
}

void DXDirectionalLight::GetViewProjMatrix(D3DXMATRIX& out, const D3DXVECTOR3& origin)
{
	D3DXVECTOR3 look;
	D3DXVECTOR3 up(0, 1, 0);
	D3DXMATRIX proj;

	look.x = origin.x + direction.w * direction.x;
	look.y = origin.y + direction.w * direction.y;
	look.z = origin.z + direction.w * direction.z;

	D3DXMatrixLookAtLH(&out, &look, &origin, &up);
	D3DXMatrixOrthoLH(&proj, 50, 50, 0.1f, 400.f);
	D3DXMatrixMultiply(&out, &out, &proj);
}

void DXDirectionalLight::BlurShadowMap(LPDIRECT3DDEVICE9 device, LPD3DXEFFECT effect)
{
	if( !shadowmap || !blur || !blurdeclforpointfordirectional || !needsblur )
		return;

	float blurvertices[36] =
	{
		-0.5f,						-0.5f,						0, 1,	0, 0,
		(float)shadowsize - 0.5f,	-0.5f,						0, 1,	1, 0,
		-0.5f,						(float)shadowsize - 0.5f,	0, 1,	0, 1,
	
		-0.5f,						(float)shadowsize - 0.5f,	0, 1,	0, 1,
		(float)shadowsize - 0.5f,	-0.5f,						0, 1,	1, 0,
		(float)shadowsize - 0.5f,	(float)shadowsize - 0.5f,	0, 1,	1, 1
	};

	LPDIRECT3DSURFACE9	surface = NULL;
	D3DXVECTOR4			texelsize(1.0f / shadowsize, 0, 0, 0);
	UINT				stride = 6 * sizeof(float);

	device->SetVertexDeclaration(blurdeclforpointfordirectional);

	// x
	blur->GetSurfaceLevel(0, &surface);

	effect->SetVector("texelSize", &texelsize);
	effect->CommitChanges();

	device->SetRenderTarget(0, surface);
	device->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0);

	device->SetTexture(0, shadowmap);
	device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, blurvertices, stride);

	surface->Release();
	std::swap(texelsize.x, texelsize.y);

	// y
	shadowmap->GetSurfaceLevel(0, &surface);

	effect->SetVector("texelSize", &texelsize);
	effect->CommitChanges();

	device->SetRenderTarget(0, surface);
	device->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0);

	device->SetTexture(0, blur);
	device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, blurvertices, stride);

	surface->Release();

	if( shadowtype == Static )
		needsblur = false;
}

void DXDirectionalLight::DrawShadowMap(LPDIRECT3DDEVICE9 device, LPD3DXEFFECT effect, void (*drawcallback)(LPD3DXEFFECT))
{
	if( !shadowmap || !needsredraw )
		return;

	LPDIRECT3DSURFACE9 surface = NULL;
	D3DXMATRIX vp;

	GetViewProjMatrix(vp, D3DXVECTOR3(0, 0, 0));

	effect->SetVector("lightPos", &direction);
	effect->SetMatrix("matViewProj", &vp);

	shadowmap->GetSurfaceLevel(0, &surface);

	device->SetRenderTarget(0, surface);
	device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);

	drawcallback(effect);
	surface->Release();

	if( shadowtype == Static )
		needsredraw = false;
}




/*
=============================================================================
DXPointLight

implementation
=============================================================================
*/
DXPointLight::DXPointLight(const D3DXCOLOR& color, const D3DXVECTOR3& pos, float radius)
{
	this->color = color;
	this->position = pos;
	this->radius = radius;

	shadowmap = 0;
	blur = 0;
	needsredraw = true;
	needsblur = true;
}

DXPointLight::~DXPointLight()
{
	if( shadowmap )
	{
		shadowmap->Release();
		shadowmap = 0;
	}

	if( blur )
	{
		blur->Release();
		blur = 0;
	}

	if( blurdeclforpoint )
	{
		if( 0 == blurdeclforpoint->Release() )
			blurdeclforpoint = 0;
	}
}

void DXPointLight::CreateShadowMap(LPDIRECT3DDEVICE9 device, DWORD type, DWORD size)
{
	HRESULT hr;

	DXLight::CreateShadowMap(device, type, size);
	hr = device->CreateCubeTexture(size, 1, D3DUSAGE_RENDERTARGET, D3DFMT_G32R32F, D3DPOOL_DEFAULT, &shadowmap, NULL);

	if( SUCCEEDED(hr) )
		hr = device->CreateCubeTexture(size, 1, D3DUSAGE_RENDERTARGET, D3DFMT_G32R32F, D3DPOOL_DEFAULT, &blur, NULL);

	if( FAILED(hr) )
	{
		if( shadowmap )
			shadowmap->Release();

		shadowmap = blur = NULL;
	}

	if( !blurdeclforpoint )
	{
		D3DVERTEXELEMENT9 elem[] =
		{
			{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			D3DDECL_END()
		};

		device->CreateVertexDeclaration(elem, &blurdeclforpoint);
	}
	else
		blurdeclforpoint->AddRef();
}

void DXPointLight::GetScissorRect(RECT& out, const D3DXMATRIX& view, const D3DXMATRIX& proj, int w, int h) const
{
	// NOTE: should also consider viewport origin

	D3DXVECTOR4 Q;
	D3DXVECTOR3 L, P1, P2;
	float u, v;

	out.left	= 0;
	out.right	= w;
	out.top		= 0;
	out.bottom	= h;

	D3DXVec3TransformCoord(&L, &position, &view);

	float d = 4 * (radius * radius * L.x * L.x - (L.x * L.x + L.z * L.z) * (radius * radius - L.z * L.z));

	if( d >= 0 && L.z >= 0.0f )
	{
		float a1 = (radius * L.x + sqrtf(d * 0.25f)) / (L.x * L.x + L.z * L.z);
		float a2 = (radius * L.x - sqrtf(d * 0.25f)) / (L.x * L.x + L.z * L.z);
		float c1 = (radius - a1 * L.x) / L.z;
		float c2 = (radius - a2 * L.x) / L.z;

		P1.y = 0;
		P2.y = 0;

		P1.x = L.x - a1 * radius;
		P1.z = L.z - c1 * radius;

		P2.x = L.x - a2 * radius;
		P2.z = L.z - c2 * radius;

		if( P1.z > 0 && P2.z > 0 )
		{
			D3DXVec3Transform(&Q, &P1, &proj);

			Q /= Q.w;
			u = (Q.x * 0.5f + 0.5f) * w;

			D3DXVec3Transform(&Q, &P2, &proj);

			Q /= Q.w;
			v = (Q.x * 0.5f + 0.5f) * w;

			if( P2.x < L.x )
			{
				out.left = (LONG)max(v, 0);
				out.right = (LONG)min(u, w);
			}
			else
			{
				out.left = (LONG)max(u, 0);
				out.right = (LONG)min(v, w);
			}
		}
	}

	d = 4 * (radius * radius * L.y * L.y - (L.y * L.y + L.z * L.z) * (radius * radius - L.z * L.z));

	if( d >= 0 && L.z >= 0.0f )
	{
		float b1 = (radius * L.y + sqrtf(d * 0.25f)) / (L.y * L.y + L.z * L.z);
		float b2 = (radius * L.y - sqrtf(d * 0.25f)) / (L.y * L.y + L.z * L.z);
		float c1 = (radius - b1 * L.y) / L.z;
		float c2 = (radius - b2 * L.y) / L.z;

		P1.x = 0;
		P2.x = 0;

		P1.y = L.y - b1 * radius;
		P1.z = L.z - c1 * radius;

		P2.y = L.y - b2 * radius;
		P2.z = L.z - c2 * radius;

		if( P1.z > 0 && P2.z > 0 )
		{
			D3DXVec3Transform(&Q, &P1, &proj);

			Q /= Q.w;
			u = (Q.y * -0.5f + 0.5f) * h;

			D3DXVec3Transform(&Q, &P2, &proj);

			Q /= Q.w;
			v = (Q.y * -0.5f + 0.5f) * h;

			if( P2.y < L.y )
			{
				out.top = (LONG)max(u, 0);
				out.bottom = (LONG)min(v, h);
			}
			else
			{
				out.top = (LONG)max(v, 0);
				out.bottom = (LONG)min(u, h);
			}
		}
	}
}

void DXPointLight::GetViewProjMatrix(D3DXMATRIX& out, DWORD face)
{
	D3DXMATRIX proj;

	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 2, 1, 0.1f, radius);
	D3DXMatrixLookAtLH(&out, &position, &(position + DXCubeForward[face]), &DXCubeUp[face]);
	D3DXMatrixMultiply(&out, &out, &proj);
}

void DXPointLight::BlurShadowMap(LPDIRECT3DDEVICE9 device, LPD3DXEFFECT effect)
{
}

void DXPointLight::DrawShadowMap(LPDIRECT3DDEVICE9 device, LPD3DXEFFECT effect, void (*drawcallback)(LPD3DXEFFECT))
{
	if( !shadowmap || !needsredraw )
		return;

	LPDIRECT3DSURFACE9 surface = NULL;
	D3DXMATRIX vp;

	effect->SetVector("lightPos", (D3DXVECTOR4*)&position);

	for( int j = 0; j < 6; ++j )
	{
		GetViewProjMatrix(vp, j);
		effect->SetMatrix("matViewProj", &vp);

		shadowmap->GetCubeMapSurface((D3DCUBEMAP_FACES)j, 0, &surface);

		device->SetRenderTarget(0, surface);
		device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);

		drawcallback(effect);
		surface->Release();
	}

	if( shadowtype == Static )
		needsredraw = false;
}
