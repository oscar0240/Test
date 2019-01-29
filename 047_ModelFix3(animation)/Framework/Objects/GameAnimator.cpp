#include "Framework.h"
#include "GameAnimator.h"
#include "Model/ModelClip.h"

GameAnimator::GameAnimator(wstring shaderFile, wstring matFile, wstring meshFile)
	: GameModel(shaderFile, matFile, meshFile)
	, currentClip(0), currentKeyframe(0), nextKeyframe(0)
	, frameTime(0.0f), frameFactor(0.0f)
{
	pass = 1; //�θ� �ִ� pass
}

GameAnimator::~GameAnimator()
{
	for (ModelClip* clip : clips)
		SAFE_DELETE(clip);
}

void GameAnimator::AddClip(wstring clipFile)
{
	clips.push_back(new ModelClip(clipFile));
}

void GameAnimator::Update()
{
	__super::Update();

	frameTime += Time::Delta();

	ModelClip* clip = clips[currentClip];
	float invFrameRate = 1.0f / clip->FrameRate();
	//������ ������ frameRate�� 30�̴�
	//�ؿ� if���� 1/30�ʸ��� UpdateBone������Ʈ ���� �׷��� ���귮�� �پ�� ��� �� ��������� ����
	if (frameTime >= invFrameRate) 
	{
		frameTime = 0.0f;

		currentKeyframe = (currentKeyframe + 1) % clip->FrameCount();
		nextKeyframe = (currentKeyframe + 1) % clip->FrameCount();

		//�����϶����� ���� �ٲ�ϱ� ����� �;��Ѵ�
		UpdateTransforms();
		MappedBoneBuffer();

		UpdateVertex();
	}
}

void GameAnimator::Render()
{
	for (ModelMesh* mesh : model->Meshes())
		mesh->Render();
}

void GameAnimator::UpdateWorld()
{
	GameRender::UpdateWorld();

	D3DXMATRIX world;
	World(&world);

	for (Material* material : model->Materials())
		material->GetShader()->AsMatrix("World")->SetMatrix(world);
}

//Model Ŭ������ CopyGlobalBoneTo�� ���� ����
//�� �����ϴ� ��
void GameAnimator::UpdateTransforms()
{
	for (UINT i = 0; i < model->BoneCount(); i++)
	{
		ModelBone* bone = model->BoneByIndex(i);

		// �ش� ���� �ִϸ��̼��� �������� ����
		//�ش� ���� frame�� ������ ���
		ModelKeyframe* frame = clips[currentClip]->Keyframe(bone->Name());
		if (frame == NULL) continue;

		D3DXMATRIX S, R, T;

		ModelKeyframeData current = frame->Transforms[currentKeyframe];
		ModelKeyframeData next = frame->Transforms[nextKeyframe];

		// ������ ���� ũ�⺸��
		D3DXVECTOR3 s1 = current.Scale;
		D3DXVECTOR3 s2 = next.Scale;
		D3DXVECTOR3 s;
		D3DXVec3Lerp(&s, &s1, &s2, frameFactor); // ���� ����
		D3DXMatrixScaling(&S, s.x, s.y, s.z);

		// ������ ���� ��������
		D3DXQUATERNION q1 = current.Rotation;
		D3DXQUATERNION q2 = current.Rotation;
		D3DXQUATERNION q;
		D3DXQuaternionSlerp(&q, &q1, &q2, frameFactor);
		D3DXMatrixRotationQuaternion(&R, &q);

		// ������ ���� ����
		D3DXVECTOR3 t1 = current.Translation;
		D3DXVECTOR3 t2 = next.Translation;
		D3DXVECTOR3 t;
		D3DXVec3Lerp(&t, &t1, &t2, frameFactor); // ���� ����
		D3DXMatrixTranslation(&T, t.x, t.y, t.z);

		//���� �����ӿ� ���� �ִϸ��̼� ��ı���
		//���� ���� �󸶸�ŭ ���������� ���� ���
		D3DXMATRIX animation = S * R * T;

		//����� �θ��ڽ� ���� �������� ����
		D3DXMATRIX transform;
		D3DXMATRIX parentTransform;

		int parentIndex = bone->ParentIndex();
		if (parentIndex < 0) // �Ѹ����
		{
			D3DXMatrixIdentity(&parentTransform); //parentIndex�� 0���� ������ �θ� �׷��� identity����

			//�̵��Ҷ� ��ġ ũ�� ���� ��ȭ��Ű�� ����
			//World(&parentTransform); 
		}
		else // �� ��
			//�ڱ��� �θ��� ��(boneTransform�ڿ� �θ��ε����� ����)
			parentTransform = boneTransforms[parentIndex];

		//�ڱ⺻�� �۷ι�
		D3DXMATRIX inv = bone->Global();
		//�۷ι� �������� ����?
		D3DXMatrixInverse(&inv, NULL, &inv);

		//�ڽ� �� ��ġ
		//�ڱⰡ �����ϰŶ� �θ� ��(animation * boneTransforms[parnetIndex]
		boneTransforms[i] = animation * parentTransform;


		//���� ���� ���� * animation �ϰ� ���⿡ �θ� ���ϸ� ���� �����ΰ� ��ġ
		//�� �������� ������ ������ ��� ���� �󸶸�ŭ ������ animation�� ��
		//�׸��� ������ ��ġ�� �θ�� ���� �����ȴ� �׷��� �������� ���ϱ� �θ� ���Ѵ�.
		renderTransforms[i] = inv * boneTransforms[i];

		/*
		������ �޼���ǥ�迡���� �θ��� ��Ŀ��� �ڱ��� global�� �����(����)��
		�ٵ� fbx������ ��������ǥ�迩�� �θ� �ڿ� ��������(�޼���ǥ���� �ݴ�)
		*/
	}
}

/*
�ؿ� �ٴڱ�� ĳ���Ͱ� ������ ���ִٰ� ���콺 ���� �������� �ɾ�� �ٽ� idle�� ��ȯ
�̵��Ҷ� ���带 �Ѹ����(parentTransform)�� ��
*/
