//=============================================================================
//
// ライフ処理 [life.cpp]
// Author : GP11B132 34 米倉 睦起
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "life.h"
#include "sprite.h"
#include "player.h"
#include "score.h"
#include "game.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(LIFE_DW)	// ハートのサイズ
#define TEXTURE_HEIGHT				(LIFE_DH)	// 
#define TEXTURE_MAX					(4)			// テクスチャの数
#define ALPHA_COUNT					(0.025f)		// α値の減衰量
#define MINUS_COUNT					(0.01f)	// GB値の減衰量

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[] = {
	"data/TEXTURE/hart.png",
	"data/TEXTURE/hart_lost.png",
	"data/TEXTURE/BoS_life.png",
	"data/TEXTURE/g_GAME_OVER.png",
};

static BOOL	g_Load = FALSE;		// 初期化を行ったかのフラグ
static LIFE	g_Life;
float lifeAlpha;
float lifeGB;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitLife(void)
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
	g_Life.w = TEXTURE_WIDTH;
	g_Life.h = TEXTURE_HEIGHT;
	g_Life.pos = XMFLOAT3(LIFE_DX, LIFE_DY, 0.0f);
	g_Life.texNo = 0;
	g_Life.life = 4;	// ライフを初期化

	lifeAlpha = 0.0f;
	lifeGB = 0.0f;
	g_Load = TRUE;		// データの初期化を行った
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitLife(void)
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
void UpdateLife(void)
{

}

//=============================================================================
// 描画処理 プレイヤーのライフは常に表示させる
//=============================================================================
void DrawLife(void)
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

	PLAYER *player = GetPlayer();
	//何個描画するか計算する
	g_Life.life = player->life;
	g_Life.life_max = player->lifeMax;
	g_Life.pos = XMFLOAT3(LIFE_DX, LIFE_DY, 0.0f);
	g_Life.w = LIFE_DW;
	g_Life.h = LIFE_DH;
	for (int i = 0; i < g_Life.life_max; i++)
	{
		//今あるライフ分は赤のハートを、現在ライフ値とライフ最大値の差分には黒のハートを描画する
		if (g_Life.life > i)
		{
			g_Life.texNo = HART;
		}
		else
		{
			g_Life.texNo = HART_LOST;
		}

		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Life.texNo]);

		//ハートの左右半分どちらを描画するか計算
		int number = i % 2;

		// ライフの位置やテクスチャー座標を反映
		float px = g_Life.pos.x + ((g_Life.w / 2)*i);	// ライフの表示位置X
		float py = g_Life.pos.y;				// ライフの表示位置Y
		float pw = g_Life.w / 2;					// ライフの表示幅
		float ph = g_Life.h;					// ライフの表示高さ

		float tw = 1.0f / LIFE_DIGIT;					// テクスチャの幅
		float th = 1.0f;					// テクスチャの高さ
		float tx = tw * (float)(number);						// テクスチャの左上X座標
		float ty = 0.0f;						// テクスチャの左上Y座標

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	g_Life.texNo = HART_BOS;

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Life.texNo]);
	g_Life.w = LIFE_BW;
	g_Life.h = LIFE_BH;
	g_Life.pos = XMFLOAT3(LIFE_DX - (LIFE_BW * 0.7f), LIFE_DY + (LIFE_BH * 0.2f), 0.0f);	//多少の微調整の元、座標を決定
	// ライフの位置やテクスチャー座標を反映
	float px = g_Life.pos.x;				// ライフの表示位置X
	float py = g_Life.pos.y;				// ライフの表示位置Y
	float pw = g_Life.w;					// ライフの表示幅
	float ph = g_Life.h;					// ライフの表示高さ

	float tw = 1.0f;						// テクスチャの幅
	float th = 1.0f;						// テクスチャの高さ
	float tx = 0.0f;						// テクスチャの左上X座標
	float ty = 0.0f;						// テクスチャの左上Y座標

	// １枚のポリゴンの頂点とテクスチャ座標を設定
	SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);

}
//=============================================================================
// ゲームオーバー描画処理
//=============================================================================
void DrawGameOver(void)
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
	g_Life.texNo = GAMEOVER;
	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Life.texNo]);

	// ゲームオーバー文字の位置やテクスチャー座標を反映
	float px = SCREEN_WIDTH * 0.5f;		// ゲームオーバー文字の表示位置X
	float py = SCREEN_HEIGHT * 0.5f;	// ゲームオーバー文字の表示位置Y
	float pw = GAMEOVER_DW;				// ゲームオーバー文字の表示幅
	float ph = GAMEOVER_DH;				// ゲームオーバー文字の表示高さ

	float tw = 1.0f;					// テクスチャの幅
	float th = 1.0f;					// テクスチャの高さ
	float tx = 0.0f;					// テクスチャの左上X座標
	float ty = 0.0f;					// テクスチャの左上Y座標
	//フェードイン用処理
	if (lifeAlpha < 1.0f)				
		lifeAlpha += ALPHA_COUNT;
	float alpha = lifeAlpha;
	//フェードイン後にRGN値を変更していく
	if (lifeAlpha >= 1.0f && lifeGB < 1.0f)	
		lifeGB += ALPHA_COUNT;
	float minus = lifeGB;
	// １枚のポリゴンの頂点とテクスチャ座標を設定
	SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
		XMFLOAT4(1.0f, 1.0f - minus, 1.0f - minus, alpha));

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);

}

//=============================================================================
// ライフを計算する
// 引数:add :ライフの変化量	chara: ライフを増減させる対象を判別 i:構造体配列の何番目？
// 戻り値:trueなら生存、falseなら死亡状態としてplayer.cppに返す
//=============================================================================
void AddLife(int add, int chara, int i)//加算量、対象(0ならプレイヤー、1ならエネミー)、配列番号
{
	PLAYER *player = GetPlayer();
	if (GetPlayMode() == TUTORIAL_GAME)return;	//チュートリアルは加算無し

	switch (chara)			//0ならプレイヤーのライフを増減させる。g_Lifeにプレイヤーのライフ情報を格納
	{
	case 0:
		g_Life.life = player->life;
		g_Life.life_max = player->lifeMax;
		g_Life.life += add;
		if (g_Life.life > g_Life.life_max)
		{
			g_Life.life = g_Life.life_max;
			AddScore(1000);					//最大HPを超える回復をしているならスコアを加算させる
		}
		player->life = g_Life.life;
		break;

	//case 1:	//1ならエネミーのライフを増減させる。g_Lifeにエネミーのライフ情報を格納

	//	g_Life.life = enemy[i].life;
	//	g_Life.life_max = enemy[i].lifeMax;
	//	g_Life.life += add;
	//	if (g_Life.life > g_Life.life_max)
	//	{
	//		g_Life.life = g_Life.life_max;
	//	}
	//	enemy[i].life = g_Life.life;
	//	break;

	//case 2:	//2ならクローンのライフを増減させる

	//	g_Life.life = clone[i].life;
	//	g_Life.life_max = clone[i].lifeMax;
	//	g_Life.life += add;
	//	if (g_Life.life > g_Life.life_max)
	//	{
	//		g_Life.life = g_Life.life_max;
	//	}
	//	clone[i].life = g_Life.life;
	//	break;

	}
}


int GetLife(void)
{
	return g_Life.life;
}


void SetLife(int life)
{
	g_Life.life = life;
}



