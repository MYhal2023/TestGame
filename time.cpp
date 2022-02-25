//=============================================================================
//
// 制限時間処理 [time.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "score.h"
#include "sprite.h"
#include "time.h"
#include "fade.h"
#include "game.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(32)	// キャラサイズ
#define TEXTURE_HEIGHT				(64)	// 
#define TEXTURE_MAX					(2)		// テクスチャの数
#define TIME_CENT					(2)		// 分と秒の単位の間にある:
#define TIME_MINUTES				(2)		// 分単位の数:
#define TIME_SECONDS				(2)		// 秒単位の数:
#define TIME_CENT					(2)		// 分と秒の単位の間にある:
#define ONE_SECOND					(60)	// 一秒にかかるフレームレート数と、一分にかかる秒数
#define SCORE_UP_INTERVAL			(5)	// スコアの増加係数を増やす間隔


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/number16x32.png",
	"data/TEXTURE/timer_cent.png",
};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

static int						g_Time;						// 制限時間
static int						second;						//秒数経過の指標。ここでは60fps
static BOOL						g_Load = FALSE;


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTime(void)
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


	// プレイヤーの初期化
	g_Use   = TRUE;
	g_w     = TIME_TEXTURE_WIDTH;
	g_h     = TIME_TEXTURE_HEIGHT;
	g_Pos   = { 150.0f, 20.0f, 0.0f };
	g_TexNo = 0;

	g_Time = 60;	// 制限時間の初期化
#ifdef DEBUG
	g_Time = 1000
#endif 

	second = 0;
	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTime(void)
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
void UpdateTime(void)
{
	RemoveTime(1);

#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTime(void)
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

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

	// 桁数分処理する
	int number = g_Time;
	for (int i = 0; i < TIME_DIGIT; i++)
	{
		if (i != 1 && i != TIME_CENT)	//十進数の処理
		{
			g_TexNo = NUMBER;
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);
			// 今回表示する桁の数字
			float x = (float)(number % 10);

			// 制限時間の位置やテクスチャー座標を反映
			float px = g_Pos.x - g_w * i;	// 制限時間の表示位置X
			float py = g_Pos.y;			// 制限時間の表示位置Y
			float pw = g_w;				// 制限時間の表示幅
			float ph = g_h;				// 制限時間の表示高さ

			float tw = 1.0f / 10;		// テクスチャの幅
			float th = 1.0f / 1;		// テクスチャの高さ
			float tx = x * tw;			// テクスチャの左上X座標
			float ty = 0.0f;			// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

			// 次の桁へ
			number /= 10;

		}
		else if (i == 1)	//時間の二桁目のみ6進数なため、処理を分ける
		{
			g_TexNo = NUMBER;
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);
			// 今回表示する桁の数字
			float x = (float)(number % 6);

			// 制限時間の位置やテクスチャー座標を反映
			float px = g_Pos.x - g_w * i;	// 制限時間の表示位置X
			float py = g_Pos.y;			// 制限時間の表示位置Y
			float pw = g_w;				// 制限時間の表示幅
			float ph = g_h;				// 制限時間の表示高さ

			float tw = 1.0f / 10;		// テクスチャの幅
			float th = 1.0f / 1;		// テクスチャの高さ
			float tx = x * tw;			// テクスチャの左上X座標
			float ty = 0.0f;			// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

			// 次の桁へ
			number /= 6;


		}
		else if (i == TIME_CENT)
		{
			g_TexNo = CENT;

			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

			// 制限時間の位置やテクスチャー座標を反映
			float px = g_Pos.x - g_w * i;			// 制限時間の表示位置X
			float py = g_Pos.y;			// 制限時間の表示位置Y
			float pw = g_w;				// 制限時間の表示幅
			float ph = g_h;				// 制限時間の表示高さ

			float tw = 1.0f;		// テクスチャの幅
			float th = 1.0f;		// テクスチャの高さ
			float tx = 0.0f;			// テクスチャの左上X座標
			float ty = 0.0f;			// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

		}
	}	
}


//=============================================================================
// 制限時間を減算する(1秒経過で1ずつ減っていく)
// 引数:remove 時間をマイナスしていく
//=============================================================================
void RemoveTime(int remove)
{
	if (GetPlayMode() == TUTORIAL_GAME)return;	//チュートリアルはスコア加算無し

	second++;
	if (second >= ONE_SECOND)
	{
		g_Time -= remove;
		second = 0;
		int k = 1;
		int count = g_Time;
		while (count > 0)
		{
			count -= SCORE_UP_INTERVAL;
			k++;
		}
		AddScore(1 * k);
		if (g_Time <= 0)
			g_Time = 0;
	}

}

//=============================================================================
// 制限時間を加算する
// 引数:add 時間をプラスしていく。制限時間が0ならば加算処理を行わない
//=============================================================================
void AddTime(int add)
{
	if (GetPlayMode() == TUTORIAL_GAME)return;	//チュートリアルはスコア加算無し

	if (g_Time > 0)
	{
		g_Time += add;

		if (g_Time > TIME_MAX)
			g_Time = TIME_MAX;

	}
}

int GetTime(void)
{
	return g_Time;
}