#pragma once
#include "stdafx.h"

/*
우리가 exportor를 통해서 material과 mesh들을 FBX SDK에서 빼내오면
빼내온 것들을 우리가 저장할 변수를 만들어주어야 하는데 그변수를 우리가 원하는
형태로 만들기 위해 새로 만들어 주는 것
*/

struct FbxMaterial
{
	string Name;
		
	string DiffuseFile;
	D3DXCOLOR Diffuse;
		
	string SpecularFile;
	D3DXCOLOR Specular;
	float SpecularExp; 

	string NormalFile;
};

//본데이터 저장
struct FbxBoneData
{
	int Index;   //본의 번호
	string Name; //본의 이름

	int Parent;  //부모

	D3DXMATRIX LocalTransform;  
	D3DXMATRIX GlobalTransform; //local * 부모의 World
};

struct FbxMeshPartData
{
	string MaterialName;

	UINT StartVertex;
	UINT VertexCount;
};

struct FbxVertex
{
	int ControlPoint;
	string MaterialName;

	VertexTextureNormalBlend Vertex;
};

//실제로 그릴 데이터
struct FbxMeshData
{
	string Name;
	int ParentBone;

	FbxMesh* Mesh;

	vector<FbxVertex *> Vertices;
	vector<FbxMeshPartData *> MeshParts;
	vector<VertexTextureNormalBlend> OutVertices;
};

struct FbxKeyframeData
{
	float Time;

	D3DXMATRIX Transform;
};

struct FbxKeyframe
{
	string BoneName;

	vector<FbxKeyframeData> Transforms;
};

struct FbxClip
{
	string Name;

	int FrameCount;
	float FrameRate;
	float Duration;

	vector<FbxKeyframe *> Keyframes;
};

struct FbxBlendWeight
{
	D3DXVECTOR4 Indices = D3DXVECTOR4(0, 0, 0, 0);
	D3DXVECTOR4 Weights = D3DXVECTOR4(0, 0, 0, 0);

	void Set(UINT index, UINT boneIndex, float weight)
	{
		float i = (float)boneIndex;
		float w = weight;

		switch (index)
		{
		case 0: Indices.x = i; Weights.x = w; break;
		case 1: Indices.y = i; Weights.y = w; break;
		case 2: Indices.z = i; Weights.z = w; break;
		case 3: Indices.w = i; Weights.w = w; break;
		}
	}
};

struct FbxBoneWeights
{
private:
	typedef pair<int, float> Pair;
	vector<Pair> BoneWeights;

public:
	void AddWeights(UINT boneIndex, float boneWeights)
	{
		if (boneWeights <= 0.0f) return;

		bool bAdd = false;
		vector<Pair>::iterator it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			if (boneWeights > it->second)
			{
				BoneWeights.insert(it, Pair(boneIndex, boneWeights));
				bAdd = true;

				break;
			}

			it++;
		}//while(it)

		if (bAdd == false)
			BoneWeights.push_back(Pair(boneIndex, boneWeights));
	}

	void GetBlendWeights(FbxBlendWeight& blendWeights)
	{
		for (UINT i = 0; i < BoneWeights.size(); i++)
		{
			if (i >= 4) return;

			blendWeights.Set(i, BoneWeights[i].first, BoneWeights[i].second);
		}
	}

	void Normalize()
	{
		float totalWeight = 0.0f;

		int i = 0;
		vector<Pair>::iterator it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			if (i < 4)
			{
				totalWeight += it->second;
				i++; it++;
			}
			else
				it = BoneWeights.erase(it);
		}


		float scale = 1.0f / totalWeight;

		it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			it->second *= scale;
			it++;
		}
	}
};