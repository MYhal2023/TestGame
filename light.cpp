//=============================================================================
//
// ライト処理 [light.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "player.h"
#include "time.h"
#include "light.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
#define TIME_SEPARATE	(3)		//時間の区切り
#define LIGHT_TYPE		(3)		//時間の区切り
#define CHANGE_ANGLE		(0.0005f)		//時間の区切り

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static LIGHT	g_Light[LIGHT_MAX];

static FOG		g_Fog;
//=============================================================================
// 初期化処理
//=============================================================================
void InitLight(void)
{

	//ライト初期化
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		g_Light[i].Position = XMFLOAT3(100.0f, 500.0f, 100.0f);
		g_Light[i].Direction = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Light[i].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_Light[i].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		g_Light[i].Attenuation = 100.0f;	// 減衰距離
		g_Light[i].Type = LIGHT_TYPE_NONE;	// ライトのタイプ
		g_Light[i].Enable = FALSE;			// ON / OFF
		SetLight(i, &g_Light[i]);
	}

	// 光源の設定（世界を照らす光）
	g_Light[0].Position = XMFLOAT3(500.0f, 500.0f, 500.0f);
	g_Light[0].Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
	g_Light[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// 並行光源
	g_Light[0].Enable = TRUE;									// このライトをON
	SetLight(0, &g_Light[0]);									// これで設定している



	// フォグの初期化（霧の効果）
	g_Fog.FogStart = 200.0f;									// 視点からこの距離離れるとフォグがかかり始める
	g_Fog.FogEnd   = 600.0f;									// ここまで離れるとフォグの色で見えなくなる
	g_Fog.FogColor = XMFLOAT4( 0.9f, 0.9f, 0.9f, 1.0f );		// フォグの色
	SetFog(&g_Fog);
	SetFogEnable(FALSE);		// 他の場所もチェックする shadow

}




//=============================================================================
// ライトの設定
// Typeによってセットするメンバー変数が変わってくる
//=============================================================================
void SetLightData(int index, LIGHT *light)
{
	SetLight(index, light);
}


LIGHT *GetLightData(int index)
{
	return(&g_Light[index]);
}


//=============================================================================
// フォグの設定
//=============================================================================
void SetFogData(FOG *fog)
{
	SetFog(fog);
}

//ライトの調整
void UpdateLight(void)
{
	PLAYER *player = GetPlayer();
	g_Light[0].Position = XMFLOAT3(player->pos.x + 0.0f, 500.0f, player->pos.z + 200.0f);
	g_Light[0].Direction = XMFLOAT3(0.5f, -1.0f, 0.0f);
	SetLight(0, &g_Light[0]);									// これで設定している
}

