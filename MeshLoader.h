//--------------------------------------------------------------------------------------
// File: MeshLoader.h
//
// Wrapper class for ID3DXMesh interface. Handles loading mesh data from an .obj file
// and resource management for material textures.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#ifndef _MESHLOADER_H_
#define _MESHLOADER_H_
#pragma once



// Material properties per mesh subset
struct Material
{
    WCHAR   strName[MAX_PATH];

    D3DXVECTOR3 vAmbient;
    D3DXVECTOR3 vDiffuse;
    D3DXVECTOR3 vSpecular;

    int nShininess;
    float fAlpha;

    bool bSpecular;

    WCHAR   strTexture[MAX_PATH];
	WCHAR	strBumpTexture[MAX_PATH];
	WCHAR	strSpecTexture[MAX_PATH];

    IDirect3DTexture9* pTexture;
    IDirect3DTexture9* pBumpTexture;
    IDirect3DTexture9* pSpecularTexture;
};

// Used for a hashtable vertex cache when creating the mesh from a .obj file
struct CacheEntry
{
    UINT index;
    CacheEntry* pNext;
};


class CMeshLoader
{
public:
            CMeshLoader();
            ~CMeshLoader();

    HRESULT Create( IDirect3DDevice9* pd3dDevice, const WCHAR* strFilename );
    void    Destroy();
	void	SetCurrentArea(float areaVal);


	LPDIRECT3DVERTEXBUFFER9 GetVertexBuffer()
	{
		return m_VertexBuffer;
	}

	LPDIRECT3DINDEXBUFFER9 GetIndexBuffer()
	{
		return m_IndexBuffer;
	}

	LPDIRECT3DINDEXBUFFER9 GetIndexBufferSelected()
	{
		return m_IndexBufferSelected;
	}

	Material* GetMaterial()
	{
		return m_Material;
	}

	int NumOfVertices;
	int NumOfIndices;
	int NumOfIndicesSelected;
	float CurrentArea;
	float MinArea;
	float MaxArea;

	

private:

	void	DestroyIndexBuffers();
	float	CalculateArea(const D3DXVECTOR3* pts);
	void	RemakeIndexBuffers(IDirect3DDevice9* pd3dDevice);
	void    InitMaterial( Material* pMaterial );

    HRESULT LoadGeometryFromOBJ( const WCHAR* strFilename );
    HRESULT LoadMaterialsFromMTL( const WCHAR* strFileName );

   // DWORD   AddVertex( UINT hash, VERTEX* pVertex );
    void    DeleteCache();

    IDirect3DDevice9* m_pd3dDevice;    // Direct3D Device object associated with this mesh
	LPDIRECT3DVERTEXBUFFER9 m_VertexBuffer;
	LPDIRECT3DINDEXBUFFER9 m_IndexBuffer;
	LPDIRECT3DINDEXBUFFER9 m_IndexBufferSelected;

	CGrowableArray <float> m_Sizes;
    CGrowableArray <CacheEntry*> m_VertexCache;   // Hashtable cache for locating duplicate vertices
    //CGrowableArray <VERTEX> m_Vertices;      // Filled and copied to the vertex buffer
    CGrowableArray <DWORD> m_Indices;       // Filled and copied to the index buffer
	Material* m_Material;

    WCHAR   m_strMediaDir[ MAX_PATH ];               // Directory where the mesh was found
};

#endif // _MESHLOADER_H_

