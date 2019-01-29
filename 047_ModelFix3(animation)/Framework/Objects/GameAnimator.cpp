#include "Framework.h"
#include "GameAnimator.h"
#include "Model/ModelClip.h"

GameAnimator::GameAnimator(wstring shaderFile, wstring matFile, wstring meshFile)
	: GameModel(shaderFile, matFile, meshFile)
	, currentClip(0), currentKeyframe(0), nextKeyframe(0)
	, frameTime(0.0f), frameFactor(0.0f)
{
	pass = 1; //부모에 있는 pass
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
	//위에서 가져온 frameRate가 30이다
	//밑에 if문은 1/30초마다 UpdateBone업데이트 갱신 그래야 연산량이 줄어듬 대신 좀 끊기는현상 있음
	if (frameTime >= invFrameRate) 
	{
		frameTime = 0.0f;

		currentKeyframe = (currentKeyframe + 1) % clip->FrameCount();
		nextKeyframe = (currentKeyframe + 1) % clip->FrameCount();

		//움직일때마다 본이 바뀌니까 여기로 와야한다
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

//Model 클래스에 CopyGlobalBoneTo와 같은 역할
//본 갱신하는 얘
void GameAnimator::UpdateTransforms()
{
	for (UINT i = 0; i < model->BoneCount(); i++)
	{
		ModelBone* bone = model->BoneByIndex(i);

		// 해당 본에 애니메이션이 없을수도 있음
		//해당 본에 frame이 없으면 계속
		ModelKeyframe* frame = clips[currentClip]->Keyframe(bone->Name());
		if (frame == NULL) continue;

		D3DXMATRIX S, R, T;

		ModelKeyframeData current = frame->Transforms[currentKeyframe];
		ModelKeyframeData next = frame->Transforms[nextKeyframe];

		// 프레임 사이 크기보간
		D3DXVECTOR3 s1 = current.Scale;
		D3DXVECTOR3 s2 = next.Scale;
		D3DXVECTOR3 s;
		D3DXVec3Lerp(&s, &s1, &s2, frameFactor); // 선형 보간
		D3DXMatrixScaling(&S, s.x, s.y, s.z);

		// 프레임 사이 각도보간
		D3DXQUATERNION q1 = current.Rotation;
		D3DXQUATERNION q2 = current.Rotation;
		D3DXQUATERNION q;
		D3DXQuaternionSlerp(&q, &q1, &q2, frameFactor);
		D3DXMatrixRotationQuaternion(&R, &q);

		// 포지션 사이 보간
		D3DXVECTOR3 t1 = current.Translation;
		D3DXVECTOR3 t2 = next.Translation;
		D3DXVECTOR3 t;
		D3DXVec3Lerp(&t, &t1, &t2, frameFactor); // 선형 보간
		D3DXMatrixTranslation(&T, t.x, t.y, t.z);

		//현재 프레임에 대한 애니매이션 행렬구함
		//원본 본이 얼마만큼 움직일지에 대한 행렬
		D3DXMATRIX animation = S * R * T;

		//여기는 부모자식 관계 맺을려고 만듬
		D3DXMATRIX transform;
		D3DXMATRIX parentTransform;

		int parentIndex = bone->ParentIndex();
		if (parentIndex < 0) // 뿌리노드
		{
			D3DXMatrixIdentity(&parentTransform); //parentIndex가 0보다 작으면 부모 그래서 identity잡음

			//이동할때 위치 크기 각도 변화시키기 위해
			//World(&parentTransform); 
		}
		else // 그 외
			//자기의 부모의 본(boneTransform뒤에 부모인덱스가 들어가서)
			parentTransform = boneTransforms[parentIndex];

		//자기본의 글로벌
		D3DXMATRIX inv = bone->Global();
		//글로벌 뒤집으면 로컬?
		D3DXMatrixInverse(&inv, NULL, &inv);

		//자신 본 위치
		//자기가 움직일거랑 부모 곱(animation * boneTransforms[parnetIndex]
		boneTransforms[i] = animation * parentTransform;


		//원본 본의 로컬 * animation 하고 여기에 부모를 곱하면 내가 실제로간 위치
		//즉 원본본의 로컬을 기준을 잡고 내가 얼마만큼 갈지의 animation을 곱
		//그리고 원본의 위치는 부모로 부터 결정된다 그래서 마지막에 곱하기 부모를 곱한다.
		renderTransforms[i] = inv * boneTransforms[i];

		/*
		원래는 왼손좌표계에서는 부모의 행렬에다 자기의 global의 역행렬(로컬)곱
		근데 fbx에서는 오른손좌표계여서 부모가 뒤에 곱해진다(왼손좌표계의 반대)
		*/
	}
}

/*
밑에 바닥깔고 캐릭터가 가만히 서있다가 마우스 누른 방향으로 걸어가고 다시 idle로 변환
이동할때 월드를 뿌리노드(parentTransform)에 곱
*/
