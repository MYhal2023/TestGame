//=============================================================================
//
// エネミーモデル処理 [enemy.cpp]
// Author : GP11B132 28 BURRUEL LEANOS GONZALO
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "renderer.h"
#include "camera.h"
#include "debugproc.h"
#include "input.h"
#include "model.h"
#include "enemy.h"
#include "enemybullet.h"
#include "player.h"
#include "shadow.h"
#include "particle.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_ENEMY			"data/MODEL/Eden.obj"		// 読み込むモデル名
//#define	MODEL_ENEMY_PARTS	"data/MODEL/ago2.obj"			// 読み込むモデル名

#define ENEMY_SHADOW_SIZE	(0.4f)						// 影の大きさ
#define ENEMY_OFFSET_Y		(7.0f)						// エネミーの足元をあわせる

#define ENEMY_ATTACK_MOVE	(20.0f)						//エネミーの攻撃の速さ

#define ENEMY_SPEED			(0.01f)						//エネミーの速さ


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];				// エネミー
static BOOL				g_Load = FALSE;



static INTERPOLATION_DATA move_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(SPAWN_POINT_X,SPAWN_POINT_Y,  SPAWN_POINT_Z), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE), 10*2 },
	{ XMFLOAT3(	  0.0f, 0.0f,  20.0f), XMFLOAT3(0.3f, XM_PI * 2, 0.0f), XMFLOAT3(ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE), 100*1 },
	{ XMFLOAT3(   0.0f, 0.0f,  20.0f), XMFLOAT3(0.3f, XM_PI * 2, 0.0f), XMFLOAT3(ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE), 50*1 },
	{ XMFLOAT3(SPAWN_POINT_X,SPAWN_POINT_Y,  SPAWN_POINT_Z), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE), 10*2 },

};

//static char *g_modelName[ENEMY_PARTS_MAX] = {
//	MODEL_ENEMY_PARTS,
//};

//時間
int					g_time;


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		LoadModel(MODEL_ENEMY, &g_Enemy[i].model);
		g_Enemy[i].load = TRUE;

		g_Enemy[i].pos = XMFLOAT3(SPAWN_POINT_X + i * 120.0f - 120.0f, SPAWN_POINT_Y, SPAWN_POINT_Z);
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		g_Enemy[i].spd = 0.0f;			// 移動スピードクリア
		g_Enemy[i].size = ENEMY_SIZE;	// 当たり判定の大きさ

		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Enemy[0].model, &g_Enemy[0].diffuse[0]);

		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		g_Enemy[i].shadowIdx = CreateShadow(pos, ENEMY_SHADOW_SIZE, ENEMY_SHADOW_SIZE);

		g_Enemy[i].move_time = 0.0f;	// 線形補間用のタイマーをクリア
		g_Enemy[i].tbl_adr = move_tbl;		// 再生するアニメデータの先頭アドレスをセット
		g_Enemy[i].tbl_size = sizeof(move_tbl) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Enemy[i].draw = TRUE;
		g_Enemy[i].use = TRUE;			// TRUE:生きてる
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].load)
		{
			UnloadModel(&g_Enemy[i].model);
			g_Enemy[i].load = FALSE;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateEnemy(void)
{
	g_time += 1;
	// エネミーを動かく場合は、影も合わせて動かす事を忘れないようにね！
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use != TRUE)	
			continue;					// このエネミーが使われている？
										//はい
		BulletAttack(i);				//弾を出す

		PLAYER p = *GetPlayer();		//プレイヤーが動いている？
		if (p.spd <= 0.1f)				
			continue;

		MoveEnemy(p.pos, i);			//はい。エネミーが動く

		//影
		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		SetPositionShadow(g_Enemy[i].shadowIdx,pos);


#ifdef _DEBUG	// デバッグ情報を表示する
		PrintDebugProc("Enemy:X:%f Y:%f Z:%f\n", g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
#endif

	}

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == FALSE || g_Enemy[i].draw == FALSE) continue;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);

		// モデル描画
		DrawModel(&g_Enemy[i].model);

	}

	// パーツの階層アニメーション




	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// エネミーのバレット攻撃
//とりあえず100フレーム経ったら弾を出す
//=============================================================================
void BulletAttack(int i)
{
	if (g_time % 100 == 0)
	{
		g_time = 0;										//大きな数字にならないため
		SetEnemyBullet(g_Enemy[i].pos, g_Enemy[i].rot);
	}
}

//=============================================================================
// エネミーの動き
//=============================================================================
void MoveEnemy(XMFLOAT3 playerpos, int i)
{

	float dirX = g_Enemy[i].pos.x - playerpos.x;
	float dirZ = g_Enemy[i].pos.z - playerpos.z;
	
	g_Enemy[i].moveVec = { dirX, 0.0f, dirZ };

	float angle = atan2f(g_Enemy[i].moveVec.x, g_Enemy[i].moveVec.z);
	g_Enemy[i].rot.y = angle;

	XMVECTOR	mVec = XMLoadFloat3(&g_Enemy[i].moveVec);
	XMVector3Normalize(mVec);

	XMVECTOR now = XMLoadFloat3(&g_Enemy[i].pos);								// 現在の場所
	XMStoreFloat3(&g_Enemy[i].pos, now - mVec * ENEMY_SPEED);	//単位ベクトルを元に移動

}




//=============================================================================
// エネミーの取得
//=============================================================================
ENEMY *GetEnemy()
{
	return &g_Enemy[0];
}