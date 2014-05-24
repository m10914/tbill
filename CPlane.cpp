/*
=========================================================

=========================================================
*/

#include "DXUT.h"
#include "SDKmisc.h"
#include "CPlane.h"
#include "Formats.h"
using namespace std;



CPlanePrimitive::CPlanePrimitive()
{
}

CPlanePrimitive::~CPlanePrimitive()
{
}


HRESULT CPlanePrimitive::Create( IDirect3DDevice9* pd3dDevice, D3DXVECTOR3 position, D3DXVECTOR2 size, D3DXVECTOR2 tiling)
{
	HRESULT hr;

	this->position = position;
	this->size = size;
	this->tiling = tiling;

	//----------------------------
	// Create vertex buffer

	VERTEX m_Vertices[4] = {
		VERTEX(position + D3DXVECTOR3(-size.x/2, 0, -size.y/2), D3DXVECTOR3(0,1,0), D3DXVECTOR2(0, 0)),
		VERTEX(position + D3DXVECTOR3(-size.x/2, 0, size.y/2), D3DXVECTOR3(0,1,0), D3DXVECTOR2(0, tiling.y)),
		VERTEX(position + D3DXVECTOR3(size.x/2, 0, size.y/2), D3DXVECTOR3(0,1,0), D3DXVECTOR2(tiling.x, tiling.y)),
		VERTEX(position + D3DXVECTOR3(size.x/2, 0, -size.y/2), D3DXVECTOR3(0,1,0), D3DXVECTOR2(tiling.x, 0)) };

	if( FAILED(pd3dDevice->CreateVertexBuffer(4 * sizeof(VERTEX), D3DUSAGE_WRITEONLY,
		VERTEX::FVF, D3DPOOL_MANAGED, &m_VertexBuffer, NULL)))
	{
		return E_FAIL;
	}
	VERTEX* pVertex;
	V_RETURN( m_VertexBuffer->Lock(0, 4*sizeof(VERTEX), (void**)&pVertex, 0) );
	memcpy( pVertex, m_Vertices, 4 * sizeof( VERTEX ) );
	m_VertexBuffer->Unlock();


	//----------------------------
	// Create index buffer

	DWORD m_Indices[6] = { 0,1,2, 0,2,3 };

	if(FAILED(pd3dDevice->CreateIndexBuffer(6 * sizeof(DWORD), D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_IndexBuffer, NULL )))
	{
		return E_FAIL;
	}

	DWORD* pIndex;
	m_IndexBuffer->Lock(0, 6*sizeof(DWORD), (void**)&pIndex, 0);
	memcpy( pIndex, m_Indices, 6 * sizeof(DWORD));
	m_IndexBuffer->Unlock();

	return S_OK;
}


void CPlanePrimitive::Destroy()
{
	SAFE_RELEASE( m_VertexBuffer );
	SAFE_RELEASE( m_IndexBuffer );
}

void CPlanePrimitive::DrawPrimitive(IDirect3DDevice9* pd3dDevice)
{
	pd3dDevice->SetFVF(VERTEX::FVF);
	pd3dDevice->SetStreamSource(0, m_VertexBuffer, 0, sizeof(VERTEX));
	pd3dDevice->SetIndices(m_IndexBuffer);
	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
}