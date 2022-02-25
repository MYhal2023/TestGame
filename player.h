//=============================================================================
//
// モデル処理 [player.h]
// Author : 
//
//=============================================================================
#pragma once
#include "model.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_PLAYER		"data/MODEL/slime.obj"			// 読み込むモデル名
#define MAX_PLAYER		(1)					// プレイヤーの数

#define	PLAYER_SIZE		(8.0f)				// 当たり判定の大きさ


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct PLAYER
{
	XMFLOAT3			pos;		// ポリゴンの位置
	XMFLOAT3			rot;		// ポリゴンの向き(回転)
	XMFLOAT3			scl;		// ポリゴンの大きさ(スケール)
	XMFLOAT3			moveVec;	//移動ベクトル。正規化して使用
	XMFLOAT4X4			mtxWorld;	// ワールドマトリックス

	BOOL				load;
	DX11_MODEL			model;		// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float				spd;		// 移動スピード
	float				dir;		// 向き
	float				size;		// 当たり判定の大きさ
	int					shadowIdx;	// 影のIndex
	BOOL				use;

	// 階層アニメーション用のメンバー変数
	INTERPOLATION_DATA	*tbl_adr;	// アニメデータのテーブル先頭アドレス
	int					tbl_size;	// 登録したテーブルのレコード総数
	float				move_time;	// 実行時間

	// 親は、NULL、子供は親のアドレスを入れる
	PLAYER				*parent;	// 自分が親ならNULL、自分が子供なら親のplayerアドレス

	// クォータニオン
	XMFLOAT4			quaternion;	// クォータニオン
	XMFLOAT3			upVector;	// 自分が立っている所

	//攻撃
	BOOL			attack;			//攻撃中か否か
	BOOL			attackUse;		//この攻撃判定が使用されたか

	int				life;			//プレイヤーHP
	int				lifeMax;		//プレイヤー最大HP

	BOOL			atInvinc;				//無敵時間中か否か
	float			atInvincTime;			//時間計測用
	float			blink;
	float			blinking;
	float			blinkingMax;
	float			blinkingCount;

};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

PLAYER *GetPlayer(void);
void SettingPlayer(void);
void IncibleEffect(void);
void PlayerMoveControl(void);