//=============================================================================
//
// ライフ処理 [life.h]
// Author : GP11B132 34 米倉 睦起
//
//=============================================================================
#pragma once

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define LIFE_DIGIT			(2)		// ハート一つにライフがいくつ必要なのか

#define	LIFE_DX			(72.0f)		// 表示位置
#define	LIFE_DY			(76.0f)		// 
#define	LIFE_DW			(32.0f)		// 
#define	LIFE_DH			(32.0f)		// 

#define	LIFE_SCL		(1.0f)					// LIFE文の倍率
#define	LIFE_BW			(57.0f * LIFE_SCL)		// LIFE文の横幅
#define	LIFE_BH			(17.0f * LIFE_SCL)		// 縦幅
#define	GAMEOVER_DW		(431.0f)				// テクスチャの横幅
#define	GAMEOVER_DH		(60.0f)					//テクスチャの縦幅
//ライフ判別
enum
{
	PLAYER_LIFE,
	ENEMY_LIFE,
	CLONE_LIFE
};
//テクスチャナンバー
enum
{
	HART,
	HART_LOST,
	HART_BOS,
	GAMEOVER
};
struct LIFE
{
	XMFLOAT3		pos;		// ポリゴンの座標
	float			w, h;		// 幅と高さ
	int				texNo;		// 使用しているテクスチャ番号

	int				life;		// ライフ
	int				life_max;	//ライフの最大値
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitLife(void);
void UninitLife(void);
void UpdateLife(void);
void DrawLife(void);
void DrawGameOver(void);
int GetLife(void);
void AddLife(int add, int chara, int i);
void SetLife(int life);