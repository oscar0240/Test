#pragma once

class ModelBone;
class ModelMesh;

class Model
{
public:
	Model();
	~Model();

	UINT MaterialCount() { return materials.size(); }
	vector<Material *>& Materials() { return materials; }
	Material* MaterialByIndex(UINT index) { return materials[index]; }
	Material* MaterialByName(wstring name);

	UINT BoneCount() { return bones.size(); }
	vector<ModelBone *>& Bones() { return bones; }
	ModelBone* BoneByIndex(UINT index) { return bones[index]; }
	ModelBone* BoneByName(wstring name);

	UINT MeshCount() { return meshes.size(); }
	vector<ModelMesh *>& Meshes() { return meshes; }
	ModelMesh* MeshByIndex(UINT index) { return meshes[index]; }
	ModelMesh* MeshByName(wstring name);

	void ReadMaterial(wstring file);
	void ReadMesh(wstring file);

	void CopyGlobalBoneTo(vector<D3DXMATRIX>& transforms);
	void CopyGlobalBoneTo(vector<D3DXMATRIX>& transforms, D3DXMATRIX& w);

private:
	void BindingBone();
	void BindingMesh();

private:
	class ModelBone* root;

	vector<Material *> materials;
	vector<class ModelMesh *> meshes;
	vector<class ModelBone *> bones;
};

class Models
{
public:
	friend class Model;

private:
	static void LoadMaterial(wstring file, vector<Material *>* materials);
	static void ReadMaterialData(wstring file);

	static void LoadMesh(wstring file, vector<ModelBone *>* bones, vector<ModelMesh *>* meshes);
	static void ReadMeshData(wstring file);

private:
	static map<wstring, vector<Material *>> materialMap;

	struct MeshData
	{
		vector<class ModelBone *> Bones;
		vector<class ModelMesh *> Meshes;
	};
	static map<wstring, MeshData> meshDataMap;
};

/*
model 구조
Modle - modelMesh
            |
	    ModelMeshPart

3D모델을 받아오면 rootnode를 중심으로 나머지 본들을
부모자식관계를 만들어서 중심이 움직이면 나머지 본들이 같이
움직일 수 있게 만든다. 이 rootnode(중심본)은 디자이너 마음

이 본들은 자기 위치에대한 SRT, 그리고 자신의 이름을 가짐

ex) 유니티에서 모델을 하나 불러오면 가장위에 head가 있고
    그 head를 펼치면 밑에 head에 연결되어있는 많은 본들이 있다
	이 head를 움직이면 밑에는 알아서 같이 움직임 
	이게 부모자식관계로 되어 있어서 가능

ex) Root (10, 0 ,0)
      \
	 자식본 (5, 0, 0) -> 이 (5,0,0)이 자식본의 local위치이다
	                    근데 실제 위치는 부모와 누적된다 그래서
						실제는 (15,0,0)이다. (15,0,0)으로 출력됨

    모델이 부모자식 관계로 되어있는게 가장위에 부모만 움직이면 밑에
	자식들도 부모가 움직인 만큼 더해져서 알아서 같이 움직이게 된다.
	부모를 5만큼 이동시키면 밑에 자식들도 5만큼 이동된다.

	그래서 자신의 Grobal는 -> 부모의 World * 자신의 local (매트릭스여서 곱하기)


<model이 나오는 순서>

Exporter -> material부르고 -> bone을 부르고 -> bone안에서 mesh를 부른다(mesh때 mesh와 meshPart구분해서 부름)
         -> 폴더에 .material하고 .mesh 생김

model 안에 modelBone클래스, modelMesh클래스, modelPart클래스
render 안에 material

이제 우리가 excute의 클래스에서 모델르 부를때
model클래스에 있는 modelReader에 있던 readmaterial를 해서 이 데이터를 Material클래스에 넣어줌
그다음 제일 위에 modelBone이 있고 그 밑에 mesh, mesh안에 meshPart 이것들을 사용

modelBone은 행렬정보만 들어가있음 자기 index, 이름, 부모 index, local, global
modelMesh는 본의 번호, modelmeshPart의 그룹 가지고 있음 즉 material이름하고 자기가 그릴 정점정보만 가지고 있음

랜더링에 들어갈때 modelBone이 하나씩 렌더링 들어가는데 modelmesh안에 본의 번호가 있다 그래서 해당 본의 위치(localTransform)를 가져와서
이걸 가지고 modelMesh 렌더링에 들어간다. modelMesh의 월드매트릭스에 본의 로컬을 셋팅하고 렌더링 그다음 mesh안에 있는 meshPart들이
하나씩 렌더링에 들어간다. meshPart는 자기가 가지고 있던 material이름을 model과 매칭해서 model에서 가져온다 그다음 그 material값을
쉐이더에 셋팅하고 정점정보(VertexBuffer)까지 셋팅해서 렌더링에 들어간다. 근데 지금은 meshPart에 있던 정점정보를 mesh로 이동시키고
이 정점정보계산을 CS를 이용해서 GPU에서 계산하다.

*/ 