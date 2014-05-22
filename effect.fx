/*
================================================================================================

Simplified shader of MeshFromObj sample, which is designed specifically to draw polygons of specified colors

Damon Wall

================================================================================================
*/



//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float3 g_vMaterialDiffuse : Diffuse = float3( 0.8f, 0.8f, 0.8f );   // Material's diffuse color
float3 g_vLightPosition : LightPosition = float3( -2.0f, 1.0f, 0.0f );   // Light position
float3 g_vEyePosition : EyePosition;

float3 LUMINANCE_WEIGHTS = float3(0.27, 0.67, 0.06);

float4x4 g_mWorld : World;          // World matrix
float4x4 g_mViewProj : ViewProjection;
float4x4 g_mView : View;
float4x4 g_mWorldView : WorldView;

texture  g_DiffTexture : Texture;   // Color texture for mesh
texture  g_BumpTexture : BumpTexture;   // Color texture for mesh
texture  g_SpecTexture : SpecTexture;   // Color texture for mesh


/*
==========================================================
Texture samplers

==========================================================
*/
sampler DiffTextureSampler = sampler_state
{
	Texture = <g_DiffTexture>; 
    MipFilter = NONE;
    MinFilter = NONE;
    MagFilter = NONE;
};
sampler BumpTextureSampler = sampler_state
{
	Texture = <g_BumpTexture>; 
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};
sampler SpecTextureSampler = sampler_state
{
	Texture = <g_SpecTexture>; 
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};










/*
=============================================================================
VertShader

Calculates matrices and per-vertex color
=============================================================================
*/

struct VS_INPUT
{
	float4 vPosObject: POSITION;
	float3 vNormalObject: NORMAL;
	float2 vTexCoordIn: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 vPosProj: POSITION;
	float2 vTexCoordOut: TEXCOORD0;
	float3 LightVec : TEXCOORD1;
	float3 EyeVec : TEXCOORD2;
	float3 Normal : TEXCOORD3;
	float4 WorldPos : TEXCOORD4;
};

VS_OUTPUT VertShader(VS_INPUT IN)
{
	VS_OUTPUT OUT;

	// calculate position and projected position
    OUT.WorldPos = mul( IN.vPosObject, g_mWorld );
    OUT.vPosProj = mul( OUT.WorldPos, g_mViewProj );
    
	// assign texture coordinates
	OUT.vTexCoordOut = IN.vTexCoordIn;

	// calculate tangent space
    float3 normal = normalize(mul( IN.vNormalObject, (float3x3)g_mWorld ));
	float3 binormal = cross(float3(1,0,0),normal);
    float3 tang = cross(normal,binormal);
   	float3x3 t;
   	t[0] = tang;
    t[1] = binormal;
    t[2] = normal;

	OUT.Normal = normal;

	// transform light and eye to tangent space
	OUT.LightVec = normalize(mul(t, g_vLightPosition));
	OUT.EyeVec = normalize(mul(t, g_vEyePosition-OUT.WorldPos));
  
	return OUT;
}



/*
=============================================================================
FragmentShader

does nothing
=============================================================================
*/
void FragmentShader( float2 vTexCoord: TEXCOORD0,
					float3 vLightVec : TEXCOORD1,
					float3 vEyeVec : TEXCOORD2,
					float3 Normal : TEXCOORD3,
					float4 WorldPos : TEXCOORD4,
               out float4 vColorOut: COLOR0)
{  

	float4 vDiffuse = tex2D( DiffTextureSampler, float2(vTexCoord.x,1-vTexCoord.y) );
	float4 vBump = tex2D( BumpTextureSampler, float2(vTexCoord.x,1-vTexCoord.y) );
	float4 vSpec = tex2D( SpecTextureSampler, float2(vTexCoord.x,1-vTexCoord.y) );

	vBump = vBump * 2.0 - 1.0;

	float light = saturate( dot(vLightVec, vBump) );
	float3 reflect = normalize( 4*light*vBump - vLightVec);
	float specular = pow(saturate(dot(reflect, vEyeVec)), 10) * length(vSpec) * 5 * light;

	vColorOut.rgb = vDiffuse * (light*0.8 + 0.2 + specular);
	vColorOut.rgb *= g_vMaterialDiffuse;
	vColorOut.a = 1;
}



//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// env map test
/*
=============================================================================
VertShader

Calculates matrices and per-vertex color
=============================================================================
*/

struct EM_VS_OUTPUT
{
	float4 vPosProj: POSITION;
	float2 vTexCoordOut: TEXCOORD0;
	float3 Normal : TEXCOORD1;
	float4 WorldPos : TEXCOORD2;
	float3 EnvTexCoord: TEXCOORD3;
};

EM_VS_OUTPUT EMVertShader(VS_INPUT IN)
{
	EM_VS_OUTPUT OUT;

	// calculate position and projected position
    OUT.WorldPos = mul( IN.vPosObject, g_mWorld );
    OUT.vPosProj = mul( OUT.WorldPos, g_mViewProj );
    OUT.Normal = normalize(mul( IN.vNormalObject, (float3x3)g_mWorld ));

	// assign texture coordinates
	OUT.vTexCoordOut = IN.vTexCoordIn;

	// em (need to optimize)
	float4 oPos = mul( IN.vPosObject, g_mWorldView );
    float3 vN = mul( IN.vNormalObject, g_mWorldView );
    vN = normalize( vN );
    float3 vEyeR = -normalize( oPos );
    OUT.EnvTexCoord = 2 * dot( vEyeR, vN ) * vN - vEyeR;

	return OUT;
}


/*
=============================================================================
EMFragmentShader

does nothing
=============================================================================
*/
void EMFragmentShader( float2 vTexCoord: TEXCOORD0,
					float3 Normal : TEXCOORD1,
					float4 WorldPos : TEXCOORD2,
					float3 EnvTexCoord : TEXCOORD3,
               out float4 vColorOut: COLOR0)
{  
	float4 envTex = texCUBE( DiffTextureSampler, EnvTexCoord );
	float4 difTex = tex2D( BumpTextureSampler, vTexCoord );

	float3 lightDir = normalize(g_vLightPosition - WorldPos);
	float3 viewDir = normalize(g_vEyePosition - WorldPos);

	float light = saturate(dot(lightDir, Normal));
	float3 reflect = normalize(2 * light * Normal - lightDir);
	float4 specular = pow(saturate(dot(reflect, viewDir)), 8) * light;
	float envSpec = pow(dot(LUMINANCE_WEIGHTS, envTex.rgb), 5);

	vColorOut = difTex * (light*0.5 + 0.5) + specular + envSpec;
}




/*
=============================================================================
PLAIN technique

Outputs texture - no lighting and other stuff
=============================================================================
*/

struct PLAIN_VS_OUTPUT
{
	float4 vPosProj: POSITION;
	float2 vTexCoordOut: TEXCOORD0;
};


PLAIN_VS_OUTPUT PlainVertShader(VS_INPUT IN)
{
	PLAIN_VS_OUTPUT OUT;

    OUT.vPosProj = mul( IN.vPosObject, g_mWorld );
    OUT.vPosProj = mul( OUT.vPosProj, g_mViewProj );
	OUT.vTexCoordOut = IN.vTexCoordIn;

	return OUT;
}

void PlainFragmentShader( float2 vTexCoord: TEXCOORD0,
               out float4 vColorOut: COLOR0)
{  

	vColorOut = tex2D( DiffTextureSampler, vTexCoord);
}




/*
=============================================================================
Techniques
=============================================================================
*/
technique Default
{
    pass P0
    {
        //VertexShader = compile vs_2_0 VertShader();    
        //PixelShader = compile ps_2_0 FragmentShader();  

		VertexShader = compile vs_2_0 VertShader();    
        PixelShader = compile ps_2_0 FragmentShader();  
    }
}

technique Plain
{
	pass P0
	{
		VertexShader = compile vs_2_0 PlainVertShader();    
        PixelShader = compile ps_2_0 PlainFragmentShader();  
	}
}


technique EnvMap
{
	pass P0
	{
		VertexShader = compile vs_2_0 EMVertShader();    
        PixelShader = compile ps_2_0 EMFragmentShader(); 
	}
}
