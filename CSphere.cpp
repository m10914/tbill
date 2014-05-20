/*
=========================================================

=========================================================
*/

#include "DXUT.h"
#include "SDKmisc.h"
#include "CSphere.h"
#include "Formats.h"
using namespace std;



CSpherePrimitive::CSpherePrimitive()
{
}

CSpherePrimitive::~CSpherePrimitive()
{
}


HRESULT CSpherePrimitive::Create( IDirect3DDevice9* pd3dDevice, D3DXVECTOR3 position, float radius, int segmentsWidth, int segmentsHeight)
{
	HRESULT hr;
	int i,j;

	this->position = position;
	this->radius = radius;
	this->segmentX = segmentsWidth;
	this->segmentY = segmentsHeight;

	CGrowableArray<VERTEX> m_Vertices;
	CGrowableArray<DWORD> m_Indices;

	//create topology
	float anglePhi = 0;
	float angleTheta = 0;
	float stepx = D3DX_PI*2/(float)segmentX;
	float stepy = D3DX_PI/(float)(segmentY-1);

	//top vertex
	m_Vertices.Add(VERTEX(position + D3DXVECTOR3(0,radius,0), D3DXVECTOR3(0,1,0), D3DXVECTOR2(0,0)));

	for(i = 0; i < segmentX; i++)
	{
		for(j = 1; j < segmentY-1; j++) //all except top and bottom
		{
			anglePhi = i*stepx;
			angleTheta = j*stepy;
			D3DXVECTOR3 vec = D3DXVECTOR3(radius*cos(anglePhi)*sin(angleTheta), radius*cos(angleTheta), radius*sin(anglePhi)*sin(angleTheta));
			D3DXVECTOR3 norm;
			D3DXVec3Normalize(&norm, &vec);
			m_Vertices.Add(VERTEX(
				position + vec,
				norm,
				D3DXVECTOR2(0,0)));
		}
	}

	//bottom vertex
	m_Vertices.Add(VERTEX(position + D3DXVECTOR3(0,-radius,0), D3DXVECTOR3(0,-1,0), D3DXVECTOR2(1,1)));

	NumOfVertices = m_Vertices.GetSize();
	NumOfFaces = segmentX * 2 + (segmentY-2) * 2 * segmentX;
	NumOfIndices = NumOfFaces*3;

	// construct top and bottom
	int stride = segmentY-2;
	for(i = 0; i < segmentX; i++)
	{
		//top
		m_Indices.Add(0);
		m_Indices.Add(stride*i + 1);
		if(i == segmentX-1) m_Indices.Add(1);
		else m_Indices.Add(stride*(i+1) + 1);

		//bottom
		m_Indices.Add(NumOfVertices-1);
		m_Indices.Add(stride*i + stride);
		if(i == segmentX-1) m_Indices.Add(stride);
		else m_Indices.Add(stride*(i+1) + stride);
	}
	// construct what's left
	for(i = 0; i < segmentX; i++)
	{
		for(j = 0; j < stride-1; j++)
		{
			int nexti = i == segmentX-1 ? 0 : i+1;

			m_Indices.Add(1 + stride*i + j);
			m_Indices.Add(1 + stride*i + j + 1);
			m_Indices.Add(1 + stride*nexti + j);

			m_Indices.Add(1 + stride*nexti + j);
			m_Indices.Add(1 + stride*i + j + 1);
			m_Indices.Add(1 + stride*nexti + j + 1);
		}
	}


	//----------------------------
	// Create vertex buffer

	if( FAILED(pd3dDevice->CreateVertexBuffer(NumOfVertices * sizeof(VERTEX), D3DUSAGE_WRITEONLY,
		VERTEX::FVF, D3DPOOL_MANAGED, &m_VertexBuffer, NULL)))
	{
		return E_FAIL;
	}
	VERTEX* pVertex;
	V_RETURN( m_VertexBuffer->Lock(0, NumOfVertices*sizeof(VERTEX), (void**)&pVertex, 0) );
	memcpy( pVertex, m_Vertices.GetData(), NumOfVertices * sizeof( VERTEX ) );
	m_VertexBuffer->Unlock();


	//----------------------------
	// Create index buffer

	if(FAILED(pd3dDevice->CreateIndexBuffer(NumOfIndices * sizeof(DWORD), D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_IndexBuffer, NULL )))
	{
		return E_FAIL;
	}

	DWORD* pIndex;
	m_IndexBuffer->Lock(0, NumOfIndices*sizeof(DWORD), (void**)&pIndex, 0);
	memcpy( pIndex, m_Indices.GetData(), NumOfIndices * sizeof(DWORD));
	m_IndexBuffer->Unlock();

	return S_OK;
}


void CSpherePrimitive::Destroy()
{
	SAFE_RELEASE( m_VertexBuffer );
	SAFE_RELEASE( m_IndexBuffer );
}

void CSpherePrimitive::DrawPrimitive(IDirect3DDevice9* pd3dDevice)
{
	pd3dDevice->SetFVF(VERTEX::FVF);
	pd3dDevice->SetStreamSource(0, m_VertexBuffer, 0, sizeof(VERTEX));
	pd3dDevice->SetIndices(m_IndexBuffer);
	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, NumOfVertices, 0, NumOfFaces);
}