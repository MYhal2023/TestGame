//*****************************************************************************
// 定数バッファ
//*****************************************************************************
cbuffer WorldBuffer : register(b0)
{
	matrix World;
}

cbuffer ViewBuffer : register(b1)
{
	matrix View;
}

cbuffer ProjectionBuffer : register(b2)
{
	matrix Projection;
}

// マテリアルバッファ
struct MATERIAL
{
	float4		Ambient;
	float4		Diffuse;
	float4		Specular;
	float4		Emission;
	float		Shininess;
	int			noTexSampling;
	float		Dummy[2];//16byte境界用
};

cbuffer MaterialBuffer : register(b3)
{
	MATERIAL	Material;
}

//=============================================================================
// 頂点シェーダ
//=============================================================================
void VertexShaderPolygon(in  float4 inPosition		: POSITION0,
	in  float4 inNormal : NORMAL0,
	in  float4 inDiffuse : COLOR0,
	in  float2 inTexCoord : TEXCOORD0,

	out float4 outPosition : SV_POSITION,
	out float2 outTexCoord: TEXCOORD0)
{
	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);
	outPosition = mul(inPosition, wvp);
	outTexCoord = inTexCoord;
}
SamplerState	g_SamplerState : register(s0);

Texture2D		g_Texture : register(t0);

//=============================================================================
// ピクセルシェーダ
//=============================================================================
void PixelShaderPolygon(in  float4 inPosition		: SV_POSITION,
	in float2 inTexCoord : TEXCOORD0,
	out float4 outDiffuse : SV_Target)
{
	
	if (Material.noTexSampling == 0)
	{
		outDiffuse = g_Texture.Sample(g_SamplerState, inTexCoord);
	}
	else
	{
		outDiffuse = 1.0f;
	}
}
