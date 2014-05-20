#ifndef _FORMATS_H_
#define _FORMATS_H_
#pragma once


// Vertex format
struct VERTEX
{
    D3DXVECTOR3 position;
    D3DXVECTOR3 normal;
    D3DXVECTOR2 texcoord;

	static const DWORD FVF;

	VERTEX(D3DXVECTOR3 position, D3DXVECTOR3 normal, D3DXVECTOR2 texcooord)
	{
		this->position = position;
		this->normal = normal;
		this->texcoord = texcooord;
	}
	VERTEX()
	{
	}
};

#endif