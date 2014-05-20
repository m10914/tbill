/*
==============================================================
Special class for constructing sphere primitive

==============================================================
*/


#ifndef _CSPHERE_H_
#define _CSPHERE_H_
#pragma once


class CSpherePrimitive
{
public:
	CSpherePrimitive();
	~CSpherePrimitive();

	HRESULT Create( IDirect3DDevice9* pd3dDevice, D3DXVECTOR3 position, float radius, int segmentWidth, int segmentHeight);
    void    Destroy();
	void	DrawPrimitive( IDirect3DDevice9* pd3dDevice );


protected:
	D3DXVECTOR3 position;
	float radius;
	int segmentX, segmentY;

	int NumOfFaces;
	int NumOfVertices;
	int NumOfIndices;

	LPDIRECT3DVERTEXBUFFER9 m_VertexBuffer;
	LPDIRECT3DINDEXBUFFER9 m_IndexBuffer;

};





#endif