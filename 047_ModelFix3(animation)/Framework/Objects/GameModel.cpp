#include "Framework.h"
#include "GameModel.h"

GameModel::GameModel(wstring shaderFile, wstring matFile, wstring meshFile)
	: shaderFile(shaderFile), matFile(matFile), meshFile(meshFile), pass(0)
{
	shader = new Shader(Shaders + L"046_CsModel.fx", true);
}

GameModel::~GameModel()
{
	//new로 새로 할당되지 않는것들
	SAFE_RELEASE(boneBuffer);
	SAFE_RELEASE(boneSrv);

	//new로 새로 할당하고 배열로 만들어서 SAFE_DELETE_ARRAY
	SAFE_DELETE_ARRAY(boneTransforms);
	SAFE_DELETE_ARRAY(renderTransforms);

	//new로 새로 할당된것들은 SAFE_DELETE
	SAFE_DELETE(model);
	SAFE_DELETE(shader);
}

void GameModel::Ready()
{
	model = new Model();
	model->ReadMaterial(matFile);
	model->ReadMesh(meshFile);
	SetShader(shaderFile);

	boneTransforms = new D3DXMATRIX[model->BoneCount()];
	renderTransforms = new D3DXMATRIX[model->BoneCount()];

	CsResource::CreateStructuredBuffer(sizeof(D3DXMATRIX), model->BoneCount(), NULL, &boneBuffer);
	CsResource::CreateSrv(boneBuffer, &boneSrv);

	UpdateWorld();
}

void GameModel::UpdateWorld()
{
	__super::UpdateWorld(); //자식 클래스에서 부모클래스를 가르킬때

	D3DXMATRIX world;
	World(&world);

	for (Material* material : model->Materials())
		material->GetShader()->AsMatrix("World")->SetMatrix(world);

	UpdateTransforms();
	MappedBoneBuffer();

	UpdateVertex();
		
}

void GameModel::UpdateVertex()
{
	for (ModelMesh* mesh : model->Meshes())
	{
		VertexTextureNormalBlend* vertices = mesh->Vertices();

		ID3D11Buffer* vertexBuffer;
		ID3D11ShaderResourceView* vertexSrv;
		ID3D11UnorderedAccessView* outputUav;

		CsResource::CreateRawBuffer(sizeof(VertexTextureNormalBlend) * mesh->VertexCount(), vertices, &vertexBuffer);

		CsResource::CreateSrv(vertexBuffer, &vertexSrv);
		CsResource::CreateUav(mesh->VertexBuffer(), &outputUav);

		shader->AsScalar("BoneIndex")->SetInt(mesh->ParentBoneIndex());
		shader->AsShaderResource("Vertex")->SetResource(vertexSrv);
		shader->AsUAV("Output")->SetUnorderedAccessView(outputUav);

		float x = ceilf((float)mesh->VertexCount() / 256.0f);
		shader->Dispatch(0, pass, (UINT)x, 1, 1);

		SAFE_RELEASE(vertexBuffer);
		SAFE_RELEASE(vertexSrv);
		SAFE_RELEASE(outputUav);
	}
}

//본들의 부모자식 관계를 만들어주는 곳
void GameModel::UpdateTransforms()
{
	for (UINT i = 0; i < model->BoneCount(); i++)
	{
		ModelBone* bone = model->BoneByIndex(i);

		//여기는 부모자식 관계 맺을려고 만듬
		D3DXMATRIX transform;
		D3DXMATRIX parentTransform;

		int parentIndex = bone->ParentIndex();
		if (parentIndex < 0) // 뿌리노드
		{
			D3DXMatrixIdentity(&parentTransform); //parentIndex가 0보다 작으면 부모 그래서 identity잡음
		}
		else // 그 외
			 //자기의 부모의 본(boneTransform뒤에 부모인덱스가 들어가서)
			parentTransform = boneTransforms[parentIndex];

		//자기본의 글로벌
		D3DXMATRIX inv = bone->Global();
		//자신 본 위치
		//자기가 움직일거랑 부모 곱(animation * boneTransforms[parnetIndex]
		boneTransforms[i] = parentTransform;
		//원본 본의 로컬 * animation 하고 여기에 부모를 곱하면 내가 실제로간 위치
		//즉 원본본의 로컬을 기준을 잡고 내가 얼마만큼 갈지의 animation을 곱
		//그리고 원본의 위치는 부모로 부터 결정된다 그래서 마지막에 곱하기 부모를 곱한다.
		renderTransforms[i] = inv * boneTransforms[i];
	}
}

void GameModel::MappedBoneBuffer()
{
	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(boneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, renderTransforms, sizeof(D3DXMATRIX) * model->BoneCount());
	}
	D3D::GetDC()->Unmap(boneBuffer, 0);

	shader->AsShaderResource("BoneBuffer")->SetResource(boneSrv);
}

void GameModel::Update()
{
	
}

void GameModel::Render()
{
	for (ModelMesh* mesh : model->Meshes())
		mesh->Render();
}

Model * GameModel::GetModel()
{
	return model;
}

void GameModel::SetShader(wstring shaderFile)
{
	for (Material* material : model->Materials())
		material->SetShader(shaderFile);
}

void GameModel::SetDiffuse(float r, float g, float b, float a)
{
	SetDiffuse(D3DXCOLOR(r, g, b, a));
}

void GameModel::SetDiffuse(D3DXCOLOR & color)
{
	for (Material* material : model->Materials())
		material->SetDiffuse(color);
}

void GameModel::SetDiffuseMap(wstring file)
{
	for (Material* material : model->Materials())
		material->SetDiffuseMap(file);
}

void GameModel::SetSpecular(float r, float g, float b, float a)
{
	SetSpecular(D3DXCOLOR(r, g, b, a));
}

void GameModel::SetSpecular(D3DXCOLOR & color)
{
	for (Material* material : model->Materials())
		material->SetSpecular(color);
}

void GameModel::SetSpecularMap(wstring file)
{
	for (Material* material : model->Materials())
		material->SetSpecularMap(file);
}

void GameModel::SetShininess(float val)
{
	for (Material* material : model->Materials())
		material->SetShininess(val);
}

void GameModel::SetNormalMap(wstring file)
{
	for (Material* material : model->Materials())
		material->SetNormalMap(file);
}