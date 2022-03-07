//=============================================================================
//
// �e���ˏ��� [bullet.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MAX_BULLET		(256)	// �e�ő吔

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
	float		size;			// �����蔻��̑傫��
	int			shadowIdx;		// �eID
	bool		use;			// �g�p���Ă��邩�ǂ���
	int			time;			// ���˂��Ă���ǂꂾ�����Ԃ��o�߂�����

} BULLET;


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitPlayerBullet(void);
void UninitPlayerBullet(void);
void UpdatePlayerBullet(void);
void DrawPlayerBullet(void);

int SetPlayerBullet(XMFLOAT3 pos, XMFLOAT3 rot);

BULLET *GetPlayerBullet(void);

