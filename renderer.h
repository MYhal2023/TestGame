//=============================================================================
//
// �����_�����O���� [renderer.h]
// Author : 
//
//=============================================================================
#pragma once


//*********************************************************
// �}�N����`
//*********************************************************
#define LIGHT_MAX		(5)
#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// �r���[���ʂ̎���p
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// �r���[���ʂ̃A�X�y�N�g��	
#define	VIEW_NEAR_Z		(10.0f)											// �r���[���ʂ�NearZ�l
#define	VIEW_FAR_Z		(10000.0f)										// �r���[���ʂ�FarZ�l

enum LIGHT_TYPE
{
	LIGHT_TYPE_NONE,		//���C�g����
	LIGHT_TYPE_DIRECTIONAL,	//�f�B���N�V���i�����C�g
	LIGHT_TYPE_POINT,		//�|�C���g���C�g

	LIGHT_TYPE_NUM
};

enum BLEND_MODE
{
	BLEND_MODE_NONE,		//�u�����h����
	BLEND_MODE_ALPHABLEND,	//���u�����h
	BLEND_MODE_ADD,			//���Z�u�����h
	BLEND_MODE_SUBTRACT,	//���Z�u�����h

	BLEDD_MODE_NUM
};

enum CULL_MODE
{
	CULL_MODE_NONE,			//�J�����O����
	CULL_MODE_FRONT,		//�\�̃|���S����`�悵�Ȃ�(CW)
	CULL_MODE_BACK,			//���̃|���S����`�悵�Ȃ�(CCW)

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
// �\����
//*********************************************************

// ���_�\����
struct VERTEX_3D
{
    XMFLOAT3	Position;
    XMFLOAT3	Normal;
    XMFLOAT4	Diffuse;
    XMFLOAT2	TexCoord;
};

// �}�e���A���\����
struct MATERIAL
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	int			noTexSampling;
};

// ���C�g�\����
struct LIGHT {
	XMFLOAT4X4			mtxView;		// �r���[�}�g���b�N�X
	XMFLOAT4X4			mtxInvView;		// �r���[�C���o�[�X�}�g���b�N�X
	XMFLOAT4X4			mtxProjection;	// �v���W�F�N�V�����}�g���b�N�X
	XMFLOAT3	Direction;	// ���C�g�̕���
	XMFLOAT3	at;			// �J�����̒����_
	XMFLOAT3	Position;	// ���C�g�̈ʒu
	XMFLOAT4	Diffuse;	// �g�U���̐F
	XMFLOAT4	Ambient;	// �����̐F
	float		Attenuation;// ������
	int			Type;		// ���C�g��ʁE�L���t���O
	int			Enable;		// ���C�g��ʁE�L���t���O
};

struct CAMERA
{
	XMFLOAT4X4			mtxView;		// �r���[�}�g���b�N�X
	XMFLOAT4X4			mtxInvView;		// �r���[�C���o�[�X�}�g���b�N�X
	XMFLOAT4X4			mtxProjection;	// �v���W�F�N�V�����}�g���b�N�X

	XMFLOAT3			pos;			// �J�����̎��_(�ʒu)
	XMFLOAT3			at;				// �J�����̒����_
	XMFLOAT3			atPos;			//�J�����̒����_�̕ύX��
	XMFLOAT3			up;				// �J�����̏�����x�N�g��
	XMFLOAT3			rot;			// �J�����̉�]

	float				len;			// �J�����̎��_�ƒ����_�̋���

	INTERPOLATION_DATA	*tbl_adr;			// �A�j���f�[�^�̃e�[�u���擪�A�h���X
	int					tbl_size;			// �o�^�����e�[�u���̃��R�[�h����
	float				move_time;			// ���s����

};

// �t�H�O�\����
struct FOG {
	float		FogStart;	// �t�H�O�̊J�n����
	float		FogEnd;		// �t�H�O�̍ő勗��
	XMFLOAT4	FogColor;	// �t�H�O�̐F
};


//*****************************************************************************
// �v���g�^�C�v�錾
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