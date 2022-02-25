//=============================================================================
//
// サウンド処理 [sound.h]
//
//=============================================================================
#pragma once
#pragma comment(lib, "XAPOFX.lib")

#include <windows.h>
#include "xaudio2.h"						// サウンド処理で必要
#include "xaudio2fx.h"
#include <XAPOFX.h>

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define BGM_VOLUME (0.7f)			//BGMの音量
#define MAX_VOLUME (1.6f)			//BGMのMAX音量
#define MIN_VOLUME (0.0f)			//BGMのMIN音量

// フェードの状態
typedef enum
{
	SOUNDFADE_NONE = 0,		// 何もない状態
	SOUNDFADE_IN,			// フェードイン処理
	SOUNDFADE_OUT,			// フェードアウト処理
} SOUND_FADE;

// 使用するエフェクト
typedef enum
{
	SOUND_EFFECT_REVERB = 0,		// リバーブ
	SOUND_EFFECT_ECHO,				// エコー
	SOUND_EFFECT_EQ,				// イコライザー
	SOUND_EFFECT_MAX,				// エフェクトのMAX数
} SOUND_EFFECT;

// 使用するサウンドの種類
enum 
{
	MASTER,
	BGM,
	SE,
	VOICE,
	SOUNDMAX,				// サウンド種類のMAX数
};

// サウンドファイル
enum 
{
	SOUND_LABEL_BGM_title,		// BGM0
	SOUND_LABEL_BGM_game,		// 
	SOUND_LABEL_BGM_result,		// 
	SOUND_LABEL_SE_button,		// SE
	SOUND_LABEL_SE_clock,		// 
	SOUND_LABEL_SE_clock_enemy,	// 
	SOUND_LABEL_SE_coin,		// 
	SOUND_LABEL_SE_coin_enemy,	// 
	SOUND_LABEL_SE_enemydown,	// 
	SOUND_LABEL_SE_hit000,		// 
	SOUND_LABEL_MAX,
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
BOOL InitSound(HWND hWnd);
void UninitSound(void);
void PlaySound(int label);
void StopSound(int label);
void StopSound(void);
void UpdateSound(void);
void SetSourceSoundPanVolume(int label);
void SetSoundFade(int label, SOUND_FADE soundfade, float TargetVolume, float TargetTime);
void SetSourceVolume(int label, float volume);
void SetFrequencyRatio(int label, float Pitch);
void PauseSound(int label);
void ReStartSound(int label);
void PassEffectON(int label, XAUDIO2_FILTER_TYPE type);
void PassEffectOFF(int label);
void SetReverbParam(XAUDIO2FX_REVERB_I3DL2_PARAMETERS type);
void ChangeReverb(float volume);
void SetEchoParam(float wetdrymix, float delay, float feedback);
void SetEQParam(float c0, float c1, float c2, float c3, float b0, float b1, float b2, float b3, float g0, float g1, float g2, float g3);
void MasteringLimit(void);
void EffectSwitch(SOUND_EFFECT effect, BOOL flag);
void SetMasterVoice(float Volume);
void SetVolumeOption(float volume1, float volume2);