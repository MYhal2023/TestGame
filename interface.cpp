//=============================================================================
//
// その他UI表示処理 [interface.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "interface.h"
#include "sprite.h"
#include "game.h"
#include "time.h"
#include "player.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(1830 * 0.20)	// テクスチャサイズ
#define TEXTURE_HEIGHT				(920 * 0.20)	// 
#define TEXTURE_MAX					(6)				// テクスチャの数
#define UI_CLONE_WIDTH				(325 * 0.25f)	// テクスチャサイズ
#define UI_CLONE_HEIGHT				(215 * 0.25f)	// 
#define UI_CLONE_X					(SCREEN_WIDTH * 0.3f)	// テクスチャ座標
#define UI_CLONE_Y					(SCREEN_HEIGHT)					// 


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/KeyExplain.png",
	"data/TEXTURE/t_TUTORIAL_MODE.png",
	"data/TEXTURE/STOP_TUTORIAL.png",
	"data/TEXTURE/clone001.png",
	"data/TEXTURE/clone002.png",
	"data/TEXTURE/clone003.png",
};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号


static BOOL						g_Load = FALSE;


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitInterface(void)
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
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;
	g_Pos   = { 720.0f, 440.0f, 0.0f };
	g_TexNo = 0;


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitInterface(void)
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
void UpdateInterface(void)
{


#ifdef _DEBUG	// デバッグ情報を表示する
	
#endif

}

////=============================================================================
//// 描画処理
////=============================================================================
//void DrawInterface(void)
//{
//	// 頂点バッファ設定
//	UINT stride = sizeof(VERTEX_3D);
//	UINT offset = 0;
//	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
//
//	// マトリクス設定
//	SetWorldViewProjection2D();
//
//	// プリミティブトポロジ設定
//	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//
//	// マテリアル設定
//	MATERIAL material;
//	ZeroMemory(&material, sizeof(material));
//	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//	SetMaterial(material);
//	g_TexNo = CONTROL_TEXTURE;
//	// テクスチャ設定
//	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);
//
//	g_w = TEXTURE_WIDTH;
//	g_h = TEXTURE_HEIGHT;
//	g_Pos = { 720.0f, 440.0f, 0.0f };
//
//	// UIの位置やテクスチャー座標を反映
//	float px = g_Pos.x;	// UIの表示位置X
//	float py = g_Pos.y;			// UIの表示位置Y
//	float pw = g_w;				// UIの表示幅
//	float ph = g_h;				// UIの表示高さ
//
//	float tw = 1.0f;		// テクスチャの幅
//	float th = 1.0f;		// テクスチャの高さ
//	float tx = 0.0f;			// テクスチャの左上X座標
//	float ty = 0.0f;			// テクスチャの左上Y座標
//
//	// １枚のポリゴンの頂点とテクスチャ座標を設定
//	SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
//
//	// ポリゴン描画
//	GetDeviceContext()->Draw(4, 0);
//
//	//クローンセットUIの表示
//	for (int i = 0; i < CLONE_MAX; i++)
//	{
//		g_TexNo = CLONE001 + i;
//		PLAYER *player = GetPlayer();
//		if ((g_TexNo == CLONE001 && GetMoney() < CLONE_OFFENSIVE_VALUE) ||
//			(g_TexNo == CLONE002 && GetMoney() < CLONE_TAKE_TIME_VALUE) ||
//			(g_TexNo == CLONE003 && GetMoney() < CLONE_CROW_VALUE) ||
//			(GetTime() <= 0 || player->life <= 0))
//		{
//			SwapShader(MODE_MONO);	//購入できない場合、シェーダーをモノクロに
//		}
//
//		// テクスチャ設定
//		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);
//
//		g_Pos.x = UI_CLONE_X + (UI_CLONE_WIDTH * i) + i * 8;//隙間を開けつつ描画
//		g_Pos.y = UI_CLONE_Y - UI_CLONE_HEIGHT * 0.5f;
//		g_w = UI_CLONE_WIDTH;
//		g_h = UI_CLONE_HEIGHT;
//
//		// UIの位置やテクスチャー座標を反映
//		px = g_Pos.x;	// UIの表示位置X
//		py = g_Pos.y;			// UIの表示位置Y
//		pw = g_w;				// UIの表示幅
//		ph = g_h;				// UIの表示高さ
//
//		tw = 1.0f;		// テクスチャの幅
//		th = 1.0f;		// テクスチャの高さ
//		tx = 0.0f;			// テクスチャの左上X座標
//		ty = 0.0f;			// テクスチャの左上Y座標
//
//		// １枚のポリゴンの頂点とテクスチャ座標を設定
//		SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
//			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
//
//		// ポリゴン描画
//		GetDeviceContext()->Draw(4, 0);
//
//		SwapShader(MODE_PLANE);//シェーダーを元に戻す
//	}
//
//	if (GetPlayMode() != TUTORIAL_GAME)return;	//チュートリアル中ならそれを明示するテクスチャを表示
//
//	g_TexNo = TUTORIAL_NOW;
//
//	// テクスチャ設定
//	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);
//
//	g_Pos.x = SCREEN_WIDTH * 0.25f;
//	g_Pos.y = SENTENCE_HEIGHT * 0.5f;
//	g_w = SENTENCE_WIDTH;
//	g_h = SENTENCE_HEIGHT;
//
//	// UIの位置やテクスチャー座標を反映
//	px = g_Pos.x;	// UIの表示位置X
//	py = g_Pos.y;			// UIの表示位置Y
//	pw = g_w;				// UIの表示幅
//	ph = g_h;				// UIの表示高さ
//
//	tw = 1.0f;		// テクスチャの幅
//	th = 1.0f;		// テクスチャの高さ
//	tx = 0.0f;			// テクスチャの左上X座標
//	ty = 0.0f;			// テクスチャの左上Y座標
//
//	// １枚のポリゴンの頂点とテクスチャ座標を設定
//	SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
//		XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
//
//	// ポリゴン描画
//	GetDeviceContext()->Draw(4, 0);
//
//	g_TexNo = STOP_TUTORIAL;
//
//	// テクスチャ設定
//	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);
//
//	g_Pos.x = SCREEN_WIDTH * 0.85f;
//	g_Pos.y = SENTENCE_HEIGHT;
//	g_w = SENTENCE_WIDTH * 0.5;
//	g_h = SENTENCE_HEIGHT * 2.0f;
//
//	// UIの位置やテクスチャー座標を反映
//	px = g_Pos.x;	// UIの表示位置X
//	py = g_Pos.y;			// UIの表示位置Y
//	pw = g_w;				// UIの表示幅
//	ph = g_h;				// UIの表示高さ
//
//	tw = 1.0f;		// テクスチャの幅
//	th = 1.0f;		// テクスチャの高さ
//	tx = 0.0f;			// テクスチャの左上X座標
//	ty = 0.0f;			// テクスチャの左上Y座標
//
//	// １枚のポリゴンの頂点とテクスチャ座標を設定
//	SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
//
//	// ポリゴン描画
//	GetDeviceContext()->Draw(4, 0);
//
//}
