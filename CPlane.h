/*
==============================================================
Special class for constructing plane primitive

==============================================================
*/


#ifndef _CPLANE_H_
#define _CPLANE_H_
#pragma once


class CPlanePrimitive
{
public:
	CPlanePrimitive();
	~CPlanePrimitive();

	HRESULT Create( IDirect3DDevice9* pd3dDevice, D3DXVECTOR3 position, D3DXVECTOR2 size);
    void    Destroy();
	void	DrawPrimitive( IDirect3DDevice9* pd3dDevice );


protected:
	D3DXVECTOR3 position;
	D3DXVECTOR2 size;

	LPDIRECT3DVERTEXBUFFER9 m_VertexBuffer;
	LPDIRECT3DINDEXBUFFER9 m_IndexBuffer;

};




#endif