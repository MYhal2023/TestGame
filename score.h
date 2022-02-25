//=============================================================================
//
// スコア処理 [score.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define SCORE_MAX			(99999)		// スコアの最大値
#define SCORE_MIN			(0)			// スコアの最小値
#define SCORE_DIGIT			(7)			// 桁数
#define SCORE_SAVE			(10)		// スコアの保存数




//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitScore(void);
void UninitScore(void);
void UpdateScore(void);
void DrawScore(void);

void AddScore(int add);
int GetScore(void);
int GetHighScore(void);
int GetSaveScore(int i);
void SetScore(int score);
void SetHighScore(int score);
void SetSaveScore(int score, int i);