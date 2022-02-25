//=============================================================================
//
// リザルト画面処理 [result.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "result.h"
#include "game.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "score.h"
#include "file.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(3)				// テクスチャの数

#define TEXTURE_WIDTH_LOGO			(480)			// ロゴサイズ
#define TEXTURE_HEIGHT_LOGO			(80)			// 

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/result001.jpg",
	"data/TEXTURE/ResultLogo.png",
	"data/TEXTURE/number16x32.png",
};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static float					g_DiffuseR, g_DiffuseG, g_DiffuseB;
static int						g_TexNo;					// テクスチャ番号
static int						g_ResultScore;				//リザルトに表示するスコア
static BOOL						g_Load = FALSE;
static BOOL						g_LoadData = FALSE;
static BOOL						g_SetScore = FALSE;
static int						g_ScoreNum = 0;
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitResult(void)
{
	ID3D11Device *pDevice = GetDevice();

	//テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// 変数の初期化
	g_Use   = TRUE;
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;
	g_Pos   = { g_w / 2, 50.0f, 0.0f };
	g_DiffuseR = 1.0f;
	g_DiffuseG = 1.0f;
	g_DiffuseB = 1.0f;
	g_TexNo = 0;
	g_ResultScore = 0;
	// BGM再生
	PlaySound(SOUND_LABEL_BGM_result);

	g_Load = TRUE;
	g_LoadData = FALSE;
	g_SetScore = FALSE;
	g_ScoreNum = SCORE_SAVE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitResult(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateResult(void)
{
	if (g_LoadData == FALSE)//ロードしていないならデータをロード
	{
		//スコアデータをロード
		LoadData();
		g_LoadData = TRUE;

	}

	if (g_SetScore == FALSE)//セーブをしていないならする
	{
		//スコアデータを更新
		int num = 0;
		int savenum = 0;
		int score = 0;		//次に保存するスコアを保存する用
		int savescore = 0;	//一時退避用
		for (int i = 0; i < SCORE_SAVE; i++)
		{
			if (GetScore() > GetSaveScore(i))	//ハイスコア更新箇所を上から探す
			{
				score = GetScore();
				savescore = GetSaveScore(i);
				SetSaveScore(score, i);
				g_ScoreNum = i;
				for (num = i + 1; num < SCORE_SAVE; num++)
				{
					savenum = num + 1;
					score = savescore;
					if (savenum < SCORE_SAVE)
						savescore = GetSaveScore(num);

					SetSaveScore(score, num);
				}
				break;
			}
		}
		g_SetScore = TRUE;
		SaveData();
	}
	if (GetKeyboardTrigger(DIK_RETURN) || IsButtonTriggered(0, BUTTON_START) || IsButtonTriggered(0, BUTTON_A))
	{// Enter押したら、ステージを切り替える
		if (GetScore() > GetHighScore())
		{
			SetHighScore(GetScore());
		}
		SetPlayMode(TITLE_NON);
		SetFade(FADE_OUT, MODE_GAME);
	}



#ifdef _DEBUG	// デバッグ情報を表示する
	
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawResult(void)
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// リザルトの背景を描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLeftTop(g_VertexBuffer, 0.0f, 0.0f, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// リザルトのロゴを描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}


	// スコア表示
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
		for (int k = 0; k < SCORE_SAVE; k++)
		{
			g_ResultScore = GetSaveScore(k);
			int number = g_ResultScore;
			for (int i = 0; i < SCORE_DIGIT; i++)
			{
				// 今回表示する桁の数字
				float x = (float)(number % 10);

				// スコアの位置やテクスチャー座標を反映
				float pw = 16 * 1.2f;			// スコアの表示幅
				float ph = 32 * 1.2f;			// スコアの表示高さ
				float px = (SCREEN_WIDTH * 0.5f + (pw * 0.5f) * ((float)(SCORE_DIGIT) * 0.5f)) - i * pw;		// スコアの表示位置X
				float py = 120.0f + (k * ph * 1.1f);// スコアの表示位置Y

				float tw = 1.0f / 10;		// テクスチャの幅
				float th = 1.0f / 1;		// テクスチャの高さ
				float tx = x * tw;			// テクスチャの左上X座標
				float ty = 0.0f;			// テクスチャの左上Y座標
				if (g_ScoreNum == k)
				{
					g_DiffuseR = 1.0f;
					g_DiffuseG = 1.0f;
					g_DiffuseB = 0.0f;
				}
				else
				{
					g_DiffuseR = 1.0f;
					g_DiffuseG = 1.0f;
					g_DiffuseB = 1.0f;
				}
				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
					XMFLOAT4(g_DiffuseR, g_DiffuseG, g_DiffuseB, 1.0f));

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);

				// 次の桁へ
				number /= 10;
			}
		}
	}
}




