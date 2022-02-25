//=============================================================================
//
// レンダリング処理 [renderer.h]
// Author : 
//
//=============================================================================
#pragma once


//*********************************************************
// マクロ定義
//*********************************************************
#define LIGHT_MAX		(5)
#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// ビュー平面の視野角
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// ビュー平面のアスペクト比	
#define	VIEW_NEAR_Z		(10.0f)											// ビュー平面のNearZ値
#define	VIEW_FAR_Z		(10000.0f)										// ビュー平面のFarZ値

enum LIGHT_TYPE
{
	LIGHT_TYPE_NONE,		//ライト無し
	LIGHT_TYPE_DIRECTIONAL,	//ディレクショナルライト
	LIGHT_TYPE_POINT,		//ポイントライト

	LIGHT_TYPE_NUM
};

enum BLEND_MODE
{
	BLEND_MODE_NONE,		//ブレンド無し
	BLEND_MODE_ALPHABLEND,	//αブレンド
	BLEND_MODE_ADD,			//加算ブレンド
	BLEND_MODE_SUBTRACT,	//減算ブレンド

	BLEDD_MODE_NUM
};

enum CULL_MODE
{
	CULL_MODE_NONE,			//カリング無し
	CULL_MODE_FRONT,		//表のポリゴンを描画しない(CW)
	CULL_MODE_BACK,			//裏のポリゴンを描画しない(CCW)

	CULL_MODE_NUM
};

enum SHADER_MODE
{
	MODE_PLANE,
	MODE_MONO,
	MODE_SPECULAR,
	MODE_BLUR,
	MODE_SHADER_NUM
};
//*********************************************************
// 構造体
//*********************************************************

// 頂点構造体
struct VERTEX_3D
{
    XMFLOAT3	Position;
    XMFLOAT3	Normal;
    XMFLOAT4	Diffuse;
    XMFLOAT2	TexCoord;
};

// マテリアル構造体
struct MATERIAL
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	int			noTexSampling;
};

// ライト構造体
struct LIGHT {
	XMFLOAT4X4			mtxView;		// ビューマトリックス
	XMFLOAT4X4			mtxInvView;		// ビューインバースマトリックス
	XMFLOAT4X4			mtxProjection;	// プロジェクションマトリックス
	XMFLOAT3	Direction;	// ライトの方向
	XMFLOAT3	at;			// カメラの注視点
	XMFLOAT3	Position;	// ライトの位置
	XMFLOAT4	Diffuse;	// 拡散光の色
	XMFLOAT4	Ambient;	// 環境光の色
	float		Attenuation;// 減衰率
	int			Type;		// ライト種別・有効フラグ
	int			Enable;		// ライト種別・有効フラグ
};

struct CAMERA
{
	XMFLOAT4X4			mtxView;		// ビューマトリックス
	XMFLOAT4X4			mtxInvView;		// ビューインバースマトリックス
	XMFLOAT4X4			mtxProjection;	// プロジェクションマトリックス

	XMFLOAT3			pos;			// カメラの視点(位置)
	XMFLOAT3			at;				// カメラの注視点
	XMFLOAT3			atPos;			//カメラの注視点の変更量
	XMFLOAT3			up;				// カメラの上方向ベクトル
	XMFLOAT3			rot;			// カメラの回転

	float				len;			// カメラの視点と注視点の距離

	INTERPOLATION_DATA	*tbl_adr;			// アニメデータのテーブル先頭アドレス
	int					tbl_size;			// 登録したテーブルのレコード総数
	float				move_time;			// 実行時間

};

// フォグ構造体
struct FOG {
	float		FogStart;	// フォグの開始距離
	float		FogEnd;		// フォグの最大距離
	XMFLOAT4	FogColor;	// フォグの色
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitRenderer(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void UninitRenderer(void);

void SetRenderer(void);
void Clear(void);
void Present(void);

ID3D11Device *GetDevice( void );
ID3D11DeviceContext *GetDeviceContext( void );
ID3D11Buffer **GetWorld(void);
ID3D11Buffer **GetLightView(void);
ID3D11Buffer **GetLightProjection(void);
ID3D11Buffer **GetView(void);
ID3D11Buffer **GetProjection(void);

void SetDepthEnable( BOOL Enable );
void SetBlendState(BLEND_MODE bm);
void SetCullingMode(CULL_MODE cm);
void SetAlphaTestEnable(BOOL flag);

void SetWorldViewProjection2D( void );
void SetWorldMatrix( XMMATRIX *WorldMatrix );
void SetViewMatrix( XMMATRIX *ViewMatrix );
void SetProjectionMatrix(XMMATRIX *ProjectionMatrix);
void SetLightViewMatrix(XMMATRIX *LightViewMatrix);
void SetLightProjectionMatrix(XMMATRIX *LightProjectionMatrix);
void SetLightViewProjection(void);

void SetMaterial( MATERIAL material );

void SetLightEnable(BOOL flag);
void SetLight(int index, LIGHT* light);

void SetFogEnable(BOOL flag);
void SetFog(FOG* fog);

void DebugTextOut(char* text, int x, int y);

void SetFuchi(int flag);
void SetShaderCamera(XMFLOAT3 pos);
void SwapShader(int flag);
LIGHT *GetLight(void);