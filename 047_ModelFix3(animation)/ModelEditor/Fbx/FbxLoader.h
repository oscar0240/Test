#pragma once

/*
animation �⺻ - �̹����� ��(ĳ������ ���� �Ӹ� ô�ߵ �ִ°�)���� �ִµ� �̺�����
�����ð��� ���� �����̴°��� �ٽ�
bone animation - ������ ������ �����̴� �ִϸ��̼�(�Ϲ����� ��ü)

skinned animation - ĳ���Ͱ��� ��� ���������� ������ ȸ���� �Ѵ�ġ�� ����
���� �κ��� �����ϰ� �ȴ� �׷��� �������� �κ��� ��ó�� �ִ� �������� ������
�ų� �ö󰡼� �� ������ ä��� �ȴ�. (blendWidth, blendHeidght)

Model �⺻
bone->���� �ش� ��ġ�� ����, ���� �ش� ������ �ٴ´�
material->ǥ������
mesh->���� ǥ���� ���� �������� ����ִ°�(skinned animation�� ����ϸ� blendweight, blendindices�� ��)
animation->������ ����

���� �͵��� �� fbx�� �������

FBX -> FBX SDK�� �����´�
FBX SDK�� scene��� �̷�����ִµ� 
scene - material
        animation
		RootNode(tree������ �ؿ� ���� ������ ����) �갡 ���� �߿� �� ������ �� mesh �׷��� ������ ������ �ְ�
		                                           �׷��� ������ world�� ������ �ִ�.

*/

namespace Xml
{
	class XMLDocument;
	class XMLElement;
}

class FbxLoader
{
public:
	FbxLoader(wstring file, wstring saveFolder, wstring saveName);
	~FbxLoader();

	//default parameter-> �⺻���� ������ �Լ� �Ű� ������. ����ڰ� �� �Ű� ������ ���� �������� ������ �⺻��(default value)�� ���ȴ�.
	//�ݴ�� �Ű� ������ ���� �����ϸ� ����� ���� ���� �⺻�� ��� ���ȴ�.
	void ExportMaterial(wstring saveFolder = L"", wstring fileName = L"");
	void ExportMesh(wstring saveFolder = L"", wstring fileName = L"");

	//�ִϸ��̼��� ������ �� �ȿ� �������� ������ ������ �� �־ Ŭ������Ʈ�� ���� �׾ȿ� ������ �ִ��� �˰� �Ϸ��� ����
	void GetClipList(vector<wstring>* list);
	//Ŭ����ȣ�� ���°�
	void ExportAnimation(UINT clipNumber, wstring saveFolder = L"", wstring fileName = L"");
	//Ŭ���̸����� ���°�
	void ExportAnimation(wstring clipName, wstring saveFolder = L"", wstring fileName = L"");

private:
	void ReadMaterial();
	void WriteMaterial(wstring saveFolder, wstring fileName);

	void ReadBoneData(FbxNode* node, int index, int parent);
	void ReadMeshData(FbxNode* node, int parentBone);
	void ReadSkinData();
	void WriteMeshData(wstring saveFolder, wstring fileName);

	void CopyTextureFile(OUT string& textureFile, wstring& saveFolder);
	void WriteXmlColor(Xml::XMLDocument* document, Xml::XMLElement* element, D3DXCOLOR& color);
	
	void ReadAnimationData(UINT index);
	void ReadAnimationData(wstring name);
	void ReadKeyframeData(FbxNode* node, FbxLongLong start, FbxLongLong end);

	void WriteAnimationData(wstring saveFolder, wstring fileName);

private:
	FbxManager* manager;   //sdk������ ��ü�� �����ϰ� �ν��Ͻ� ����
	FbxImporter* importer; //���� �������� ����� FbxImporter Ŭ������ ���� �߻�ȭ�˴ϴ�.
	FbxScene* scene;	   //�� ��ü�� ���� �� �ش� ���� �޸𸮸� ������ SDK ������ ��ü�� �����ؾ� �մϴ�.

	/*
	FbxIOSettings-> ���� ��Ҹ� ���Ͽ��� �������ų� ���Ϸ� �������� ���θ� �����մϴ�. 
	�̷��� ��ҿ��� camera, light, mesh, texture, material, animation, ����� ���� �Ӽ� ���� ���Ե˴ϴ�.
	FbxImporter �Ǵ� FbxExporter ��ü�� ���޵Ǳ� ���� �ν��Ͻ�ȭ�ǰ� �����Ǿ���մϴ�.
	*/
	FbxIOSettings* ios;

	FbxGeometryConverter* converter;

private:
	wstring fbxFile;
	wstring saveFolder;
	wstring saveName;

	//�����͸� ����
	vector<struct FbxMaterial *> materials;
	vector<struct FbxBoneData *> boneDatas;
	vector<struct FbxMeshData *> meshDatas;

	struct FbxClip * clipData;

	//unordered_map<UINT, struct FbxControlPointData> cpDatas;
	/*
	map            - pair(set) �����Ʈ��
				   
	unordered_map  - unordered_hash set 
	ex)abc�� � �˰����� �̿��ؼ� 2691020�κ�ȯ �� �����ǰ��� �ּҰ����� �����(���Ű)
				   
	multi_map	   - pair(set) �ߺ�key�� ���ȴ�(�߾Ⱦ�)

	key�� ��� �����ϰ� ��� �������°� ����
	*/
};