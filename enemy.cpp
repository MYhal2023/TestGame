//=============================================================================
//
// �G�l�~�[���f������ [enemy.cpp]
// Author : GP11B132 28 BURRUEL LEANOS GONZALO
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "renderer.h"
#include "camera.h"
#include "debugproc.h"
#include "input.h"
#include "model.h"
#include "enemy.h"
#include "enemybullet.h"
#include "player.h"
#include "shadow.h"
#include "particle.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_ENEMY			"data/MODEL/Eden.obj"		// �ǂݍ��ރ��f����
//#define	MODEL_ENEMY_PARTS	"data/MODEL/ago2.obj"			// �ǂݍ��ރ��f����

#define ENEMY_SHADOW_SIZE	(0.4f)						// �e�̑傫��
#define ENEMY_OFFSET_Y		(7.0f)						// �G�l�~�[�̑��������킹��

#define ENEMY_ATTACK_MOVE	(20.0f)						//�G�l�~�[�̍U���̑���

#define ENEMY_SPEED			(0.01f)						//�G�l�~�[�̑���


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];				// �G�l�~�[
static BOOL				g_Load = FALSE;



static INTERPOLATION_DATA move_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(SPAWN_POINT_X,SPAWN_POINT_Y,  SPAWN_POINT_Z), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE), 10*2 },
	{ XMFLOAT3(	  0.0f, 0.0f,  20.0f), XMFLOAT3(0.3f, XM_PI * 2, 0.0f), XMFLOAT3(ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE), 100*1 },
	{ XMFLOAT3(   0.0f, 0.0f,  20.0f), XMFLOAT3(0.3f, XM_PI * 2, 0.0f), XMFLOAT3(ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE), 50*1 },
	{ XMFLOAT3(SPAWN_POINT_X,SPAWN_POINT_Y,  SPAWN_POINT_Z), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE), 10*2 },

};

//static char *g_modelName[ENEMY_PARTS_MAX] = {
//	MODEL_ENEMY_PARTS,
//};

//����
int					g_time;


//=============================================================================
// ����������
//=============================================================================
HRESULT InitEnemy(void)
{
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		LoadModel(MODEL_ENEMY, &g_Enemy[i].model);
		g_Enemy[i].load = TRUE;

		g_Enemy[i].pos = XMFLOAT3(SPAWN_POINT_X + i * 120.0f - 120.0f, SPAWN_POINT_Y, SPAWN_POINT_Z);
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		g_Enemy[i].spd = 0.0f;			// �ړ��X�s�[�h�N���A
		g_Enemy[i].size = ENEMY_SIZE;	// �����蔻��̑傫��

		// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
		GetModelDiffuse(&g_Enemy[0].model, &g_Enemy[0].diffuse[0]);

		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		g_Enemy[i].shadowIdx = CreateShadow(pos, ENEMY_SHADOW_SIZE, ENEMY_SHADOW_SIZE);

		g_Enemy[i].move_time = 0.0f;	// ���`��ԗp�̃^�C�}�[���N���A
		g_Enemy[i].tbl_adr = move_tbl;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Enemy[i].tbl_size = sizeof(move_tbl) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Enemy[i].draw = TRUE;
		g_Enemy[i].use = TRUE;			// TRUE:�����Ă�
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitEnemy(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].load)
		{
			UnloadModel(&g_Enemy[i].model);
			g_Enemy[i].load = FALSE;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateEnemy(void)
{
	g_time += 1;
	// �G�l�~�[�𓮂����ꍇ�́A�e�����킹�ē���������Y��Ȃ��悤�ɂˁI
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use != TRUE)	
			continue;					// ���̃G�l�~�[���g���Ă���H
										//�͂�
		BulletAttack(i);				//�e���o��

		PLAYER p = *GetPlayer();		//�v���C���[�������Ă���H
		if (p.spd <= 0.1f)				
			continue;

		MoveEnemy(p.pos, i);			//�͂��B�G�l�~�[������

		//�e
		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		SetPositionShadow(g_Enemy[i].shadowIdx,pos);


#ifdef _DEBUG	// �f�o�b�O����\������
		PrintDebugProc("Enemy:X:%f Y:%f Z:%f\n", g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
#endif

	}

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == FALSE || g_Enemy[i].draw == FALSE) continue;

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);

		// ���f���`��
		DrawModel(&g_Enemy[i].model);

	}

	// �p�[�c�̊K�w�A�j���[�V����




	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// �G�l�~�[�̃o���b�g�U��
//�Ƃ肠����100�t���[���o������e���o��
//=============================================================================
void BulletAttack(int i)
{
	if (g_time % 100 == 0)
	{
		g_time = 0;										//�傫�Ȑ����ɂȂ�Ȃ�����
		SetEnemyBullet(g_Enemy[i].pos, g_Enemy[i].rot);
	}
}

//=============================================================================
// �G�l�~�[�̓���
//=============================================================================
void MoveEnemy(XMFLOAT3 playerpos, int i)
{

	float dirX = g_Enemy[i].pos.x - playerpos.x;
	float dirZ = g_Enemy[i].pos.z - playerpos.z;
	
	g_Enemy[i].moveVec = { dirX, 0.0f, dirZ };

	float angle = atan2f(g_Enemy[i].moveVec.x, g_Enemy[i].moveVec.z);
	g_Enemy[i].rot.y = angle;

	XMVECTOR	mVec = XMLoadFloat3(&g_Enemy[i].moveVec);
	XMVector3Normalize(mVec);

	XMVECTOR now = XMLoadFloat3(&g_Enemy[i].pos);								// ���݂̏ꏊ
	XMStoreFloat3(&g_Enemy[i].pos, now - mVec * ENEMY_SPEED);	//�P�ʃx�N�g�������Ɉړ�

}




//=============================================================================
// �G�l�~�[�̎擾
//=============================================================================
ENEMY *GetEnemy()
{
	return &g_Enemy[0];
}