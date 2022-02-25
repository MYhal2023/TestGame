//=============================================================================
//
// ゲーム画面処理 [game.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "camera.h"
#include "light.h"
#include "input.h"
#include "sound.h"
#include "fade.h"
#include "game.h"

#include "player.h"
#include "meshfield.h"
#include "meshwall.h"
#include "shadow.h"
#include "score.h"
#include "interface.h"
#include "life.h"
#include "time.h"
#include "collision.h"
#include "debugproc.h"
#include "file.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************



//*****************************************************************************
// グローバル変数
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

static BOOL	g_bPause = TRUE;	// ポーズON/OFF
static int	g_PlayMode = MAIN_GAME;
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitGame(void)
{
	switch (GetMode())
	{
	case MODE_TITLE:
	case MODE_GAME:
		g_ViewPortType_Game = TYPE_FULL_SCREEN;


		// フィールドの初期化
		InitMeshField(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), FIELD_X, FIELD_Z, BLOCK_SIZE, BLOCK_SIZE, WATER);

		// ライトを有効化	// 影の初期化処理
		InitShadow();

		// プレイヤーの初期化
		InitPlayer();

		// 壁の初期化
		InitMeshWall(XMFLOAT3(0.0f, WAVE_POS_Y, FIELD_Z_LIMIT), XMFLOAT3(0.0f, 0.0f, 0.0f),
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), WALL_XZ, WALL_Y, WALL_BLOCK_SIZE_XZ, WALL_BLOCK_SIZE_Y);
		InitMeshWall(XMFLOAT3(-FIELD_X_LIMIT, WAVE_POS_Y, 0.0f), XMFLOAT3(0.0f, -XM_PI * 0.50f, 0.0f),
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), WALL_XZ, WALL_Y, WALL_BLOCK_SIZE_XZ, WALL_BLOCK_SIZE_Y);
		InitMeshWall(XMFLOAT3(FIELD_X_LIMIT, WAVE_POS_Y, 0.0f), XMFLOAT3(0.0f, XM_PI * 0.50f, 0.0f),
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), WALL_XZ, WALL_Y, WALL_BLOCK_SIZE_XZ, WALL_BLOCK_SIZE_Y);
		InitMeshWall(XMFLOAT3(0.0f, WAVE_POS_Y, -FIELD_Z_LIMIT), XMFLOAT3(0.0f, XM_PI, 0.0f),
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), WALL_XZ, WALL_Y, WALL_BLOCK_SIZE_XZ, WALL_BLOCK_SIZE_Y);

		// 壁(裏側用の半透明)
		InitMeshWall(XMFLOAT3(0.0f, WAVE_POS_Y, FIELD_Z_LIMIT), XMFLOAT3(0.0f, XM_PI, 0.0f),
			XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), WALL_XZ, WALL_Y, WALL_BLOCK_SIZE_XZ, WALL_BLOCK_SIZE_Y);
		InitMeshWall(XMFLOAT3(-FIELD_X_LIMIT, WAVE_POS_Y, 0.0f), XMFLOAT3(0.0f, XM_PI * 0.50f, 0.0f),
			XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), WALL_XZ, WALL_Y, WALL_BLOCK_SIZE_XZ, WALL_BLOCK_SIZE_Y);
		InitMeshWall(XMFLOAT3(FIELD_X_LIMIT, WAVE_POS_Y, 0.0f), XMFLOAT3(0.0f, -XM_PI * 0.50f, 0.0f),
			XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), WALL_XZ, WALL_Y, WALL_BLOCK_SIZE_XZ, WALL_BLOCK_SIZE_Y);
		InitMeshWall(XMFLOAT3(0.0f, WAVE_POS_Y, -FIELD_Z_LIMIT), XMFLOAT3(0.0f, 0.0f, 0.0f),
			XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), WALL_XZ, WALL_Y, WALL_BLOCK_SIZE_XZ, WALL_BLOCK_SIZE_Y);


		// スコアの初期化
		InitScore();

		// ライフの初期化
		InitLife();

		// 制限時間の初期化
		InitTime();

		//UI表示初期化
		InitInterface();



		////BGM再生
		//PlaySound(SOUND_LABEL_BGM_title);
		break;

	}

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitGame(void)
{

	//UI表示終了処理
	UninitInterface();

	// スコアの終了処理
	UninitScore();

	// ライフの終了処理
	UninitLife();

	// 制限時間の終了処理
	UninitTime();

	// 壁の終了処理
	UninitMeshWall();

	// 地面の終了処理
	UninitMeshField();


	// 影の終了処理
	UninitShadow();

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateGame(void)
{
#ifdef _DEBUG
	if (GetKeyboardTrigger(DIK_V))
	{
		g_ViewPortType_Game = (g_ViewPortType_Game + 1) % TYPE_NONE;
		SetViewPort(g_ViewPortType_Game);
	}

	if (GetKeyboardTrigger(DIK_P))
	{
		g_bPause = g_bPause ? FALSE : TRUE;
	}


#endif

	if(g_bPause == FALSE)
		return;
	PLAYER *player = GetPlayer();

	// 地面処理の更新
	UpdateMeshField();

	// 壁処理の更新
	UpdateMeshWall();

	// プレイヤーの更新処理
	UpdatePlayer();

	UpdateLight();

	// 影の更新処理
	UpdateShadow();


	if (g_PlayMode <= TUTORIAL)	//タイトルならここまで更新
		return;



	// スコアの更新処理
	UpdateScore();

	//UI表示更新処理
	UpdateInterface();

	// ライフの更新処理
	UpdateLife();

	// 制限時間の更新処理
	UpdateTime();
	

	CheckModeChange();

	////各オブジェクトを設置
	//SetCoin();
	//SetExten();
	//SetEnemy();
}

//=============================================================================
// 描画処理(カメラ目線)
//=============================================================================
void DrawGame0(void)
{
	PLAYER *player = GetPlayer();
	//シェーダー管理
	//ポストエフェクトをかける場合はここから
	int ans = MODE_PLANE;
	SwapShader(ans);

	// 3Dの物を描画する処理
	//シェーダー切り替え。水面の描画処理
	ans = MODE_SPECULAR;
	if (GetTime() <= 0 || player->life <= 0)
		ans = MODE_MONO;

	SwapShader(ans);
	// 地面の描画処理
	DrawMeshField();

	//シェーダー切り替え。ノーマルの物に戻す
	ans = MODE_PLANE;
	if (GetTime() <= 0 || player->life <= 0)
		ans = MODE_MONO;

	SwapShader(ans);

	// 影の描画処理
	DrawShadow();


	// プレイヤーの描画処理
	DrawPlayer();

	// 壁の描画処理
	DrawMeshWall();

	// 2Dの物を描画する処理
	// Z比較なし
	SetDepthEnable(FALSE);

	// ライティングを無効
	SetLightEnable(FALSE);

	if (g_PlayMode > TUTORIAL)
	{
		// スコアの描画処理
		DrawScore();

		// ライフの描画処理
		DrawLife();

		// 制限時間の描画処理
		DrawTime();

		////UI表示描画処理
		//DrawInterface();
	}

	//シェーダー管理
	//シェーダーを元に戻す。ポストエフェクトはここまで
	ans = MODE_PLANE;
	SwapShader(ans);
	if (GetTime() <= 0 || player->life <= 0)
		DrawGameOver();

	// ライティングを有効に
	SetLightEnable(TRUE);

	// Z比較あり
	SetDepthEnable(TRUE);
}

//=============================================================================
// 描画処理(ライト目線。生成したい影に関するオブジェクトだけを指定)
//=============================================================================
void DrawGame1(void)
{
	// 地面の描画処理
	DrawMeshField();


	// プレイヤーの描画処理
	DrawPlayer();

}


void DrawGame(void)
{


#ifdef _DEBUG
	// デバッグ表示
	PrintDebugProc("ViewPortType:%d\n", g_ViewPortType_Game);

#endif
	XMFLOAT3 pos;

	// プレイヤー視点
	pos = GetPlayer()->pos;
	//pos = GetEnemy()->pos;	//デバッグ用
	SetCameraAT(pos);
	SetCamera();

	switch(g_ViewPortType_Game)
	{
	case TYPE_FULL_SCREEN:
		SetViewPort(TYPE_FULL_SCREEN);
		DrawGame0();
		break;

	case TYPE_LEFT_HALF_SCREEN:
	case TYPE_RIGHT_HALF_SCREEN:
		SetViewPort(TYPE_LEFT_HALF_SCREEN);
		DrawGame0();

		// エネミー視点
		break;

	case TYPE_UP_HALF_SCREEN:
	case TYPE_DOWN_HALF_SCREEN:
		SetViewPort(TYPE_UP_HALF_SCREEN);
		DrawGame0();

		break;

	case TYPE_LIGHT_SCREEN:	//ライト目線の描画をするためのビューポートタイプ
		SetViewPort(TYPE_FULL_SCREEN);
		DrawGame1();
		break;

	}

}



void CheckModeChange(void)
{
	PLAYER *player = GetPlayer();
	if (GetPlayMode() == TUTORIAL_GAME)
	{
		if (GetKeyboardTrigger(DIK_X) || IsButtonTriggered(0, BUTTON_B))
		{
			SetFade(FADE_OUT, MODE_RESULT);
		}
	}


	//時間切れかライフ切れでリザルトへ
	if (GetTime() > 0 && player->life > 0)	
		return;

	if (GetKeyboardTrigger(DIK_RETURN) || IsButtonTriggered(0, BUTTON_A))
	{
		SetFade(FADE_OUT, MODE_RESULT);
	}
}

void SetPlayMode(int playMode)
{
	g_PlayMode = playMode;
}

int GetPlayMode(void)
{
	return g_PlayMode;
}

void SetViewPortType(int viewport)
{
	g_ViewPortType_Game = viewport;
}

int GetViewPortTypeGame(void)
{
	return g_ViewPortType_Game;
}