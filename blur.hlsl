//*****************************************************************************
// �萔�o�b�t�@
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

// �}�e���A���o�b�t�@
struct MATERIAL
{
	float4		Ambient;
	float4		Diffuse;
	float4		Specular;
	float4		Emission;
	float		Shininess;
	int			noTexSampling;
	float		Dummy[2];//16byte���E�p
};

cbuffer MaterialBuffer : register(b3)
{
	MATERIAL	Material;
}

// ���C�g�p�o�b�t�@
struct LIGHT
{
	float4		Direction[5];
	float4		Position[5];
	float4		Diffuse[5];
	float4		Ambient[5];
	float4		Attenuation[5];
	int4		Flags[5];
	int			Enable;
	int			Dummy[3];//16byte���E�p
};

cbuffer LightBuffer : register(b4)
{
	LIGHT		Light;
}

cbuffer LightViewBuffer : register(b8)
{
	matrix LightView;
}

cbuffer LightProjectionBuffer : register(b9)
{
	matrix LightProjection;
}

//=============================================================================
// ���_�V�F�[�_
//=============================================================================
void VertexShaderPolygon(in  float4 inPosition		: POSITION0,
	in  float4 inNormal : NORMAL0,
	in  float4 inDiffuse : COLOR0,
	in  float2 inTexCoord : TEXCOORD0,

	out float4 outPosition : SV_POSITION,
	out float4 outNormal : NORMAL0,
	out float2 outTexCoord : TEXCOORD0,
	out float4 outDiffuse : COLOR0,
	out float4 outWorldPos : POSITION0,
	out float4 outSMPosition : POSITION1)
{
	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);
	outPosition = mul(inPosition, wvp);

	outNormal = normalize(mul(float4(inNormal.xyz, 0.0f), World));

	outTexCoord = inTexCoord;

	outWorldPos = mul(inPosition, World);

	outDiffuse = inDiffuse;

	//���C�g�̖ڐ��ɂ�郏�[���h�r���[�ˉe�ϊ�
	matrix mat;
	mat = mul(World, LightView);	//���[���h�ϊ��s��A���C�g�r���[�ϊ��s��
	mat = mul(mat, LightProjection);	//���C�g�r���[�ł̎ˉe�ϊ��s��
	outSMPosition = mul(inPosition, mat);
}

SamplerState	g_SamplerState : register(s0);

Texture2D		g_Texture : register(t0);

//=============================================================================
// �s�N�Z���V�F�[�_
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
