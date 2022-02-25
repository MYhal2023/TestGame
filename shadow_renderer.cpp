#include "main.h"
#include "renderer.h"
#include "shadow_renderer.h"

#define SHADOWMAP_SIZE		(4000)

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11DepthStencilView*		g_DepthStencilView = NULL;
static ID3D11ShaderResourceView*	g_DepthResource = NULL;
static ID3D11SamplerState*			g_SamplerState = NULL;

static ID3D11PixelShader*		g_PixelShader = NULL;

static D3D11_VIEWPORT			g_ViewPort;

static ID3D11DepthStencilState* g_DSState = NULL;

static ID3D11VertexShader*		g_VertexShader = NULL;
static ID3D11InputLayout*		g_VertexLayout = NULL;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitSM(void)
{
	HRESULT hr = S_OK;
	ID3D11Device* pDevice = GetDevice();

	//深度テクスチャー作成
	ID3D11Texture2D* depthTexture = NULL;
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
	td.Format = DXGI_FORMAT_R24G8_TYPELESS;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.SampleDesc.Count = 1;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	td.Height = SHADOWMAP_SIZE;
	td.Width = SHADOWMAP_SIZE;
	pDevice->CreateTexture2D(&td, NULL, &depthTexture);


	//深度ステンシルビュー作成
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;
	pDevice->CreateDepthStencilView(depthTexture, &dsvd, &g_DepthStencilView);

	//シェーダーリソース作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvd.Texture2D.MipLevels = 1;
	pDevice->CreateShaderResourceView(depthTexture, &srvd, &g_DepthResource);

	// ビューポート設定
	ZeroMemory(&g_ViewPort, sizeof(D3D11_VIEWPORT));
	g_ViewPort.Width = SHADOWMAP_SIZE;
	g_ViewPort.Height = SHADOWMAP_SIZE;
	g_ViewPort.MinDepth = 0.0f;
	g_ViewPort.MaxDepth = 1.0f;
	g_ViewPort.TopLeftX = 0;
	g_ViewPort.TopLeftY = 0;

	// 深度ステンシルステート作成
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	// ステンシルテスト無効化
	depthStencilDesc.StencilEnable = FALSE;
	pDevice->CreateDepthStencilState(&depthStencilDesc, &g_DSState);

	// サンプラーステート設定
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = 0.f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MipLODBias = 0.f;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

	pDevice->CreateSamplerState(&samplerDesc, &g_SamplerState);

	// 頂点シェーダコンパイル・生成
	ID3DBlob* pErrorBlob;
	ID3DBlob* pVSBlob = NULL;
	DWORD shFlag = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(_DEBUG) && defined(DEBUG_SHADER)
	shFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	hr = D3DX11CompileFromFile("shadow.hlsl", NULL, NULL, "VertexShaderPolygon", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShader);

	// 入力レイアウト生成
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	pDevice->CreateInputLayout(layout,
		numElements,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&g_VertexLayout);

	pVSBlob->Release();


	// ピクセルシェーダコンパイル・生成
	ID3DBlob* pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shadow.hlsl", NULL, NULL, "PixelShaderPolygon", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShader);

	pPSBlob->Release();

	return hr;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitSM(void)
{
	// オブジェクト解放
	if (g_DSState)				g_DSState->Release();

	if (g_VertexLayout)			g_VertexLayout->Release();
	if (g_VertexShader)			g_VertexShader->Release();

	if (g_DepthStencilView)		g_DepthStencilView->Release();
	if (g_DepthResource)		g_DepthResource->Release();
	if (g_SamplerState)			g_SamplerState->Release();
}

void SetSMRenderer(void)
{
	ID3D11DeviceContext* pImmediateContext = GetDeviceContext();

	pImmediateContext->OMSetRenderTargets(0,NULL, g_DepthStencilView);
	pImmediateContext->OMSetDepthStencilState(g_DSState, NULL);

	pImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
	pImmediateContext->PSSetShader(g_PixelShader, NULL, 0);


	pImmediateContext->RSSetViewports(1, &g_ViewPort);
	pImmediateContext->IASetInputLayout(g_VertexLayout);

	pImmediateContext->VSSetConstantBuffers(0, 1, GetWorld());
	pImmediateContext->VSSetConstantBuffers(1, 1, GetLightView());
	pImmediateContext->VSSetConstantBuffers(2, 1, GetLightProjection());

	// カリングモード設定
	SetCullingMode(CULL_MODE_BACK);
	// アルファブレンド設定
	SetBlendState(BLEND_MODE_ALPHABLEND);

	pImmediateContext->ClearDepthStencilView(g_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11ShaderResourceView** SMResource(void)
{
	return &g_DepthResource;
}

ID3D11SamplerState** SMSampler(void)
{
	return &g_SamplerState;
}