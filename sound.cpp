//=============================================================================
//
// サウンド処理 [sound.cpp]
//
//=============================================================================
#include "sound.h"
#include "debugproc.h"

//*****************************************************************************
// パラメータ構造体定義
//*****************************************************************************
typedef struct
{
	char *pFilename;	// ファイル名
	int nCntLoop;		// ループカウント
	BOOL UseFilter;		// エフェクト使うか
	int type;			// サウンドの種類
} SOUNDPARAM;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD *pChunkSize, DWORD *pChunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, void *pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
IXAudio2 *g_pXAudio2 = NULL;								// XAudio2オブジェクトへのインターフェイス
IXAudio2MasteringVoice *g_pMasteringVoice = NULL;			// マスターボイス
IXAudio2SourceVoice *g_apSourceVoice[SOUND_LABEL_MAX] = {};	// ソースボイス
BYTE *g_apDataAudio[SOUND_LABEL_MAX] = {};					// オーディオデータ
DWORD g_aSizeAudio[SOUND_LABEL_MAX] = {};					// オーディオデータサイズ
IXAudio2SubmixVoice *g_apDrySubmixVoice;					//サブミックスボイス(エフェクトなし)
IXAudio2SubmixVoice *g_apWetSubmixVoice;					//サブミックスボイス(エフェクトあり)

//エフェクト関連
//リバーブ
IUnknown *g_apXPO_Reverb;
XAUDIO2_EFFECT_DESCRIPTOR g_Descriptior[SOUND_EFFECT_MAX];
XAUDIO2_EFFECT_CHAIN g_Chain;

//リバーブのパラメータ
FXECHO_PARAMETERS g_ReverbParam;
XAUDIO2FX_REVERB_I3DL2_PARAMETERS g_i3dl2Param = XAUDIO2FX_I3DL2_PRESET_BATHROOM;
XAUDIO2FX_REVERB_PARAMETERS g_reverbParam;

//エコー
IUnknown *g_apXPO_Echo;

//エコーのパラメータ
FXECHO_PARAMETERS g_EchoParam;

//EQ
IUnknown *g_apXPO_EQ;

//EQのパラメータ
FXEQ_PARAMETERS g_EQParam;

#ifdef _DEBUG	// デバッグ時のみ
IUnknown *g_apXPO_Volmeter;
XAUDIO2_EFFECT_DESCRIPTOR g_Descriptor_Volmeter;
XAUDIO2_EFFECT_CHAIN g_Chain_Volmeter;
XAUDIO2FX_VOLUMEMETER_LEVELS g_Levels;
float g_PeakLevels[2];			//ピークレベルの受信場所
float g_RMSLevels[2];			//RMSレベルの受信場所
#endif

// 各音素材のパラメータ
SOUNDPARAM g_aParam[SOUND_LABEL_MAX] =
{
	{ (char*)"data/BGM/title.wav", -1 },		// タイトルBGM
	{ (char*)"data/BGM/game.wav", -1 },			// ゲームBGM1
	{ (char*)"data/BGM/result.wav", -1 },		// リザルトBGM2
	{ (char*)"data/SE/button.wav", 0 },			// ボタン音
	{ (char*)"data/SE/clock.wav", 0 },			// プレイヤー版時計音
	{ (char*)"data/SE/clock_enemy.wav", 0 },	// エネミー版時計音
	{ (char*)"data/SE/coin.wav", 0 },			// プレイヤー版コイン音
	{ (char*)"data/SE/coin_enemy.wav", 0 },		// エネミー版コイン音
	{ (char*)"data/SE/enemydown.wav", 0 },		// エネミー討伐音
	{ (char*)"data/SE/hit000.wav", 0 },			// ヒット音
};

//Fade関連
static BOOL			g_Use;									// TRUE:使っている  FALSE:未使用
SOUND_FADE			g_Fade = SOUNDFADE_NONE;				// フェードの状態
int					g_label;
float				g_NowVolume = 1.0f;
float				g_NextVolume;
float				g_FadeRate;

//音量管理
float				g_VolParam[SOUNDMAX];

//=============================================================================
// 初期化処理
//=============================================================================
BOOL InitSound(HWND hWnd)
{
	HRESULT hr;

	// COMライブラリの初期化
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// XAudio2オブジェクトの作成
	hr = XAudio2Create(&g_pXAudio2, 0);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "XAudio2オブジェクトの作成に失敗！", "警告！", MB_ICONWARNING);

		// COMライブラリの終了処理
		CoUninitialize();

		return FALSE;
	}
	
	// マスターボイスの生成
	hr = g_pXAudio2->CreateMasteringVoice(&g_pMasteringVoice);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "マスターボイスの生成に失敗！", "警告！", MB_ICONWARNING);

		if(g_pXAudio2)
		{
			// XAudio2オブジェクトの開放
			g_pXAudio2->Release();
			g_pXAudio2 = NULL;
		}

		// COMライブラリの終了処理
		CoUninitialize();

		return FALSE;
	}

	//サブミックスボイス（エフェクトあり）の作成
	hr = g_pXAudio2->CreateSubmixVoice(&g_apWetSubmixVoice, 2, 48000);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "サブミックス（エフェクトあり）の生成に失敗！", "警告！", MB_ICONWARNING);
		return FALSE;
	}

	//サブミックスボイス（エフェクトなし）の作成
	hr = g_pXAudio2->CreateSubmixVoice(&g_apDrySubmixVoice, 2, 48000);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "サブミックス（エフェクトなし）の生成に失敗！", "警告！", MB_ICONWARNING);
		return FALSE;
	}

	// サウンドデータの初期化
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		HANDLE hFile;
		DWORD dwChunkSize = 0;
		DWORD dwChunkPosition = 0;
		DWORD dwFiletype;
		WAVEFORMATEXTENSIBLE wfx;
		XAUDIO2_BUFFER buffer;

		// バッファのクリア
		memset(&wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));
		memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));

		// サウンドデータファイルの生成
		hFile = CreateFile(g_aParam[nCntSound].pFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(hWnd, "サウンドデータファイルの生成に失敗！(1)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		if(SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{// ファイルポインタを先頭に移動
			MessageBox(hWnd, "サウンドデータファイルの生成に失敗！(2)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
	
		// WAVEファイルのチェック
		hr = CheckChunk(hFile, 'FFIR', &dwChunkSize, &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "WAVEファイルのチェックに失敗！(1)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		hr = ReadChunkData(hFile, &dwFiletype, sizeof(DWORD), dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "WAVEファイルのチェックに失敗！(2)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		if(dwFiletype != 'EVAW')
		{
			MessageBox(hWnd, "WAVEファイルのチェックに失敗！(3)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
	
		// フォーマットチェック
		hr = CheckChunk(hFile, ' tmf', &dwChunkSize, &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "フォーマットチェックに失敗！(1)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		hr = ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "フォーマットチェックに失敗！(2)", "警告！", MB_ICONWARNING);
			return FALSE;
		}

		// オーディオデータ読み込み
		hr = CheckChunk(hFile, 'atad', &g_aSizeAudio[nCntSound], &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "オーディオデータ読み込みに失敗！(1)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		g_apDataAudio[nCntSound] = (BYTE*)malloc(g_aSizeAudio[nCntSound]);
		hr = ReadChunkData(hFile, g_apDataAudio[nCntSound], g_aSizeAudio[nCntSound], dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "オーディオデータ読み込みに失敗！(2)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
	
		// ソースボイスの生成
		hr = g_pXAudio2->CreateSourceVoice(&g_apSourceVoice[nCntSound], &(wfx.Format));
		if(FAILED(hr))
		{
			MessageBox(hWnd, "ソースボイスの生成に失敗！", "警告！", MB_ICONWARNING);
			return FALSE;
		}

		// バッファの値設定
		memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
		buffer.AudioBytes = g_aSizeAudio[nCntSound];
		buffer.pAudioData = g_apDataAudio[nCntSound];
		buffer.Flags      = XAUDIO2_END_OF_STREAM;
		buffer.LoopCount  = g_aParam[nCntSound].nCntLoop;

		// オーディオバッファの登録
		g_apSourceVoice[nCntSound]->SubmitSourceBuffer(&buffer);
	}

	//エフェクトの生成
	{
		//リバーブの生成
		XAudio2CreateReverb(&g_apXPO_Reverb);

		//EFFECT_DESCRIPTOR(リバーブ)の初期化
		g_Descriptior[SOUND_EFFECT_REVERB].InitialState = FALSE;			//無効状態に
		g_Descriptior[SOUND_EFFECT_REVERB].OutputChannels = 2;				//2chのエフェクト
		g_Descriptior[SOUND_EFFECT_REVERB].pEffect = g_apXPO_Reverb;		//エフェクト本体

		//エコーの生成
		CreateFX(_uuidof(FXEcho), &g_apXPO_Echo);

		//EFFECT_DESCRIPTOR(エコー)の初期化
		g_Descriptior[SOUND_EFFECT_ECHO].InitialState = FALSE;				//無効状態に
		g_Descriptior[SOUND_EFFECT_ECHO].OutputChannels = 2;				//2chのエフェクト
		g_Descriptior[SOUND_EFFECT_ECHO].pEffect = g_apXPO_Echo;			//エフェクト本体

		//EQの生成
		CreateFX(_uuidof(FXEQ), &g_apXPO_EQ);

		//EFFECT_DESCRIPTOR(EQ)の初期化
		g_Descriptior[SOUND_EFFECT_EQ].InitialState = FALSE;				//無効状態に
		g_Descriptior[SOUND_EFFECT_EQ].OutputChannels = 2;					//2chのエフェクト
		g_Descriptior[SOUND_EFFECT_EQ].pEffect = g_apXPO_EQ;				//エフェクト本体

		//EFFECT_CHAINの作成
		g_Chain.EffectCount = SOUND_EFFECT_MAX;								//挿すのはSOUND_EFFECT_MAX分
		g_Chain.pEffectDescriptors = g_Descriptior;							//さっきの構造体を指示

		//ボイスにEFFECT_CHAINを挿す
		g_apWetSubmixVoice->SetEffectChain(&g_Chain);

		//Release
		g_apXPO_Reverb->Release();
		g_apXPO_Echo->Release();
		g_apXPO_EQ->Release();
	}

	//パラメーターの初期化
	{
		// リバーブ
		// I3DL2_REVERB_PARAM 変換
		ReverbConvertI3DL2ToNative(&g_i3dl2Param, &g_reverbParam);

		//エフェクトに通知する
		g_apWetSubmixVoice->SetEffectParameters(SOUND_EFFECT_REVERB, &g_reverbParam, sizeof(g_reverbParam));

		// エコー
		g_EchoParam.WetDryMix = FXECHO_DEFAULT_WETDRYMIX;
		g_EchoParam.Delay = FXECHO_DEFAULT_DELAY;
		g_EchoParam.Feedback = FXECHO_DEFAULT_FEEDBACK;

		//エフェクトに通知する
		g_apWetSubmixVoice->SetEffectParameters(SOUND_EFFECT_ECHO, &g_EchoParam, sizeof(g_EchoParam));

		//EQ
		g_EQParam.FrequencyCenter0 = FXEQ_DEFAULT_FREQUENCY_CENTER_0;
		g_EQParam.FrequencyCenter1 = FXEQ_DEFAULT_FREQUENCY_CENTER_1;
		g_EQParam.FrequencyCenter2 = FXEQ_DEFAULT_FREQUENCY_CENTER_2;
		g_EQParam.FrequencyCenter3 = FXEQ_DEFAULT_FREQUENCY_CENTER_3;

		g_EQParam.Bandwidth0 = FXEQ_DEFAULT_BANDWIDTH;
		g_EQParam.Bandwidth1 = FXEQ_DEFAULT_BANDWIDTH;
		g_EQParam.Bandwidth2 = FXEQ_DEFAULT_BANDWIDTH;
		g_EQParam.Bandwidth3 = FXEQ_DEFAULT_BANDWIDTH;

		g_EQParam.Gain0 = FXEQ_DEFAULT_GAIN;
		g_EQParam.Gain1 = FXEQ_DEFAULT_GAIN;
		g_EQParam.Gain2 = FXEQ_DEFAULT_GAIN;
		g_EQParam.Gain3 = FXEQ_DEFAULT_GAIN;

		//エフェクトに通知する
		g_apWetSubmixVoice->SetEffectParameters(SOUND_EFFECT_EQ, &g_EQParam, sizeof(g_EQParam));
	}

	//音量の初期化
	{
		g_VolParam[MASTER] = 1.0f;
		g_VolParam[BGM] = 0.75f;
		g_VolParam[SE] = 0.8f;
		g_VolParam[VOICE] = 1.6f;
	}

#ifdef _DEBUG	// デバッグ時のみ
	hr = XAudio2CreateVolumeMeter(&g_apXPO_Volmeter);
	g_Descriptor_Volmeter.InitialState = true;
	g_Descriptor_Volmeter.OutputChannels = 2;
	g_Descriptor_Volmeter.pEffect = g_apXPO_Volmeter;

	g_Chain_Volmeter.EffectCount = 1;
	g_Chain_Volmeter.pEffectDescriptors = &g_Descriptor_Volmeter;

	g_pMasteringVoice->SetEffectChain(&g_Chain_Volmeter);

	g_apXPO_Volmeter->Release();
#endif

	// Fade関連の初期化
	g_Use = FALSE;
	g_Fade = SOUNDFADE_NONE;
	g_FadeRate = 0.0f;

	return TRUE;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitSound(void)
{
	// 一時停止
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if(g_apSourceVoice[nCntSound])
		{
			// 一時停止
			g_apSourceVoice[nCntSound]->Stop(0);
	
			// ソースボイスの破棄
			g_apSourceVoice[nCntSound]->DestroyVoice();
			g_apSourceVoice[nCntSound] = NULL;
	
			// オーディオデータの開放
			free(g_apDataAudio[nCntSound]);
			g_apDataAudio[nCntSound] = NULL;
		}
	}
	
	// マスターボイスの破棄
	g_pMasteringVoice->DestroyVoice();
	g_pMasteringVoice = NULL;
	
	// サブミックスボイスの破棄
	g_apWetSubmixVoice->DestroyVoice();
	g_apDrySubmixVoice->DestroyVoice();

	if(g_pXAudio2)
	{
		// XAudio2オブジェクトの開放
		g_pXAudio2->Release();
		g_pXAudio2 = NULL;
	}
	
	// COMライブラリの終了処理
	CoUninitialize();
}

//=============================================================================
// 更新処理（主にFadeで使用）
//=============================================================================
void UpdateSound(void)
{
	if (g_Fade != SOUNDFADE_NONE)
	{// フェード処理中
		if (g_Fade == SOUNDFADE_OUT)
		{// フェードアウト処理
			g_NowVolume -= g_FadeRate;		// 減算
			if (g_NowVolume > g_NextVolume)
			{
				g_apSourceVoice[g_label]->SetVolume(g_NowVolume);
			}
			else
			{
				// フェードアウト処理終了
				g_apSourceVoice[g_label]->SetVolume(0.0f);
				g_apSourceVoice[g_label]->Stop();
				g_Fade = SOUNDFADE_NONE;
			}
		}

		else if (g_Fade == SOUNDFADE_IN)
		{// フェードイン処理
			g_NowVolume += g_FadeRate;		// 加算
			g_apSourceVoice[g_label]->SetVolume(g_NowVolume);

			if (g_NowVolume >= g_NextVolume)
			{
				// フェードイン処理終了
				g_Fade = SOUNDFADE_NONE;
			}
		}
	}
	//if (GetOption())
	//{
	//	//音量調整
	//	{
	//		XAUDIO2_VOICE_STATE xa2state;

	//		for (int i = 0; i < SOUND_LABEL_MAX; i++)
	//		{
	//			// 状態取得
	//			g_apSourceVoice[i]->GetState(&xa2state);
	//			if (xa2state.BuffersQueued != 0)
	//			{// 再生中
	//				// 音量調整
	//				g_apSourceVoice[i]->SetVolume(g_VolParam[g_aParam[i].type]);
	//			}
	//		}
	//	}
	//}

}

//=============================================================================
// SoundFade処理
//=============================================================================
void SetSoundFade(int label, SOUND_FADE fademode, float TargetVolume, float TargetTime)
{
	g_Fade = fademode;
	g_label = label;
	g_NextVolume = TargetVolume;
	g_FadeRate = fabsf(TargetVolume - g_NowVolume) / (TargetTime * 60);
	return;
}

//=============================================================================
// セグメント再生(再生中なら停止)
//=============================================================================
void PlaySound(int label)
{
	XAUDIO2_VOICE_STATE xa2state;
	XAUDIO2_BUFFER buffer;

	// バッファの値設定
	memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
	buffer.AudioBytes = g_aSizeAudio[label];
	buffer.pAudioData = g_apDataAudio[label];
	buffer.Flags      = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount  = g_aParam[label].nCntLoop;

	// 状態取得
	g_apSourceVoice[label]->GetState(&xa2state);
	if(xa2state.BuffersQueued != 0)
	{// 再生中
		// 一時停止
		g_apSourceVoice[label]->Stop(0);

		// オーディオバッファの削除
		g_apSourceVoice[label]->FlushSourceBuffers();
	}

	if (g_aParam[label].UseFilter == TRUE)
	{
		//サブミックスボイスに送信
		XAUDIO2_SEND_DESCRIPTOR Send[2] = { 0, g_apWetSubmixVoice,
											0, g_apDrySubmixVoice };
		XAUDIO2_VOICE_SENDS SendList = { 2, Send };
		g_apSourceVoice[label]->SetOutputVoices(&SendList);
	}

	// オーディオバッファの登録
	g_apSourceVoice[label]->SubmitSourceBuffer(&buffer);

	// 再生
	g_apSourceVoice[label]->Start(0);

}

//=============================================================================
// セグメント停止(ラベル指定)
//=============================================================================
void StopSound(int label)
{
	XAUDIO2_VOICE_STATE xa2state;

	// 状態取得
	g_apSourceVoice[label]->GetState(&xa2state);
	if(xa2state.BuffersQueued != 0)
	{// 再生中
		// 一時停止
		g_apSourceVoice[label]->Stop(0);

		// オーディオバッファの削除
		g_apSourceVoice[label]->FlushSourceBuffers();
	}
}

//=============================================================================
// セグメント停止(全て)
//=============================================================================
void StopSound(void)
{
	// 一時停止
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if(g_apSourceVoice[nCntSound])
		{
			// 一時停止
			g_apSourceVoice[nCntSound]->Stop(0);
		}
	}
}

//=============================================================================
// チャンクのチェック
//=============================================================================
HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD *pChunkSize, DWORD *pChunkDataPosition)
{
	HRESULT hr = S_OK;
	DWORD dwRead;
	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD dwBytesRead = 0;
	DWORD dwOffset = 0;
	
	if(SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{// ファイルポインタを先頭に移動
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	while(hr == S_OK)
	{
		if(ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL) == 0)
		{// チャンクの読み込み
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if(ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL) == 0)
		{// チャンクデータの読み込み
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		switch(dwChunkType)
		{
		case 'FFIR':
			dwRIFFDataSize  = dwChunkDataSize;
			dwChunkDataSize = 4;
			if(ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL) == 0)
			{// ファイルタイプの読み込み
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			break;

		default:
			if(SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			{// ファイルポインタをチャンクデータ分移動
				return HRESULT_FROM_WIN32(GetLastError());
			}
		}

		dwOffset += sizeof(DWORD) * 2;
		if(dwChunkType == format)
		{
			*pChunkSize         = dwChunkDataSize;
			*pChunkDataPosition = dwOffset;

			return S_OK;
		}

		dwOffset += dwChunkDataSize;
		if(dwBytesRead >= dwRIFFDataSize)
		{
			return S_FALSE;
		}
	}
	
	return S_OK;
}

//=============================================================================
// チャンクデータの読み込み
//=============================================================================
HRESULT ReadChunkData(HANDLE hFile, void *pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset)
{
	DWORD dwRead;
	
	if(SetFilePointer(hFile, dwBufferoffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{// ファイルポインタを指定位置まで移動
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if(ReadFile(hFile, pBuffer, dwBuffersize, &dwRead, NULL) == 0)
	{// データの読み込み
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	return S_OK;
}

//=============================================================================
// マスターボイスの音量調整
//=============================================================================
void SetMasterVoice(float volume)
{
	volume *= volume;
	g_pMasteringVoice->SetVolume(volume);
	return;
}


//=============================================================================
// ソースボイスの音量調整
//=============================================================================
void SetSourceVolume(int label, float volume)
{
	volume *= volume;
	g_apSourceVoice[label]->SetVolume(volume);
	return;
}


//=============================================================================
// オプション用音量調整
//=============================================================================
void SetVolumeOption(float volume1, float volume2)
{
	volume1 *= volume1;
	volume2 *= volume2;

	g_VolParam[BGM] = volume1;
	g_VolParam[SE] = volume2;
	return;
}


//=============================================================================
// Panning(音響)調整
//=============================================================================
void SetSourceSoundPanVolume(int label)
{
	int InChannels;
	int OutChannels;

	InChannels = 2;
	OutChannels = 2;

	float Volumes[2] = { 1.0f, 0.0f };
	g_apSourceVoice[label]->SetOutputMatrix(NULL, InChannels, OutChannels, Volumes);
	return;
}


//=============================================================================
// ソースボイスのボイスの再生ピッチ調整
//=============================================================================
void SetFrequencyRatio(int label, float Pitch)
{
	g_apSourceVoice[label]->SetFrequencyRatio(Pitch);
	return;
}


//=============================================================================
// ソースボイスの一時停止
//=============================================================================
void PauseSound(int label)
{
	g_apSourceVoice[label]->Stop(XAUDIO2_PLAY_TAILS);
	return;
}


//=============================================================================
// ソースボイスの再開
//=============================================================================
void ReStartSound(int label)
{
	g_apSourceVoice[label]->Start();
	return;
}


//=============================================================================
// ローパス、ハイパスフィルターのON
//=============================================================================
void PassEffectON(int label, XAUDIO2_FILTER_TYPE type)
{
	XAUDIO2_FILTER_PARAMETERS FilterParams;
	FilterParams.Type = type;
	FilterParams.Frequency = 0.5f;
	FilterParams.OneOverQ = 1.4142f;
	g_apSourceVoice[label]->SetFilterParameters(&FilterParams);
	return;
}


//=============================================================================
// ローパス、ハイパスフィルターのOFF
//=============================================================================
void PassEffectOFF(int label)
{
	XAUDIO2_FILTER_PARAMETERS FilterParams;
	FilterParams.Type = XAUDIO2_FILTER_TYPE::LowPassFilter;
	FilterParams.Frequency = 1.0f;
	FilterParams.OneOverQ = 1.0f;
	g_apSourceVoice[label]->SetFilterParameters(&FilterParams);
	return;
}

//=============================================================================
// リバーブのパラメータ調整
//=============================================================================
void SetReverbParam(XAUDIO2FX_REVERB_I3DL2_PARAMETERS type)
{
	//デフォルトのやつから引っ張ってくる
	g_i3dl2Param = type;

	// I3DL2_REVERB_PARAM 変換
	ReverbConvertI3DL2ToNative(&g_i3dl2Param, &g_reverbParam);

	//エフェクトに通知
	g_apWetSubmixVoice->SetEffectParameters(SOUND_EFFECT_REVERB, &g_reverbParam, sizeof(g_reverbParam));

	//細かく調整したかったら
	//XAUDIO2FX_REVERB_PARAMETERS reverbParameters;
	//reverbParameters.ReflectionsDelay = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_DELAY;
	//reverbParameters.ReverbDelay = XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY;
	//reverbParameters.RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY;
	//reverbParameters.PositionLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION;
	//reverbParameters.PositionRight = XAUDIO2FX_REVERB_DEFAULT_POSITION;
	//reverbParameters.PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
	//reverbParameters.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
	//reverbParameters.EarlyDiffusion = XAUDIO2FX_REVERB_DEFAULT_EARLY_DIFFUSION;
	//reverbParameters.LateDiffusion = XAUDIO2FX_REVERB_DEFAULT_LATE_DIFFUSION;
	//reverbParameters.LowEQGain = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN;
	//reverbParameters.LowEQCutoff = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF;
	//reverbParameters.HighEQGain = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN;
	//reverbParameters.HighEQCutoff = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF;
	//reverbParameters.RoomFilterFreq = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ;
	//reverbParameters.RoomFilterMain = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN;
	//reverbParameters.RoomFilterHF = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF;
	//reverbParameters.ReflectionsGain = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN;
	//reverbParameters.ReverbGain = XAUDIO2FX_REVERB_DEFAULT_REVERB_GAIN;
	//reverbParameters.DecayTime = XAUDIO2FX_REVERB_DEFAULT_DECAY_TIME;
	//reverbParameters.Density = XAUDIO2FX_REVERB_DEFAULT_DENSITY;
	//reverbParameters.RoomSize = XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE;
	//reverbParameters.WetDryMix = XAUDIO2FX_REVERB_DEFAULT_WET_DRY_MIX;
	//g_apWetSubmixVoice->SetEffectParameters(0, &reverbParameters, sizeof(reverbParameters));
	return;
}


//=============================================================================
// リバーブのパラメータ調整（かかりぐあい）
//=============================================================================
void ChangeReverb(float depth)
{
	float WelVolume = 0.5f * depth * depth;
	if (depth >= 1.0f)
	{
		float WelVolume = 1.0f;
	}
	float DryVolume = 1.0f - WelVolume;

	g_apWetSubmixVoice->SetVolume(WelVolume);
	g_apDrySubmixVoice->SetVolume(DryVolume);

	return;
}


//=============================================================================
// エコーのパラメータ調整
//=============================================================================
void SetEchoParam(float wetdrymix, float delay, float feedback)
{
	g_apWetSubmixVoice->EnableEffect(SOUND_EFFECT_ECHO);

	//パラメーターの初期化
	g_EchoParam.WetDryMix = wetdrymix;
	g_EchoParam.Delay = delay;
	g_EchoParam.Feedback = feedback;

	//エフェクトに通知する
	g_apWetSubmixVoice->SetEffectParameters(SOUND_EFFECT_ECHO, &g_EchoParam, sizeof(g_EchoParam));
}


//=============================================================================
// EQのパラメータ調整
//=============================================================================
void SetEQParam(float c0, float c1, float c2, float c3, float b0, float b1, float b2, float b3, float g0, float g1, float g2, float g3)
{
	g_EQParam.FrequencyCenter0 = c0;
	g_EQParam.FrequencyCenter1 = c1;
	g_EQParam.FrequencyCenter2 = c2;
	g_EQParam.FrequencyCenter3 = c3;

	g_EQParam.Bandwidth0 = b0;
	g_EQParam.Bandwidth1 = b1;
	g_EQParam.Bandwidth2 = b2;
	g_EQParam.Bandwidth3 = b3;

	g_EQParam.Gain0 = g0;
	g_EQParam.Gain1 = g1;
	g_EQParam.Gain2 = g2;
	g_EQParam.Gain3 = g3;

	//エフェクトに通知する
	g_apWetSubmixVoice->SetEffectParameters(SOUND_EFFECT_EQ, &g_EQParam, sizeof(g_EQParam));
	return;
}


//=============================================================================
// 最大音量の調整
//=============================================================================
void MasteringLimit(void)
{
	IUnknown *apXPO_FXMLimit;
	CreateFX(_uuidof(FXMasteringLimiter), &apXPO_FXMLimit);

	XAUDIO2_EFFECT_DESCRIPTOR Descriptor_FXMLimit;
	Descriptor_FXMLimit.InitialState = true;
	Descriptor_FXMLimit.OutputChannels = 2;
	Descriptor_FXMLimit.pEffect = apXPO_FXMLimit;

	XAUDIO2_EFFECT_CHAIN Chain_FXMLimit;
	Chain_FXMLimit.EffectCount = 1;
	Chain_FXMLimit.pEffectDescriptors = &Descriptor_FXMLimit;

	g_pMasteringVoice->SetEffectChain(&Chain_FXMLimit);

	apXPO_FXMLimit->Release();

	FXMASTERINGLIMITER_PARAMETERS FXMLimit;

	FXMLimit.Loudness = FXMASTERINGLIMITER_DEFAULT_LOUDNESS;
	FXMLimit.Release = FXMASTERINGLIMITER_DEFAULT_RELEASE;

	return;
}


//=============================================================================
// エフェクトスイッチ
//=============================================================================
void EffectSwitch(SOUND_EFFECT effect, BOOL flag)
{
	if (flag == FALSE)
	{
		g_apWetSubmixVoice->DisableEffect(effect);
	}
	else
	{
		g_apWetSubmixVoice->EnableEffect(effect);
	}
}


#ifdef _DEBUG	// デバッグ情報を表示する
//=============================================================================
// マスター音量を取得
//=============================================================================
void GetVolMeter(void)
{
	g_Levels.pPeakLevels = g_PeakLevels;
	g_Levels.pRMSLevels = g_RMSLevels;
	g_Levels.ChannelCount = 2;

	g_pMasteringVoice->GetEffectParameters(0, &g_Levels, sizeof(g_Levels));
	return;
}
#endif
