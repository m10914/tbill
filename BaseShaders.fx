/*
================================================================================================

Simplified shader of MeshFromObj sample, which is designed specifically to draw polygons of specified colors


(c) Damon Wall

================================================================================================
*/



//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float3 g_vMaterialDiffuse : Diffuse = float3( 0.8f, 0.8f, 0.8f );   // Material's diffuse color

float4 g_vLightPosition : LightPosition;   // Light positiondirLightIntensity
float3 g_vLightColor : LightColor = float3( 1.0f, 1.0f, 1.0f );

float3 g_vEyePosition : EyePosition;

float3 LUMINANCE_WEIGHTS = float3(0.27, 0.67, 0.06);

float4x4 g_mWorld : World;          // World matrix
float4x4 g_mWorldInv : WorldInv;	// World matrix inversed
float4x4 g_mViewProj : ViewProjection;
float4x4 g_mViewProjInv: ViewProjectionInv;
float4x4 g_mView : View;
float4x4 g_mWorldView : WorldView;
float4x4 g_mLightViewProj : LightViewProj;


texture  g_DiffTexture : Texture;   // tex1
texture  g_BumpTexture : BumpTexture;   // tex2
texture  g_SpecTexture : SpecTexture;   // tex3
texture  g_ShadowTexture : ShadowTexture;   // tex4


//function declarations
float2 FuncDirectionalLight(float2 tex, float4 normd);


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

	AddressU = clamp;
	AddressV = clamp;
};
sampler BumpTextureSampler = sampler_state
{
	Texture = <g_BumpTexture>; 
    MipFilter = NONE;
    MinFilter = NONE;
    MagFilter = NONE;

	AddressU = clamp;
	AddressV = clamp;
};
sampler SpecTextureSampler = sampler_state
{
	Texture = <g_SpecTexture>; 
    MipFilter = NONE;
    MinFilter = NONE;
    MagFilter = NONE;

	AddressU = clamp;
	AddressV = clamp;
};
sampler ShadowSampler = sampler_state
{
	Texture = <g_ShadowTexture>; 
    MipFilter = NONE;
    MinFilter = NONE;
    MagFilter = NONE;

	AddressU = clamp;
	AddressV = clamp;
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
	float4 envTex = texCUBE( BumpTextureSampler, EnvTexCoord );
	float4 difTex = tex2D( DiffTextureSampler, vTexCoord );

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
DEFERRED technique

Outputs texture to 3 render targets
=============================================================================
*/

struct DEF_VS_OUTPUT
{
	float4 vPosProj: POSITION;
	float2 vTexCoordOut: TEXCOORD0;
	float3 vInvNormal : TEXCOORD1;
	float2 vDepth : TEXCOORD2;
};

DEF_VS_OUTPUT DeferredVertShader(VS_INPUT IN)
{
	DEF_VS_OUTPUT OUT;

    OUT.vPosProj = mul( IN.vPosObject, g_mWorld );
    OUT.vPosProj = mul( OUT.vPosProj, g_mViewProj );
	OUT.vTexCoordOut = IN.vTexCoordIn;

	OUT.vInvNormal = mul(g_mWorldInv, float4(IN.vNormalObject, 0)).xyz;
	OUT.vDepth = OUT.vPosProj.zw;

	return OUT;
}

void DeferredFragmentShader( float2 vTexCoord: TEXCOORD0,
							float3 vInvNormal : TEXCOORD1,
							float2 vDepth : TEXCOORD2,
               out float4 vColorAlbedo: COLOR0,
               out float4 vColorNormals: COLOR1,
               out float4 vColorDepth: COLOR2 )
{  

	vColorAlbedo = tex2D( DiffTextureSampler, vTexCoord );
	vColorAlbedo.a = 1;
	vColorDepth = float4(vDepth.x / vDepth.y, 0, 0, 1);
	vColorNormals = float4(vInvNormal, 1);
}



/*
=============================================================================
DEFERRED for directional lights

=============================================================================
*/

void DeferredDirectionalFragmentShader(
	in	float2 tex		: TEXCOORD0,
	out	float4 color	: COLOR)
{
	float4 scene = tex2D(DiffTextureSampler, tex);
	float4 normd = tex2D(BumpTextureSampler, tex);
	float2 irrad;

	normd.w = tex2D(SpecTextureSampler, tex).r;
	irrad = FuncDirectionalLight(tex, normd);

	scene.rgb = pow(scene.rgb, 2.2f);
	color.rgb = scene.rgb * g_vLightColor.rgb * irrad.x + g_vLightColor.rgb * irrad.y;
	color.a = scene.a;
}

float2 FuncDirectionalLight(float2 tex, float4 normd)
{
	float2 irrad = 0;
	//float4 wpos = float4(tex.x * 2 - 1, tex.y * -2 + 1, normd.w, 1);
	float4 wpos = float4(tex.x * 2 - 1, tex.y * -2 + 1, normd.w, 1);

	if( normd.w > 0 )
	{
		wpos = mul(wpos, g_mViewProjInv);
		wpos /= wpos.w;

		float3 n = normalize(normd.xyz);
		float3 l = normalize(g_vLightPosition.xyz);
		float3 v = normalize(g_vEyePosition.xyz - wpos.xyz);
		float3 h = normalize(l + v);

		float diffuse = saturate(dot(n, l));
		float specular = saturate(dot(n, h));

		specular = pow(specular, 200);

		// shadow term
		float4 lpos = mul(wpos, g_mLightViewProj);
		float2 ltex = (lpos.xy / lpos.w) * float2(0.5f, -0.5f) + 0.5f;

		float2 sd = tex2D(ShadowSampler, ltex).rg;
		float d = length(g_vLightPosition.w * g_vLightPosition.xyz - wpos.xyz);

		float mean = sd.x;
		float variance = max(sd.y - sd.x * sd.x, 0.5f);

		float md = mean - d;
		float pmax = variance / (variance + md * md);

		float t = max(d <= mean, pmax);
		float s = ((sd.x < 0.01f) ? 1.0f : t);

		s = saturate(s + 0.1f);

		irrad.x = diffuse * s * 0.4f;
		irrad.y = specular * s * 0.4f;
	}

	return irrad;
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


technique DeferredGBuffer
{
	pass P0
	{
		VertexShader = compile vs_2_0 DeferredVertShader();    
        PixelShader = compile ps_2_0 DeferredFragmentShader(); 
	}
}

technique DeferredGBufferEnv
{
	pass P0
	{
		VertexShader = compile vs_2_0 DeferredVertShader();    
        PixelShader = compile ps_2_0 DeferredFragmentShader(); 
	}
}

technique DeferredDirectional
{
	pass P0
	{
		vertexshader = null;
		pixelshader = compile ps_3_0 DeferredDirectionalFragmentShader();
	}
}
