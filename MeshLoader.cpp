//--------------------------------------------------------------------------------------
// File: MeshLoader.cpp
//
// Wrapper class for ID3DXMesh interface. Handles loading mesh data from an .obj file
// and resource management for material textures.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "SDKmisc.h"
#pragma warning(disable: 4995)
#include "meshloader.h"
#include "Formats.h"
#include <fstream>
using namespace std;
#pragma warning(default: 4995)



//--------------------------------------------------------------------------------------
CMeshLoader::CMeshLoader()
{
    m_pd3dDevice = NULL;
	m_IndexBuffer = NULL;
	m_VertexBuffer = NULL;
	m_Material = NULL;

	MinArea = 999;
	MaxArea = -999;

    ZeroMemory( m_strMediaDir, sizeof( m_strMediaDir ) );
}


//--------------------------------------------------------------------------------------
CMeshLoader::~CMeshLoader()
{
    Destroy();
}


//--------------------------------------------------------------------------------------
void CMeshLoader::Destroy()
{
	if(m_Material != NULL)
	{
		SAFE_RELEASE( m_Material->pTexture );
		SAFE_RELEASE( m_Material->pBumpTexture );
		SAFE_RELEASE( m_Material->pSpecularTexture );
		SAFE_DELETE( m_Material );
	}
	
    //m_Vertices.RemoveAll();
    m_Indices.RemoveAll();
	m_Sizes.RemoveAll();

	//destroy buffers
	SAFE_RELEASE( m_VertexBuffer );

	DestroyIndexBuffers();
    
	m_pd3dDevice = NULL;
}

void CMeshLoader::SetCurrentArea(float areaVal)
{
	CurrentArea = areaVal;
	RemakeIndexBuffers(m_pd3dDevice);
}

//--------------------------------------------------------------------------------------
void CMeshLoader::DestroyIndexBuffers()
{
	SAFE_RELEASE( m_IndexBuffer );
	SAFE_RELEASE( m_IndexBufferSelected );
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader::Create( IDirect3DDevice9* pd3dDevice, const WCHAR* strFilename )
{
    HRESULT hr;
    WCHAR str[ MAX_PATH ] = {0};

    // Start clean
    Destroy();

    // Store the device pointer
    m_pd3dDevice = pd3dDevice;

    // Load the vertex buffer, index buffer, and subset information from a file. In this case, 
    // an .obj file was chosen for simplicity, but it's meant to illustrate that ID3DXMesh objects
    // can be filled from any mesh file format once the necessary data is extracted from file.
    V_RETURN( LoadGeometryFromOBJ( strFilename ) );

    // Set the current directory based on where the mesh was found
    WCHAR wstrOldDir[MAX_PATH] = {0};
    GetCurrentDirectory( MAX_PATH, wstrOldDir );
    SetCurrentDirectory( m_strMediaDir );

	
	// LOAD TEXTURES

	if(m_Material->strTexture != NULL)
	{
		V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, m_Material->strTexture ) );
		V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, m_Material->strTexture, &( m_Material->pTexture ) ) );
	}
	if(m_Material->strBumpTexture != NULL)
	{
		V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, m_Material->strBumpTexture ) );
		V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, m_Material->strBumpTexture, &( m_Material->pBumpTexture ) ) );
	}
	if(m_Material->strSpecTexture != NULL)
	{
		V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, m_Material->strSpecTexture ) );
		V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, m_Material->strSpecTexture, &( m_Material->pSpecularTexture ) ) );
	}


    // Restore the original current directory
    SetCurrentDirectory( wstrOldDir );

	NumOfIndices = m_Indices.GetSize();
	//NumOfVertices = m_Vertices.GetSize();
	CurrentArea = (MinArea + MaxArea) / 2;

	RemakeIndexBuffers(pd3dDevice);

	// create vertex buffer
	/*if( FAILED(pd3dDevice->CreateVertexBuffer(m_Vertices.GetSize() * sizeof(VERTEX), D3DUSAGE_WRITEONLY,
		VERTEX::FVF, D3DPOOL_MANAGED, &m_VertexBuffer, NULL)))
	{
		return E_FAIL;
	}
	VERTEX* pVertex;
	V_RETURN( m_VertexBuffer->Lock(0, NumOfVertices*sizeof(VERTEX), (void**)&pVertex, 0) );
	memcpy( pVertex, m_Vertices.GetData(), NumOfVertices * sizeof( VERTEX ) );
	m_VertexBuffer->Unlock();*/


    return S_OK;
}

//--------------------------------------------------------------------------------------
void CMeshLoader::RemakeIndexBuffers( IDirect3DDevice9* pd3dDevice )
{	
	DWORD* pIndex;
	CGrowableArray<DWORD> selected;
	CGrowableArray<DWORD> notSelected;

	DestroyIndexBuffers();

	for(int i = 0; i < NumOfIndices/3; i++)
	{
		float size = m_Sizes.GetAt(i);

		if(size < CurrentArea)
		{
			selected.Add(m_Indices.GetAt((i*3)));
			selected.Add(m_Indices.GetAt((i*3)+1));
			selected.Add(m_Indices.GetAt((i*3)+2));
		}
		else
		{
			notSelected.Add(m_Indices.GetAt((i*3)));
			notSelected.Add(m_Indices.GetAt((i*3)+1));
			notSelected.Add(m_Indices.GetAt((i*3)+2));
		}
	}

	NumOfIndicesSelected = selected.GetSize();
	int NumOfIndicesNotSelected = (NumOfIndices-NumOfIndicesSelected);

	//build index buffers

	if(NumOfIndicesNotSelected > 0)
	{
		if(FAILED(pd3dDevice->CreateIndexBuffer(NumOfIndicesNotSelected * sizeof(DWORD), D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_IndexBuffer, NULL )))
		{
			return;
		}

		m_IndexBuffer->Lock(0, NumOfIndicesNotSelected*sizeof(DWORD), (void**)&pIndex, 0);
		memcpy( pIndex, notSelected.GetData(), NumOfIndicesNotSelected * sizeof(DWORD));
		m_IndexBuffer->Unlock();
	}
	
	if(NumOfIndicesSelected > 0)
	{
		if(FAILED(pd3dDevice->CreateIndexBuffer(NumOfIndicesSelected * sizeof(DWORD), D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_IndexBufferSelected, NULL )))
		{
			return;
		}

		m_IndexBufferSelected->Lock(0, NumOfIndicesSelected*sizeof(DWORD), (void**)&pIndex, 0);
		memcpy( pIndex, selected.GetData(), NumOfIndicesSelected * sizeof(DWORD));
		m_IndexBufferSelected->Unlock();
	}
	
	

	//cleanup
	selected.RemoveAll();
	notSelected.RemoveAll();
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader::LoadGeometryFromOBJ( const WCHAR* strFileName )
{
    WCHAR strMaterialFilename[MAX_PATH] = {0};
    WCHAR wstr[MAX_PATH];
    char str[MAX_PATH];
    HRESULT hr;

    // Find the file
    V_RETURN( DXUTFindDXSDKMediaFileCch( wstr, MAX_PATH, strFileName ) );
    WideCharToMultiByte( CP_ACP, 0, wstr, -1, str, MAX_PATH, NULL, NULL );

    // Store the directory where the mesh was found
    wcscpy_s( m_strMediaDir, MAX_PATH - 1, wstr );
    WCHAR* pch = wcsrchr( m_strMediaDir, L'\\' );
    if( pch )
        *pch = NULL;

    // Create temporary storage for the input data. Once the data has been loaded into
    // a reasonable format we can create a D3DXMesh object and load it with the mesh data.
    CGrowableArray <D3DXVECTOR3> Positions;
    CGrowableArray <D3DXVECTOR2> TexCoords;
    CGrowableArray <D3DXVECTOR3> Normals;


	// The first subset uses the default material
    // File input
    WCHAR strCommand[256] = {0};
    wifstream InFile( str );
    if( !InFile )
        return DXTRACE_ERR( L"wifstream::open", E_FAIL );

    for(; ; )
    {
        InFile >> strCommand;
        if( !InFile )
            break;

        if( 0 == wcscmp( strCommand, L"#" ) )
        {
            // Comment
        }
        else if( 0 == wcscmp( strCommand, L"v" ) )
        {
            // Vertex Position
            float x, y, z;
            InFile >> x >> y >> z;
            Positions.Add( D3DXVECTOR3( x, y, z ) );
        }
        else if( 0 == wcscmp( strCommand, L"vt" ) )
        {
            // Vertex TexCoord
            float u, v;
            InFile >> u >> v;
            TexCoords.Add( D3DXVECTOR2( u, v ) );
        }
        else if( 0 == wcscmp( strCommand, L"vn" ) )
        {
            // Vertex Normal
            float x, y, z;
            InFile >> x >> y >> z;
            Normals.Add( D3DXVECTOR3( x, y, z ) );
        }
        else if( 0 == wcscmp( strCommand, L"f" ) )
        {
            // Face
			D3DXVECTOR3 verticesOfFace[3];
            UINT iPosition, iTexCoord, iNormal;
            /*VERTEX vertex;

            for( UINT iFace = 0; iFace < 3; iFace++ )
            {
                ZeroMemory( &vertex, sizeof( VERTEX ) );

                // OBJ format uses 1-based arrays
                InFile >> iPosition;
                vertex.position = Positions[ iPosition - 1 ];
				verticesOfFace[iFace] = Positions[ iPosition - 1];

                if( '/' == InFile.peek() )
                {
                    InFile.ignore();

                    if( '/' != InFile.peek() )
                    {
                        // Optional texture coordinate
                        InFile >> iTexCoord;
                        vertex.texcoord = TexCoords[ iTexCoord - 1 ];
                    }

                    if( '/' == InFile.peek() )
                    {
                        InFile.ignore();

                        // Optional vertex normal
                        InFile >> iNormal;
                        vertex.normal = Normals[ iNormal - 1 ];
                    }
                }

                // If a duplicate vertex doesn't exist, add this vertex to the Vertices
                // list. Store the index in the Indices array. The Vertices and Indices
                // lists will eventually become the Vertex Buffer and Index Buffer for
                // the mesh.
                DWORD index = AddVertex( iPosition, &vertex );
                if ( index == (DWORD)-1 )
                    return E_OUTOFMEMORY;

                m_Indices.Add( index );
            }

			//HERE'S CALCULATION OF SQUARE
			float res = CalculateArea(verticesOfFace);
			m_Sizes.Add( res );
			if(res < MinArea) MinArea = res;
			if(res > MaxArea) MaxArea = res;*/
        }


		else if( 0 == wcscmp( strCommand, L"mtllib" ) )
        {
            // Material library
            InFile >> strMaterialFilename;
        }
        else if( 0 == wcscmp( strCommand, L"usemtl" ) )
        {
            //ignore this command, as we allow no more then 1 material for this object (for now)
        }


        else
        {
            // Unimplemented or unrecognized command
        }

        InFile.ignore( 1000, '\n' );
    }

    // Cleanup
    InFile.close();
    DeleteCache();


	if( strMaterialFilename[0] )
    {
        V_RETURN( LoadMaterialsFromMTL( strMaterialFilename ) );
    }
	else
	{
		m_Material = new Material();
		InitMaterial(m_Material);
	}

    return S_OK;
}


//
float CMeshLoader::CalculateArea(const D3DXVECTOR3* pts)
{
	D3DXVECTOR3 a(pts[1].x-pts[0].x, pts[1].y-pts[0].y, pts[1].z-pts[0].z);
	D3DXVECTOR3 b(pts[2].x - pts[0].x, pts[2].y - pts[0].y, pts[2].z - pts[0].z);

	float area = 0.5f * sqrt(
		pow((a.y*b.z - a.z*b.y),2) + 
		pow((a.z*b.x - a.x*b.z),2) + 
		pow((a.x*b.y - a.y*b.x),2) 
		);
	return area;
}


//--------------------------------------------------------------------------------------
/*DWORD CMeshLoader::AddVertex( UINT hash, VERTEX* pVertex )
{
    // If this vertex doesn't already exist in the Vertices list, create a new entry.
    // Add the index of the vertex to the Indices list.
    bool bFoundInList = false;
    DWORD index = 0;

    // Since it's very slow to check every element in the vertex list, a hashtable stores
    // vertex indices according to the vertex position's index as reported by the OBJ file
    if( ( UINT )m_VertexCache.GetSize() > hash )
    {
        CacheEntry* pEntry = m_VertexCache.GetAt( hash );
        while( pEntry != NULL )
        {
            VERTEX* pCacheVertex = m_Vertices.GetData() + pEntry->index;

            // If this vertex is identical to the vertex already in the list, simply
            // point the index buffer to the existing vertex
            if( 0 == memcmp( pVertex, pCacheVertex, sizeof( VERTEX ) ) )
            {
                bFoundInList = true;
                index = pEntry->index;
                break;
            }

            pEntry = pEntry->pNext;
        }
    }

    // Vertex was not found in the list. Create a new entry, both within the Vertices list
    // and also within the hashtable cache
    if( !bFoundInList )
    {
        // Add to the Vertices list
        index = m_Vertices.GetSize();
        m_Vertices.Add( *pVertex );

        // Add this to the hashtable
        CacheEntry* pNewEntry = new CacheEntry;
        if( pNewEntry == NULL )
            return (DWORD)-1;

        pNewEntry->index = index;
        pNewEntry->pNext = NULL;

        // Grow the cache if needed
        while( ( UINT )m_VertexCache.GetSize() <= hash )
        {
            m_VertexCache.Add( NULL );
        }

        // Add to the end of the linked list
        CacheEntry* pCurEntry = m_VertexCache.GetAt( hash );
        if( pCurEntry == NULL )
        {
            // This is the head element
            m_VertexCache.SetAt( hash, pNewEntry );
        }
        else
        {
            // Find the tail
            while( pCurEntry->pNext != NULL )
            {
                pCurEntry = pCurEntry->pNext;
            }

            pCurEntry->pNext = pNewEntry;
        }
    }

    return index;
}*/


//--------------------------------------------------------------------------------------
void CMeshLoader::DeleteCache()
{
    // Iterate through all the elements in the cache and subsequent linked lists
    for( int i = 0; i < m_VertexCache.GetSize(); i++ )
    {
        CacheEntry* pEntry = m_VertexCache.GetAt( i );
        while( pEntry != NULL )
        {
            CacheEntry* pNext = pEntry->pNext;
            SAFE_DELETE( pEntry );
            pEntry = pNext;
        }
    }

    m_VertexCache.RemoveAll();
}





//--------------------------------------------------------------------------------------
HRESULT CMeshLoader::LoadMaterialsFromMTL( const WCHAR* strFileName )
{
    HRESULT hr;

    // Set the current directory based on where the mesh was found
	WCHAR strMaterialFilename[MAX_PATH] = {0};
    WCHAR wstrOldDir[MAX_PATH] = {0};
    GetCurrentDirectory( MAX_PATH, wstrOldDir );
    SetCurrentDirectory( m_strMediaDir );

    // Find the file
    WCHAR strPath[MAX_PATH];
    char cstrPath[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( strPath, MAX_PATH, strFileName ) );
    WideCharToMultiByte( CP_ACP, 0, strPath, -1, cstrPath, MAX_PATH, NULL, NULL );

    // File input
    WCHAR strCommand[256] = {0};
    wifstream InFile( cstrPath );
    if( !InFile )
        return DXTRACE_ERR( L"wifstream::open", E_FAIL );

    // Restore the original current directory
    SetCurrentDirectory( wstrOldDir );

    for(; ; )
    {
        InFile >> strCommand;
        if( !InFile )
            break;

		if( wcscmp( strCommand, L"newmtl" ) == 0L)
        {
			if(m_Material == NULL)
			{
				m_Material = new Material();
				InitMaterial(m_Material);
			}
			else
			{
				break;
			}
        }

        // The rest of the commands rely on an active material
		if( m_Material == NULL ) continue;

        if( 0 == wcscmp( strCommand, L"#" ) )
        {
            // Comment
        }
        else if( 0 == wcscmp( strCommand, L"Ka" ) )
        {
            // Ambient color
            float r, g, b;
            InFile >> r >> g >> b;
            m_Material->vAmbient = D3DXVECTOR3( r, g, b );
        }
        else if( 0 == wcscmp( strCommand, L"Kd" ) )
        {
            // Diffuse color
            float r, g, b;
            InFile >> r >> g >> b;
            m_Material->vDiffuse = D3DXVECTOR3( r, g, b );
        }
        else if( 0 == wcscmp( strCommand, L"Ks" ) )
        {
            // Specular color
            float r, g, b;
            InFile >> r >> g >> b;
            m_Material->vSpecular = D3DXVECTOR3( r, g, b );
        }
        else if( 0 == wcscmp( strCommand, L"d" ) ||
                 0 == wcscmp( strCommand, L"Tr" ) )
        {
            // Alpha
            InFile >> m_Material->fAlpha;
        }
        else if( 0 == wcscmp( strCommand, L"Ns" ) )
        {
            // Shininess
            int nShininess;
            InFile >> nShininess;
            m_Material->nShininess = nShininess;
        }
        else if( 0 == wcscmp( strCommand, L"illum" ) )
        {
            // Specular on/off
            int illumination;
            InFile >> illumination;
            m_Material->bSpecular = ( illumination == 2 );
        }
        else if( 0 == wcscmp( strCommand, L"map_Kd" ) )
        {
            // Texture
            InFile >> m_Material->strTexture;
        }
		else if( 0 == wcscmp( strCommand, L"map_Ks" ) )
        {
            // Texture
			InFile >> m_Material->strSpecTexture;
        }
		else if( 0 == wcscmp( strCommand, L"map_bump" ) )
        {
            // Texture
			InFile >> m_Material->strBumpTexture;
        }
        else
        {
            // Unimplemented or unrecognized command
        }

        InFile.ignore( 1000, L'\n' );
    }

    InFile.close();

    return S_OK;
}


//--------------------------------------------------------------------------------------
void CMeshLoader::InitMaterial( Material* pMaterial )
{
    ZeroMemory( pMaterial, sizeof( Material ) );

    pMaterial->vAmbient = D3DXVECTOR3( 0.2f, 0.2f, 0.2f );
    pMaterial->vDiffuse = D3DXVECTOR3( 0.8f, 0.8f, 0.8f );
    pMaterial->vSpecular = D3DXVECTOR3( 1.0f, 1.0f, 1.0f );
    pMaterial->nShininess = 0;
    pMaterial->fAlpha = 1.0f;
    pMaterial->bSpecular = false;
    pMaterial->pTexture = NULL;
	pMaterial->pBumpTexture = NULL;
	pMaterial->pSpecularTexture = NULL;
}
