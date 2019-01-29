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
model ����
Modle - modelMesh
            |
	    ModelMeshPart

3D���� �޾ƿ��� rootnode�� �߽����� ������ ������
�θ��ڽİ��踦 ���� �߽��� �����̸� ������ ������ ����
������ �� �ְ� �����. �� rootnode(�߽ɺ�)�� �����̳� ����

�� ������ �ڱ� ��ġ������ SRT, �׸��� �ڽ��� �̸��� ����

ex) ����Ƽ���� ���� �ϳ� �ҷ����� �������� head�� �ְ�
    �� head�� ��ġ�� �ؿ� head�� ����Ǿ��ִ� ���� ������ �ִ�
	�� head�� �����̸� �ؿ��� �˾Ƽ� ���� ������ 
	�̰� �θ��ڽİ���� �Ǿ� �־ ����

ex) Root (10, 0 ,0)
      \
	 �ڽĺ� (5, 0, 0) -> �� (5,0,0)�� �ڽĺ��� local��ġ�̴�
	                    �ٵ� ���� ��ġ�� �θ�� �����ȴ� �׷���
						������ (15,0,0)�̴�. (15,0,0)���� ��µ�

    ���� �θ��ڽ� ����� �Ǿ��ִ°� �������� �θ� �����̸� �ؿ�
	�ڽĵ鵵 �θ� ������ ��ŭ �������� �˾Ƽ� ���� �����̰� �ȴ�.
	�θ� 5��ŭ �̵���Ű�� �ؿ� �ڽĵ鵵 5��ŭ �̵��ȴ�.

	�׷��� �ڽ��� Grobal�� -> �θ��� World * �ڽ��� local (��Ʈ�������� ���ϱ�)


<model�� ������ ����>

Exporter -> material�θ��� -> bone�� �θ��� -> bone�ȿ��� mesh�� �θ���(mesh�� mesh�� meshPart�����ؼ� �θ�)
         -> ������ .material�ϰ� .mesh ����

model �ȿ� modelBoneŬ����, modelMeshŬ����, modelPartŬ����
render �ȿ� material

���� �츮�� excute�� Ŭ�������� �𵨸� �θ���
modelŬ������ �ִ� modelReader�� �ִ� readmaterial�� �ؼ� �� �����͸� MaterialŬ������ �־���
�״��� ���� ���� modelBone�� �ְ� �� �ؿ� mesh, mesh�ȿ� meshPart �̰͵��� ���

modelBone�� ��������� ������ �ڱ� index, �̸�, �θ� index, local, global
modelMesh�� ���� ��ȣ, modelmeshPart�� �׷� ������ ���� �� material�̸��ϰ� �ڱⰡ �׸� ���������� ������ ����

�������� ���� modelBone�� �ϳ��� ������ ���µ� modelmesh�ȿ� ���� ��ȣ�� �ִ� �׷��� �ش� ���� ��ġ(localTransform)�� �����ͼ�
�̰� ������ modelMesh �������� ����. modelMesh�� �����Ʈ������ ���� ������ �����ϰ� ������ �״��� mesh�ȿ� �ִ� meshPart����
�ϳ��� �������� ����. meshPart�� �ڱⰡ ������ �ִ� material�̸��� model�� ��Ī�ؼ� model���� �����´� �״��� �� material����
���̴��� �����ϰ� ��������(VertexBuffer)���� �����ؼ� �������� ����. �ٵ� ������ meshPart�� �ִ� ���������� mesh�� �̵���Ű��
�� ������������� CS�� �̿��ؼ� GPU���� ����ϴ�.

*/ 