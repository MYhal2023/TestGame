//=============================================================================
//
// �G�l�~�[���f������ [enemy.h]
// Author : GP11B132 28 BURRUEL LEANOS GONZALO
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_ENEMY		(3)					// �G�l�~�[�̐�

#define	ENEMY_SIZE		(0.6f)				// �����蔻��̑傫��

#define TUTORIAL_ENEMY	(3)
#define STAGE_1_ENEMY	(10)


#define SPAWN_POINT_X			(0.0f)					//�G�l�~�[���ǂ��Ɏn�߂邩
#define SPAWN_POINT_Y			(10.0f)					//�G�l�~�[���ǂ��Ɏn�߂邩
#define SPAWN_POINT_Z			(20.0f)					//�G�l�~�[���ǂ��Ɏn�߂邩

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct ENEMY
{
	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)

	BOOL				use;
	BOOL				load;
	BOOL				draw;				//�`�����ǂ���
	DX11_MODEL			model;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F

	float				spd;				// �ړ��X�s�[�h
	float				size;				// �����蔻��̑傫��
	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�

	int					type;				//�G�l�~�[�̈Ⴂ
	int					track;				//�G�l�~�[���ǂ��ɍU������
	int					attcnt,attcntmax;	//�G�l�~�[������U�����邩

	
	INTERPOLATION_DATA	*tbl_adr;			// �A�j���f�[�^�̃e�[�u���擪�A�h���X
	int					tbl_size;			// �o�^�����e�[�u���̃��R�[�h����
	float				move_time;			// ���s����

	XMFLOAT3			moveVec;			//�����x�N�g��
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);
XMFLOAT3 MoveEnemy(XMFLOAT3 playerpos, int i);

ENEMY *GetEnemy(void);
