#pragma once

/*
animation 기본 - 이미지에 본(캐릭터의 관절 머리 척추등에 있는것)들이 있는데 이본들이
일정시간에 어디로 움직이는것이 핵심
bone animation - 본들을 가지고 움직이는 애니매이션(일반적인 물체)

skinned animation - 캐릭터같은 경우 관절같은게 있으면 회전을 한다치면 벌어
지는 부분이 존재하게 된다 그래서 벌어지는 부분의 근처에 있는 정점들이 내려가
거나 올라가서 그 공간을 채우게 된다. (blendWidth, blendHeidght)

Model 기본
bone->본이 해당 위치를 결정, 본에 해당 정점이 붙는다
material->표면재질
mesh->실제 표현할 정보 정점들이 들어있는것(skinned animation을 사용하면 blendweight, blendindices가 들어감)
animation->움직일 정보

위에 것들은 다 fbx에 들어있음

FBX -> FBX SDK로 가져온다
FBX SDK는 scene들로 이루어져있는데 
scene - material
        animation
		RootNode(tree구조로 밑에 여러 노드들이 있음) 얘가 가장 중요 각 노드들은 각 mesh 그려질 정보를 가지고 있고
		                                           그려질 정보의 world를 가지고 있다.

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

	//default parameter-> 기본값이 제공된 함수 매개 변수다. 사용자가 이 매개 변수의 값을 제공하지 않으면 기본값(default value)이 사용된다.
	//반대로 매개 변수의 값을 제공하면 사용자 제공 값이 기본값 대신 사용된다.
	void ExportMaterial(wstring saveFolder = L"", wstring fileName = L"");
	void ExportMesh(wstring saveFolder = L"", wstring fileName = L"");

	//애니메이션을 받으면 그 안에 여러가지 동작이 있을수 도 있어서 클립리스트를 만들어서 그안에 무엇이 있는지 알게 하려고 만듬
	void GetClipList(vector<wstring>* list);
	//클립번호로 빼는것
	void ExportAnimation(UINT clipNumber, wstring saveFolder = L"", wstring fileName = L"");
	//클립이름으로 빼는것
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
	FbxManager* manager;   //sdk관리자 객체를 선언하고 인스턴스 생성
	FbxImporter* importer; //씬을 가져오는 기능은 FbxImporter 클래스에 의해 추상화됩니다.
	FbxScene* scene;	   //씬 객체를 만들 때 해당 씬의 메모리를 관리할 SDK 관리자 객체를 전달해야 합니다.

	/*
	FbxIOSettings-> 씬의 요소를 파일에서 가져오거나 파일로 내보낼지 여부를 지정합니다. 
	이러한 요소에는 camera, light, mesh, texture, material, animation, 사용자 정의 속성 등이 포함됩니다.
	FbxImporter 또는 FbxExporter 객체에 전달되기 전에 인스턴스화되고 구성되어야합니다.
	*/
	FbxIOSettings* ios;

	FbxGeometryConverter* converter;

private:
	wstring fbxFile;
	wstring saveFolder;
	wstring saveName;

	//데이터를 저장
	vector<struct FbxMaterial *> materials;
	vector<struct FbxBoneData *> boneDatas;
	vector<struct FbxMeshData *> meshDatas;

	struct FbxClip * clipData;

	//unordered_map<UINT, struct FbxControlPointData> cpDatas;
	/*
	map            - pair(set) 레드블랙트리
				   
	unordered_map  - unordered_hash set 
	ex)abc를 어떤 알고리즘을 이용해서 2691020로변환 이 임의의값이 주소값으로 저장됨(비밀키)
				   
	multi_map	   - pair(set) 중복key가 허용된다(잘안씀)

	key를 어떻게 보관하고 어떻게 가져오는가 차이
	*/
};