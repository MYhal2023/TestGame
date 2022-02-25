//=============================================================================
//
// メッシュ地面の処理 [meshfield.h]
// Author : 
//
//=============================================================================
#pragma once

enum
{
	WATER,
	GROUND,
};
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitMeshField(XMFLOAT3 pos, XMFLOAT3 rot,
							int nNumBlockX, int nNumBlockZ, float nBlockSizeX, float nBlockSizeZ, int fType);
void UninitMeshField(void);
void UpdateMeshField(void);
void DrawMeshField(void);

BOOL RayHitField(XMFLOAT3 pos, XMFLOAT3 *HitPosition, XMFLOAT3 *Normal);

