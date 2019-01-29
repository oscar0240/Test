#pragma once

class ModelMesh
{
public:
	friend class Model;
	friend class Models;

public:
	void Render();

	wstring Name() { return name; }

	int ParentBoneIndex() { return parentBoneIndex; }
	class ModelBone* ParentBone() { return parentBone; }

	void Copy(ModelMesh** clone);

	void Pass(UINT pass);

	ID3D11Buffer* VertexBuffer() { return vertexBuffer; }
	VertexTextureNormalBlend* Vertices() { return vertices; }
	UINT VertexCount() { return vertexCount; }

private:
	void Binding();

private:
	ModelMesh();
	~ModelMesh();

	wstring name;

	int parentBoneIndex;
	class ModelBone* parentBone;

	vector<class ModelMeshPart *> meshParts;

	UINT vertexCount;
	VertexTextureNormalBlend* vertices;
	ID3D11Buffer* vertexBuffer;
};

/*
Mesh
  \
 meshPart
  \
 meshPart

mesh란 메쉬파트들의 그룹
그러니까 전체 메쉬가 있고 material(색등)이 달라지는 구간에서는 meshPart로
분할해준다.
ex) 팔을 잘랐을때 팔과 손이 색이 다르면 팔이 meshPart1 손이 meshPart2
*/