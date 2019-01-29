#include "stdafx.h"
#include "FbxUtility.h"
#include "FbxType.h"


bool FbxUtility::bRightHand = false;

//Scale(1, 1, -1)
//RotationY(180)
const D3DXMATRIX FbxUtility::Negative =
{
	1,  0,  0,  0,
	0,  1,  0,  0,
	0,  0, -1,  0,
	0,  0,  0,  1
};

D3DXVECTOR2 FbxUtility::ToVector2(FbxVector2 & vec)
{
	return D3DXVECTOR2((float)vec.mData[0], (float)vec.mData[1]);
}

D3DXVECTOR3 FbxUtility::ToVector3(FbxVector4 & vec)
{
	return D3DXVECTOR3((float)vec.mData[0], (float)vec.mData[1], (float)vec.mData[2]);
}

D3DXCOLOR FbxUtility::ToColor(FbxVector4 & vec)
{
	return D3DXCOLOR((float)vec.mData[0], (float)vec.mData[1], (float)vec.mData[2], 1);
}

D3DXCOLOR FbxUtility::ToColor(FbxPropertyT<FbxDouble3>& vec, FbxPropertyT<FbxDouble>& factor)
{
	FbxDouble3 color = vec;

	D3DXCOLOR result;
	result.r = (float)color.mData[0];
	result.g = (float)color.mData[1];
	result.b = (float)color.mData[2];
	result.a = (float)factor;

	return result;
}

/*
쿼터니언은 이런 짐벌락 현상을 최대한 피하기 위해 도입되었다.
사원수는 3차원 공간의 정점에 대한 연산을 연구하다 나온 개념으로
1개의 실수부, 3개의 허수부로 나타낸다.
여기서 허수부 3가지를 3차원 벡터로 간주가 가능한데 이 벡터를 이동을 구형보간을 하게 되면
처음 벡터를 목적지로 회전시킨 것과 같게 된다.
*/
D3DXQUATERNION FbxUtility::ToQuaternion(FbxQuaternion & vec)
{
	D3DXQUATERNION result;
	result.x = (float)vec.mData[0];
	result.y = (float)vec.mData[1];
	result.z = (float)vec.mData[2];
	result.w = (float)vec.mData[3];

	return result;
}

D3DXMATRIX FbxUtility::ToMatrix(FbxAMatrix & matrix)
{
	D3DXVECTOR3 S = ToVector3(matrix.GetS());
	D3DXVECTOR3 T = ToVector3(matrix.GetT());
	D3DXQUATERNION R = ToQuaternion(matrix.GetQ());

	D3DXMATRIX s, r, t;
	D3DXMatrixScaling(&s, S.x, S.y, S.z);
	D3DXMatrixRotationQuaternion(&r, &R); //쿼터니온(사원수)을 행렬로 바꿔주는 함수
	D3DXMatrixTranslation(&t, T.x, T.y, T.z);

	if (bRightHand == true)
		return Negative * s * r * t * Negative;

	return s * r * t;
}

/*
벡터를 4x4행렬로 만들어주는것인데
좌표개념으로 사용된 것이라면 D3DXVec3TransformCoord(..)를 사용하시면 되고
방향개념이시라면  D3DXVec3TransformNormal(..)
*/

D3DXVECTOR3 FbxUtility::ToPosition(FbxVector4 & vec)
{
	D3DXVECTOR3 position = ToVector3(vec); //축에따라 위치가 바뀌어서

	//만약 direct좌표계가 오면 상관이 없고 마야업(오른손좌계가 왔을때 수행)
	if (bRightHand == true)
		D3DXVec3TransformCoord(&position, &position, &Negative);

	return position;
}

D3DXVECTOR3 FbxUtility::ToNormal(FbxVector4 & vec)
{
	D3DXVECTOR3 normal = ToVector3(vec);

	if (bRightHand == true)
		D3DXVec3TransformNormal(&normal, &normal, &Negative);

	return normal;
}

//FbxProperty안에 파일명이 있다면 텍스쳐의 파일명을 가져오는 함수
string FbxUtility::GetTextureFile(FbxProperty & prop)
{
	if (prop.IsValid() == true)
	{
		if (prop.GetSrcObjectCount() > 0)
		{
			FbxFileTexture* texture = prop.GetSrcObject<FbxFileTexture>();

			if (texture != NULL)
				return string(texture->GetFileName());
		}
	}

	return "";
}

string FbxUtility::GetMaterialName(FbxMesh * mesh, int polygonIndex, int cpIndex)
{
	FbxNode* node = mesh->GetNode();
	if (node == NULL) return "";

	FbxLayerElementMaterial* material = mesh->GetLayer(0)->GetMaterials();
	if (material == NULL) return "";


	FbxLayerElement::EMappingMode mappingMode = material->GetMappingMode();
	FbxLayerElement::EReferenceMode refMode = material->GetReferenceMode();

	int mappingIndex = -1;
	switch (mappingMode)
	{
		case FbxLayerElement::eAllSame: mappingIndex = 0; break;
		case FbxLayerElement::eByPolygon: mappingIndex = polygonIndex; break;
		case FbxLayerElement::eByControlPoint: mappingIndex = cpIndex; break;
		case FbxLayerElement::eByPolygonVertex: mappingIndex = polygonIndex * 3; break;
		default: assert(false); break;
	}


	FbxSurfaceMaterial* findMaterial = NULL;
	if (refMode == FbxLayerElement::eDirect)
	{
		if (mappingIndex < node->GetMaterialCount())
			findMaterial = node->GetMaterial(mappingIndex);
	}
	else if (refMode == FbxLayerElement::eIndexToDirect)
	{
		FbxLayerElementArrayTemplate<int>& indexArr = material->GetIndexArray();

		if (mappingIndex < indexArr.GetCount())
		{
			int tempIndex = indexArr.GetAt(mappingIndex);

			if (tempIndex < node->GetMaterialCount())
				findMaterial = node->GetMaterial(tempIndex);
		}//if(mappingIndex)
	}//if(refMode)

	if (findMaterial == NULL)
		return "";

	return findMaterial->GetName();
}

D3DXVECTOR2 FbxUtility::GetUv(FbxMesh * mesh, int cpIndex, int uvIndex)
{
	D3DXVECTOR2 result = D3DXVECTOR2(0, 0);

	FbxLayerElementUV* uv = mesh->GetLayer(0)->GetUVs();
	if (uv == NULL) return result;


	FbxLayerElement::EMappingMode mappingMode = uv->GetMappingMode();
	FbxLayerElement::EReferenceMode refMode = uv->GetReferenceMode();

	switch (mappingMode)
	{
		case FbxLayerElement::eByControlPoint:
		{
			if (refMode == FbxLayerElement::eDirect)
			{
				result.x = (float)uv->GetDirectArray().GetAt(cpIndex).mData[0];
				result.y = (float)uv->GetDirectArray().GetAt(cpIndex).mData[1];
			}
			else if (refMode == FbxLayerElement::eIndexToDirect)
			{
				int index = uv->GetIndexArray().GetAt(cpIndex);

				result.x = (float)uv->GetDirectArray().GetAt(index).mData[0];
				result.y = (float)uv->GetDirectArray().GetAt(index).mData[1];
			}
		}
		break;

		case FbxLayerElement::eByPolygonVertex:
		{
			result.x = (float)uv->GetDirectArray().GetAt(uvIndex).mData[0];
			result.y = (float)uv->GetDirectArray().GetAt(uvIndex).mData[1];
		}
		break;
	}

	if (bRightHand == true)
		return D3DXVECTOR2(result.x, 1.0f - result.y);

	return result;
}
