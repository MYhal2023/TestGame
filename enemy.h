//=============================================================================
//
// エネミーモデル処理 [enemy.h]
// Author : GP11B132 28 BURRUEL LEANOS GONZALO
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_ENEMY		(3)					// エネミーの数

#define	ENEMY_SIZE		(0.6f)				// 当たり判定の大きさ

#define TUTORIAL_ENEMY	(3)
#define STAGE_1_ENEMY	(10)


#define SPAWN_POINT_X			(0.0f)					//エネミーがどこに始めるか
#define SPAWN_POINT_Y			(10.0f)					//エネミーがどこに始めるか
#define SPAWN_POINT_Z			(20.0f)					//エネミーがどこに始めるか

//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct ENEMY
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)

	BOOL				use;
	BOOL				load;
	BOOL				draw;				//描くかどうか
	DX11_MODEL			model;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float				spd;				// 移動スピード
	float				size;				// 当たり判定の大きさ
	int					shadowIdx;			// 影のインデックス番号

	int					type;				//エネミーの違い
	int					track;				//エネミーがどこに攻撃する
	int					attcnt,attcntmax;	//エネミーが何回攻撃するか

	
	INTERPOLATION_DATA	*tbl_adr;			// アニメデータのテーブル先頭アドレス
	int					tbl_size;			// 登録したテーブルのレコード総数
	float				move_time;			// 実行時間

	XMFLOAT3			moveVec;			//動くベクトル
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);
XMFLOAT3 MoveEnemy(XMFLOAT3 playerpos, int i);

ENEMY *GetEnemy(void);
