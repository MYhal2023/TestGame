

//*****************************************************************************
// 定数バッファ
//*****************************************************************************

// マトリクスバッファ
cbuffer WorldBuffer : register( b0 )
{
	matrix World;
}

cbuffer ViewBuffer : register( b1 )
{
	matrix View;
}

cbuffer ProjectionBuffer : register( b2 )
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

cbuffer MaterialBuffer : register( b3 )
{
	MATERIAL	Material;
}

// ライト用バッファ
struct LIGHT
{
	float4		Direction[5];
	float4		Position[5];
	float4		Diffuse[5];
	float4		Ambient[5];
	float4		Attenuation[5];
	int4		Flags[5];
	int			Enable;
	int			Dummy[3];//16byte境界用
};

cbuffer LightBuffer : register( b4 )
{
	LIGHT		Light;
}

struct FOG
{
	float4		Distance;
	float4		FogColor;
	int			Enable;
	float		Dummy[3];//16byte境界用
};

// フォグ用バッファ
cbuffer FogBuffer : register( b5 )
{
	FOG			Fog;
};

// 縁取り用バッファ
cbuffer Fuchi : register(b6)
{
	int			fuchi;
	int			fill[3];
};


cbuffer CameraBuffer : register(b7)
{
	float4 Camera;
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
// 頂点シェーダ
//=============================================================================
void VertexShaderPolygon( in  float4 inPosition		: POSITION0,
						  in  float4 inNormal		: NORMAL0,
						  in  float4 inDiffuse		: COLOR0,
						  in  float2 inTexCoord		: TEXCOORD0,

						  out float4 outPosition	: SV_POSITION,
						  out float4 outNormal		: NORMAL0,
						  out float2 outTexCoord	: TEXCOORD0,
						  out float4 outDiffuse		: COLOR0,
						  out float4 outWorldPos	: POSITION0,
						  out float4 outSMPosition  : POSITION1)
{
	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);
	outPosition = mul(inPosition, wvp);

	outNormal = normalize(mul(float4(inNormal.xyz, 0.0f), World));

	outTexCoord = inTexCoord;

	outWorldPos = mul(inPosition, World);

	outDiffuse = inDiffuse;

	//ライトの目線によるワールドビュー射影変換
	matrix mat;
	mat = mul(World, LightView);	//ワールド変換行列、ライトビュー変換行列
	mat = mul(mat, LightProjection);	//ライトビューでの射影変換行列
	outSMPosition = mul(inPosition, mat);
}



//*****************************************************************************
// グローバル変数
//*****************************************************************************
Texture2D		g_Texture : register( t0 );
Texture2D<float>	g_SMDepth : register(t1);

SamplerState	g_SamplerState : register( s0 );
SamplerState	g_SMSamplerState : register(s1);

//=============================================================================
// ピクセルシェーダ
//=============================================================================
void PixelShaderPolygon( in  float4 inPosition		: SV_POSITION,
						 in  float4 inNormal		: NORMAL0,
						 in  float2 inTexCoord		: TEXCOORD0,
						 in  float4 inDiffuse		: COLOR0,
						 in  float4 inWorldPos      : POSITION0,
						 in  float4 inSMPosition    : POSITION1,

						 out float4 outDiffuse		: SV_Target )
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_Texture.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}

	// 影の処理
	float ZValue = inSMPosition.z / inSMPosition.w;

	float2 SMTexture;
	SMTexture.x = (1.0f + inSMPosition.x / inSMPosition.w) * 0.5f;
	SMTexture.y = (1.0f - inSMPosition.y / inSMPosition.w) * 0.5f;

	float SMZValue = g_SMDepth.Sample(g_SMSamplerState, SMTexture).r + 0.001f;	//影にしたい領域

	if (Light.Enable == 0)
	{
		color = color * Material.Diffuse;
	}
	else if(SMZValue < ZValue) // ライトPOSから計算　SM＝ライトからの情報　Z＝視点からの情報
	{
		color = float4(clamp(color.rgb - 0.85f, 0.0f, 1.0f), 1.0f);   // 影の色
	}
	else
	{
		float4 tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

		for (int i = 0; i < 5; i++)
		{
			float3 lightDir;
			float light;

			if (Light.Flags[i].y == 1)
			{
				if (Light.Flags[i].x == 1)
				{
					lightDir = normalize(Light.Direction[i].xyz);
					light = dot(lightDir, inNormal.xyz);

					light = 0.5 - 0.5 * light;
					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];
				}
				else if (Light.Flags[i].x == 2)
				{
					lightDir = normalize(Light.Position[i].xyz - inWorldPos.xyz);
					light = dot(lightDir, inNormal.xyz);

					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];

					float distance = length(inWorldPos - Light.Position[i]);

					float att = saturate((Light.Attenuation[i].x - distance) / Light.Attenuation[i].x);
					tempColor *= att;
				}
				else
				{
					tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
				}

				outColor += tempColor;
			}
		}

		color = outColor;
		color.a = inDiffuse.a * Material.Diffuse.a;
	}

	//フォグ
	if (Fog.Enable == 1)
	{
		float z = inPosition.z*inPosition.w;
		float f = (Fog.Distance.y - z) / (Fog.Distance.y - Fog.Distance.x);
		f = saturate(f);
		outDiffuse = f * color + (1 - f)*Fog.FogColor;
		outDiffuse.a = color.a;
	}
	else
	{
		outDiffuse = color;
	}

	////縁取り
	//if (fuchi == 1)
	//{
	//	float angle = dot(normalize(inWorldPos.xyz - Camera.xyz), normalize(inNormal));
	//	//if ((angle < 0.5f)&&(angle > -0.5f))
	//	if (angle > -0.4f)
	//	{
	//		outDiffuse.r  = 1.0f;
	//		outDiffuse.g = 1.0f;		
	//		outDiffuse.b = 1.0f;
	//	}
	//}
}

//=============================================================================
// モノクロフィルタ用頂点シェーダ
//=============================================================================
void VertexShaderMonoPolygon(in  float4 inPosition		: POSITION0,	//射影変換座標
	in	float4 inNormal : NORMAL0,
	in  float4 inDiffuse : COLOR0,											//ディフューズ色
	in  float2 inTexCoord : TEXCOORD0,										//Z値算出用テクスチャ
	out float4 outPosition : SV_POSITION,
	out float4 outNormal : NORMAL0,
	out float2 outTexCoord : TEXCOORD0,
	out float4 outDiffuse : COLOR0)
{
	//ワールドビュー射影変換
	matrix mat;
	mat = mul(World, View);		//ワールド変換行列
	mat = mul(mat, Projection);	//射影変換行列
	outPosition = mul(inPosition, mat);
	outNormal = normalize(mul(float4(inNormal.xyz, 0.0f), World));
	outTexCoord = inTexCoord;
	outDiffuse = inDiffuse;
}

//=============================================================================
// モノクロフィルタ用ピクセルシェーダ
//=============================================================================
void PixelShaderMonoPolygon(in  float4 inPosition		: SV_POSITION,
	in  float4 inNormal : NORMAL0,
	in  float2 inTexCoord : TEXCOORD0,
	in  float4 inDiffuse : COLOR0,

	out float4 outDiffuse : SV_Target)
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_Texture.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}

	float Y = color.r * 0.29891f + color.g * 0.58661f + color.b * 0.11448f;	//ピクセルの輝度値を抽出
	color.r = Y;
	color.g = Y;
	color.b = Y;


	outDiffuse = color;
}

//=============================================================================
// 水面の頂点シェーダ
//=============================================================================
void VertexShaderSpecularPolygon(in  float4 inPosition		: POSITION0,
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

	//ライトの目線によるワールドビュー射影変換
	matrix mat;
	mat = mul(World, LightView);	//ワールド変換行列、ライトビュー変換行列
	mat = mul(mat, LightProjection);	//ライトビューでの射影変換行列
	outSMPosition = mul(inPosition, mat);
}

//=============================================================================
//水面のピクセルシェーダー
//=============================================================================
void PixelShaderSpecularPolygon(	in  float4 inPosition	: SV_POSITION,
							in  float4 inNormal		: NORMAL0,
							in  float2 inTexCoord	: TEXCOORD0,
							in  float4 inDiffuse	: COLOR0,
							in  float4 inWorldPos	: POSITION0,
							in  float4 inSMPosition : POSITION1,

							out float4 outDiffuse	: SV_Target)
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_Texture.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}

	// 影の処理
	float ZValue = inSMPosition.z / inSMPosition.w;

	float2 SMTexture;
	SMTexture.x = (1.0f + inSMPosition.x / inSMPosition.w) * 0.5f;
	SMTexture.y = (1.0f - inSMPosition.y / inSMPosition.w) * 0.5f;

	float SMZValue = g_SMDepth.Sample(g_SMSamplerState, SMTexture).r + 0.001f;	//影にしたい領域

	if (SMZValue < ZValue) // ライトPOSから計算　SM＝ライトからの情報　Z＝視点からの情報
	{
		color = float4(clamp(color.rgb - 0.2f, 0.0f, 1.0f), 1.0f);   // 影の色
	}
	else
	{
		float3 lightDir;
		float light;
		float4 tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

		if (Light.Flags[0].y == 1)
		{
			if (Light.Flags[0].x == 1)
			{
				lightDir = normalize(Light.Direction[0].xyz);
				light = dot(lightDir, inNormal.xyz);

				light = 0.5 - 0.5 * light;
				tempColor = color * Material.Diffuse * light * Light.Diffuse[0];
			}
			else if (Light.Flags[0].x == 2)
			{
				lightDir = normalize(Light.Position[0].xyz - inWorldPos.xyz);
				light = dot(lightDir, inNormal.xyz);

				tempColor = color * Material.Diffuse * light * Light.Diffuse[0];

				float distance = length(inWorldPos - Light.Position[0]);

				float att = saturate((Light.Attenuation[0].x - distance) / Light.Attenuation[0].x);
				tempColor *= att;
			}
			else
			{
				tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
			}

			outColor += tempColor;
		}

		color = outColor;
		color.a = inDiffuse.a * Material.Diffuse.a;

	}

	color.a = inDiffuse.a * Material.Diffuse.a;
	outDiffuse = color;

}