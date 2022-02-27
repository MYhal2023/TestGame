//=============================================================================
//
// �e���ˏ��� [bullet.h]
// Author : GP11B132 28 BURRUEL LEANOS GONZALO
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MAX_BULLET		(256)	// �e�ő吔

#define	BULLET_WH		(5.0f)	// �����蔻��̑傫��

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
typedef struct
{
	XMFLOAT4X4	mtxWorld;		// ���[���h�}�g���b�N�X
	XMFLOAT3	pos;			// �ʒu
	XMFLOAT3	rot;			// �p�x
	XMFLOAT3	scl;			// �X�P�[��
	MATERIAL	material;		// �}�e���A��
	float		spd;			// �ړ���
	float		fWidth;			// ��
	float		fHeight;		// ����
	int			shadowIdx;		// �eID
	int			frames;			//�t���[��
	BOOL		use;			// �g�p���Ă��邩�ǂ���


} E_BULLET;


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitEnemyBullet(void);
void UninitEnemyBullet(void);
void UpdateEnemyBullet(void);
void DrawEnemyBullet(void);

int SetEnemyBullet(XMFLOAT3 pos, XMFLOAT3 rot);

E_BULLET *GetEnemyBullet(void);

