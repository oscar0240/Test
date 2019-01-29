#include "stdafx.h"
#include "FbxLoader.h"
#include "FbxType.h"
#include "FbxUtility.h"
#include "Utilities/Xml.h"
#include "Utilities/BinaryFile.h"

FbxLoader::FbxLoader(wstring file, wstring saveFolder, wstring saveName)
	: fbxFile(file), saveFolder(saveFolder), saveName(saveName)
	, clipData(NULL)
{
	//클래스의 create()멤버 함수를 호출, 이때 첫 번째 매개 변수로 객체가 속한 씬 객체를 전달합니다.
	//fbx sdk 관리자객체 생성
	manager = FbxManager::Create();
	scene = FbxScene::Create(manager, "");

	//IOSettings 객체 생성 및 설정
	ios = FbxIOSettings::Create(manager, IOSROOT);
	ios->SetBoolProp(IMP_FBX_TEXTURE, true);
	manager->SetIOSettings(ios);

	//fbximpoter 객체 생성
	//유니티에 importer와 같은 기능 fbx sdk를 우리가 쓸 scene으로 불러올거임
	importer = FbxImporter::Create(manager, "");

	//importer 초기화(Initialize의 맨앞 매개변수는 불러올 파일 이름)
	string loadFile = String::ToString(file);
	bool b = importer->Initialize(loadFile.c_str(), -1, ios);
	assert(b == true);
	
	//fbx 파일 내용을 scene으로 가져온다
	b = importer->Import(scene);
	assert(b == true);

	//축 가져오기
	FbxAxisSystem axis = scene->GetGlobalSettings().GetAxisSystem();
	FbxUtility::bRightHand = axis.GetCoorSystem() == FbxAxisSystem::eRightHanded;
	
	//시스템 단위 눈금자 가져오기
	//FbxSystemUnit systemUnit = scene->GetGlobalSettings().GetSystemUnit();
	//if (systemUnit != FbxSystemUnit::m) //내가 불러오는 모델의 단위를 다 m단위로 바꿔주는것
	//{
	//	FbxSystemUnit::ConversionOptions option =
	//	{
	//		/*
	//		1.부모크기를 계속 상속받아서 할것이가? 우리는 각가의 본들을 사용하기 때문에 false
	//		2.최대크기를 변환?
	//		3.cluster 여러개의 시스템으로 연결된것을 하나의 시스템으로?
	//		4.빛에 대한 노멀벡터를 변환?
	//		5.측광라이트속성
	//		6.카메라의 클립면을 변환?
	//		*/
	//		false, true, true, true, true, true
	//	};
	//	FbxSystemUnit::m.ConvertScene(scene, option);
	//}

	//씬 내에서 삼각형화 할 수 있는 모든 노드를 삼각형화 시킨다.
	converter = new FbxGeometryConverter(manager);

	/*
	FBX Nodes
	노드는 주로 씬 내의 씬 요소의 위치, 회전 및 크기를 지정하는 데 사용됩니다. 
	노드는 FbxNode 클래스에 의해 추상화됩니다. FbxScene에는 노드의 부모-자식 계층이 있습니다. 
	이 트리의 루트 노드는 위에서 설명 했듯이 FbxScene::GetRootNode()를 통해 접근 가능합니다.
	*/
}

FbxLoader::~FbxLoader()
{
	//메모리 줄이기 위해서 삭제
	SAFE_DELETE(converter);
	ios->Destroy();
	importer->Destroy();
	scene->Destroy();
	manager->Destroy();
}

//FBX SDK에 들어있는 우리가 필요한 material만 꺼내온다.(애는 xml파일을 이용 열어서 편집을해야할 경우가 많아서 사용)
void FbxLoader::ExportMaterial(wstring saveFolder, wstring fileName)
{
	ReadMaterial();

	//그냥 exportMaterial()괄호 안에 아무것도 안쓰면 자신의 폴더와 이름이 들어간다.
	wstring tempFolder = saveFolder.size() < 1 ? this->saveFolder : saveFolder;
	wstring tempName = fileName.size() < 1 ? this->saveName : fileName;

	//tempName뒤에 확장자를 자동으로 붙혀줌
	WriteMaterial(tempFolder, tempName + L".material");
}

//RootNode를 .mesh라고 꺼내오는 함수(이것은 이진파일 text파일로 읽으면 파일의 크기가 크면 느리기 때문에 이진파일사용)
void FbxLoader::ExportMesh(wstring saveFolder, wstring fileName)
{
	ReadBoneData(scene->GetRootNode(), -1, -1); //node들이 트리형태여서 재귀함수를 탄다
	ReadSkinData();

	wstring tempFolder = saveFolder.size() < 1 ? this->saveFolder : saveFolder;
	wstring tempName = fileName.size() < 1 ? this->saveName : fileName;

	WriteMeshData(tempFolder, tempName + L".mesh");
}

void FbxLoader::GetClipList(vector<wstring>* list)
{
	list->clear();

	//fbx에서는 clip(애니메이션동작들)을 animationStack이라고 부름
	for (int i = 0; i < importer->GetAnimStackCount(); i++)
	{
		FbxTakeInfo* takeInfo = importer->GetTakeInfo(i);

		string name = takeInfo->mName.Buffer();
		list->push_back(String::ToWString(name));
	}
}

void FbxLoader::ExportAnimation(UINT clipNumber, wstring saveFolder, wstring fileName)
{
	ReadAnimationData(clipNumber);

	wstring tempFolder = saveFolder.size() < 1 ? this->saveFolder : saveFolder;
	wstring tempName = fileName.size() < 1 ? this->saveName : fileName;

	WriteAnimationData(tempFolder, tempName + L".animation");
}

void FbxLoader::ExportAnimation(wstring clipName, wstring saveFolder, wstring fileName)
{
	ReadAnimationData(clipName);

	wstring tempFolder = saveFolder.size() < 1 ? this->saveFolder : saveFolder;
	wstring tempName = fileName.size() < 1 ? this->saveName : fileName;

	WriteAnimationData(tempFolder, tempName + L".animation");
}

//material은 scene에 붙어있다.
void FbxLoader::ReadMaterial()
{
	//scene이 가지고 있는 전체 material개수
	int count = scene->GetMaterialCount();

	for (int i = 0; i < count; i++)
	{
		//fbxMaterial를 통해서 우리가 원하는 material 정보를 빼온다.
		FbxSurfaceMaterial* fbxMaterial = scene->GetMaterial(i);

		//빼내온 변수를 우리가 원하는 형태로 만들어 주고 그걸 씀
		FbxMaterial* material = new FbxMaterial();
		//이렇게 하면 해당 scene에 저장되어있던 material의 이름이 넘어온다
		//이 이름이 중요 나중에 이름으로 매칭
		material->Name = fbxMaterial->GetName();

		//Dynamic cast check여부
		if (fbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId) == true)
		{
			//캐스팅이 가능하면 실행
			//lambert -> diffuse(자기색을 표현)
			//DiffuseFactor -> 빛을 얼마나 받을지?
			FbxSurfaceLambert* lambert = (FbxSurfaceLambert *)fbxMaterial;
			material->Diffuse = FbxUtility::ToColor(lambert->Diffuse, lambert->DiffuseFactor);
		}

		if (fbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId) == true)
		{
			//정반사값
			FbxSurfacePhong* phong = (FbxSurfacePhong *)fbxMaterial;
			material->Specular = FbxUtility::ToColor(phong->Specular, phong->SpecularFactor);
			material->SpecularExp = (float)phong->Shininess;
		}

		//텍스쳐 불러오려고 씀
		FbxProperty prop;

		//diffues, specular normal의 파일명을 불러오는과정
		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		material->DiffuseFile = FbxUtility::GetTextureFile(prop);

		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
		material->SpecularFile = FbxUtility::GetTextureFile(prop);

		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
		material->NormalFile = FbxUtility::GetTextureFile(prop);

		//위의 데이터들을 저장
		materials.push_back(material);
	}
}

void FbxLoader::WriteMaterial(wstring saveFolder, wstring fileName)
{
	//폴더가 없으면 자동으로 폴더를 만들어주는 함수
	Path::CreateFolders(saveFolder);

	//xml파일 우리가 변경을 쉽게 하기위해 text파일로 불러서 변경
	Xml::XMLDocument* document = new Xml::XMLDocument();
	Xml::XMLDeclaration *decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement* root = document->NewElement("Materials");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	//stl에서 iterator를 가진 객체를 하나씩 순회해주는 방식 foreach
	for (FbxMaterial* material : materials)
	{
		Xml::XMLElement* node = document->NewElement("Material");
		root->LinkEndChild(node);


		Xml::XMLElement* element = NULL;

		element = document->NewElement("Name");
		element->SetText(material->Name.c_str());
		node->LinkEndChild(element);

		//우리가 필요한 것들을 xml로 만든것을 CopyTextureFile함수를 이용해서 _Model파일로 복사해준다
		element = document->NewElement("DiffuseFile");
		CopyTextureFile(material->DiffuseFile, saveFolder);
		element->SetText(material->DiffuseFile.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("SpecularFile");
		CopyTextureFile(material->SpecularFile, saveFolder);
		element->SetText(material->SpecularFile.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("NormalFile");
		CopyTextureFile(material->NormalFile, saveFolder);
		element->SetText(material->NormalFile.c_str());
		node->LinkEndChild(element);

		
		element = document->NewElement("Diffuse");
		node->LinkEndChild(element);

		WriteXmlColor(document, element, material->Diffuse);

		element = document->NewElement("Specular");
		node->LinkEndChild(element);

		WriteXmlColor(document, element, material->Specular);

		//SpecularExp -> shininess
		element = document->NewElement("SpecularExp");
		element->SetText(material->SpecularExp);
		node->LinkEndChild(element);

		SAFE_DELETE(material);
	}

	//우리가 만든 xml파일 저장
	string file = String::ToString(saveFolder + fileName);
	document->SaveFile(file.c_str());
}

void FbxLoader::ReadBoneData(FbxNode * node, int index, int parent)
{
	//노드는 자기가 가지는 속성들이 있다 그래서 우리가 일부만 가져옴
	FbxNodeAttribute* attribute = node->GetNodeAttribute();

	if (attribute != NULL)
	{
		//노드 속성중에 타입이 있다
		FbxNodeAttribute::EType nodeType = attribute->GetAttributeType();

		//우리가 쓸 타입들
		bool b = false;
		b |= (nodeType == FbxNodeAttribute::eSkeleton);  //본
		b |= (nodeType == FbxNodeAttribute::eMesh);		 //우리가 그릴 데이터를 가지고 있는것
		b |= (nodeType == FbxNodeAttribute::eNull);      //그냥 임의로
		b |= (nodeType == FbxNodeAttribute::eMarker);	 //머리위에 아이디를 띄워야할때 머리위에 마커라는 본을 넣어놈

		if (b == true)
		{
			//본 데이터 만들기
			FbxBoneData* bone = new FbxBoneData();
			bone->Index = index;
			bone->Parent = parent;
			bone->Name = node->GetName();
			//FBX matrix를 DX matrix로 바꿔줌
			bone->LocalTransform = FbxUtility::ToMatrix(node->EvaluateLocalTransform());
			bone->GlobalTransform = FbxUtility::ToMatrix(node->EvaluateGlobalTransform());
			boneDatas.push_back(bone);

			if (nodeType == FbxNodeAttribute::eMesh)
			{
				converter->Triangulate(attribute, true, true);

				ReadMeshData(node, index);
			}	
		}
	}

	//재귀를 탄다 자기의 자식의 개수 만큼 탄다.
	for (int i = 0; i < node->GetChildCount(); i++)
		ReadBoneData(node->GetChild(i), boneDatas.size(), index);
}

void FbxLoader::ReadMeshData(FbxNode * node, int parentBone)
{
	/*
	FbxMesh
	메쉬란 다각형으로 만들어진 지오메트리.
	메쉬는 흔히 정점(vertices)으로 불리는 제어점(control point)의 리스트로 정의됩니다.
	제어점(control point)은 XYZ좌표이며 꼭짓점과 같습니다.
	FbxNode::GetMesh() 멤버 함수를 호출해 FbxMesh로 캐스팅된 노드 속성의 포인터를 가져올 수 있습니다.
	*/
	//ControlPoint -> 해당정점의 번호와 위치만 가지고 있는것
	//cp에서 추가로 vertexWeight, vertexIndices를 가지는데 이것은
	//vertexIndices->움직일 정점의 가중치의 번호, vertexWeight->번호의 위치를 가져다가 얼마만큼 이동
	FbxMesh* mesh = node->GetMesh();

	//deformer->cluster(관절)을 총괄하는 얘 ex)팔이면 여러관절이 있는데 그녁석들을 총괄
	int deformerCount = mesh->GetDeformerCount();
	
	//mesh의 정보들 저장
	//위에서 모든 노드를 삼각형화해서 메쉬의 polygon은 삼각형으로 구성
	vector<FbxVertex *> vertices;
	for (int p = 0; p < mesh->GetPolygonCount(); p++) //삼각형의 개수
	{
		int vertexInPolygon = mesh->GetPolygonSize(p); //해당 폴리곤에 있는 정점의 개수가 넘어감
		assert(vertexInPolygon == 3);

		for (int vi = 0; vi < 3; vi++) //삼각형은 3개의 정점으로 구성
		{
			//GetCoorSystem->좌표시스템을 가져온다(왼손? 오른손?)
			//왼손좌표계는 시계방향으로 회전
			int pvi = FbxUtility::bRightHand ? 2 - vi : vi;
			int cpIndex = mesh->GetPolygonVertex(p, pvi); //제어점 인덱스를 가져옴

			FbxVertex* vertex = new FbxVertex();
			vertex->ControlPoint = cpIndex;

			FbxVector4 position = mesh->GetControlPointAt(cpIndex); //현재 정점에 대한 위치
			vertex->Vertex.Position = FbxUtility::ToPosition(position);

			//노말화 해주기
			FbxVector4 normal;
			mesh->GetPolygonVertexNormal(p, pvi, normal);
			vertex->Vertex.Normal = FbxUtility::ToNormal(normal);
			

			vertex->MaterialName = FbxUtility::GetMaterialName(mesh, p, cpIndex);

			int uvIndex = mesh->GetTextureUVIndex(p, pvi); //uv정점 인덱스
			vertex->Vertex.Uv = FbxUtility::GetUv(mesh, cpIndex, uvIndex);

			vertices.push_back(vertex);
		}//for(vi)
	}//for(p)

	//위에서 구한 메쉬의 정보를 meshData에 넣어주는 것
	FbxMeshData* meshData = new FbxMeshData();
	meshData->Name = node->GetName();
	meshData->ParentBone = parentBone;
	meshData->Vertices = vertices;
	meshData->Mesh = mesh;
	meshDatas.push_back(meshData);
}

//바로 위에 적재되어있는 메쉬를 대상으로 스킨을 입힌다(여기 함수가 가장 중요)
void FbxLoader::ReadSkinData()
{
	for (FbxMeshData* meshData : meshDatas)
	{
		FbxMesh* mesh = meshData->Mesh;

		//관절을 총괄하는애(피부라고 생각)
		int deformerCount = mesh->GetDeformerCount();

		vector<FbxBoneWeights> boneWeights;
		boneWeights.assign(mesh->GetControlPointsCount(), FbxBoneWeights());

		for (int i = 0; i < deformerCount; i++)
		{
			FbxDeformer* deformer = mesh->GetDeformer(i, FbxDeformer::eSkin);

			//reinterpret_cast->임의의 포인터 타입끼리 변환을 허용하는 캐스트 연산자, 정수형을 포인터로 바꿀 수도있다
			//상속관계가 아니라면 실패해버린다
			FbxSkin* skin = reinterpret_cast<FbxSkin *>(deformer);
			if (skin == NULL) continue;

			for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); clusterIndex++)
			{
				//해당 관절 가져오기
				FbxCluster* cluster = skin->GetCluster(clusterIndex);
				assert(cluster->GetLinkMode() == FbxCluster::eNormalize);

				//관절하고 연결되어있는것이 본이기때문에
				//GetLink가 리턴해주는 것이 fbxBone 그래서 그 본의 이름을 가져온다
				string linkName = cluster->GetLink()->GetName();

				UINT boneIndex = -1;
				for (UINT i = 0; i < boneDatas.size(); i++)
				{
					if (boneDatas[i]->Name == linkName)
					{
						boneIndex = i;

						break;        //내가 찾고자하는 본을 찾은것
					}
				}

				/*
				관절이 같이 안움직이는 모델도 있기때문에 관절을 이용해서
				위에서 우리가 본들의 리스트를 불렀고 그중에서
				cluster와 연결되어있는 같은본을 찾아서 이름을 찾고 그 본의 위치를 계산
				*/
				FbxAMatrix transform;     //자기 자신에 대한 local
				FbxAMatrix linkTransform; //부모에 대한 world

				cluster->GetTransformMatrix(transform);
				cluster->GetTransformLinkMatrix(linkTransform);
				//관절을 통해서 해당 본에 매트릭스 심어주는것
				boneDatas[boneIndex]->LocalTransform = FbxUtility::ToMatrix(transform);
				boneDatas[boneIndex]->GlobalTransform = FbxUtility::ToMatrix(linkTransform);

				for (int indexCount = 0; indexCount < cluster->GetControlPointIndicesCount(); indexCount++)
				{
					/*
					관절을 기준으로 정점을 끌어내리거나 올려서 피부를 매꿔야하기 때문에 움직이는 정점
					vertexIndices, vertexWeights가 관절에 들어있다
					*/
					int temp = cluster->GetControlPointIndices()[indexCount];
					//관절의 가중치
					double* weights = cluster->GetControlPointWeights();

					boneWeights[temp].AddWeights(boneIndex, (float)weights[indexCount]);
				}
			}//for(clusterIndex)
		}//for(deformer)

		for (UINT i = 0; i < boneWeights.size(); i++)
			boneWeights[i].Normalize();

		for (FbxVertex* vertex : meshData->Vertices)
		{
			int cpIndex = vertex->ControlPoint;

			FbxBlendWeight weights;
			boneWeights[cpIndex].GetBlendWeights(weights); //가중치값 가져오기
			vertex->Vertex.BlendIndices = weights.Indices; //해당 정점에 가중치의 indices 넣어주기
			vertex->Vertex.BlendWeights = weights.Weights; //해당 정점에 가중치 넣어주기
		}


		for (int i = 0; i < scene->GetMaterialCount(); i++)
		{
			FbxSurfaceMaterial* material = scene->GetMaterial(i);
			string materialName = material->GetName();

			vector<FbxVertex *> gather;
			for (FbxVertex* temp : meshData->Vertices)
			{
				if (temp->MaterialName == materialName)
					gather.push_back(temp);
			}
			if (gather.size() < 1) continue;


			FbxMeshPartData* meshPart = new FbxMeshPartData();
			meshPart->MaterialName = materialName;


			vector<VertexTextureNormalBlend> vertices;
			for (FbxVertex* temp : gather)
			{
				VertexTextureNormalBlend vertex;
				vertex = temp->Vertex;

				vertices.push_back(vertex);
			}

			meshPart->StartVertex = meshData->OutVertices.size();
			meshPart->VertexCount = vertices.size();

			meshData->OutVertices.insert(meshData->OutVertices.end(), vertices.begin(), vertices.end());
			meshData->MeshParts.push_back(meshPart);
		}
	}//for(MeshData)
}

void FbxLoader::WriteMeshData(wstring saveFolder, wstring fileName)
{
	Path::CreateFolders(saveFolder);

	BinaryWriter* w = new BinaryWriter();
	w->Open(saveFolder + fileName);

	w->UInt(boneDatas.size());  //이진 파일은 몇개가 나올지 몰라서 데이터의 사이즈를 써준다 그래서 얘의 갯수만큼 읽는다
	for (FbxBoneData* bone : boneDatas)
	{
		//fbxBoneData에 있는 변수들 순서대로 써준다
		w->Int(bone->Index);
		w->String(bone->Name);

		w->Int(bone->Parent);

		w->Matrix(bone->LocalTransform);
		w->Matrix(bone->GlobalTransform);

		SAFE_DELETE(bone);
	}


	w->UInt(meshDatas.size());
	for (FbxMeshData* meshData : meshDatas)
	{
		w->String(meshData->Name);
		w->Int(meshData->ParentBone);

		w->UInt(meshData->OutVertices.size());
		w->Byte(&meshData->OutVertices[0], sizeof(VertexTextureNormalBlend) * meshData->OutVertices.size());

		//meshPartdata 쓰기
		w->UInt(meshData->MeshParts.size());
		for (FbxMeshPartData* part : meshData->MeshParts)
		{
			w->String(part->MaterialName);

			w->UInt(part->StartVertex);
			w->UInt(part->VertexCount);

			SAFE_DELETE(part);
		}

		SAFE_DELETE(meshData);
	}

	//반드시 닫아줘야지 다른 파일을 연다
	w->Close();
	SAFE_DELETE(w);
}

//우리가 파일에 _Model파일을 만들었는데 model파일안에 textureFile을 복사해주려고 만듬
void FbxLoader::CopyTextureFile(OUT string & textureFile, wstring & saveFolder)
{
	//textureFile에 diffusefile의 경로가 들어온다
	if (textureFile.length() < 1)
		return;

	
	wstring file = String::ToWString(textureFile);
	wstring fileName = Path::GetFileName(file);  //전체경로에서 파일명만 가져오는 함수

	//들어온 diffuseFile을 복사해주는것
	if (Path::ExistFile(textureFile) == true)
		CopyFile(file.c_str(), (saveFolder + fileName).c_str(), FALSE);

	textureFile = String::ToString(fileName);
}

void FbxLoader::WriteXmlColor(Xml::XMLDocument * document, Xml::XMLElement * element, D3DXCOLOR & color)
{
	Xml::XMLElement* r = document->NewElement("R");
	r->SetText(color.r);
	element->LinkEndChild(r);

	Xml::XMLElement* g = document->NewElement("G");
	g->SetText(color.g);
	element->LinkEndChild(g);

	Xml::XMLElement* b = document->NewElement("B");
	b->SetText(color.b);
	element->LinkEndChild(b);

	Xml::XMLElement* a = document->NewElement("A");
	a->SetText(color.a);
	element->LinkEndChild(a);
}

void FbxLoader::ReadAnimationData(UINT index)
{
	FbxTakeInfo* info = importer->GetTakeInfo(index);
	assert(info != NULL);

	ReadAnimationData(String::ToWString(info->mName.Buffer()));
}

void FbxLoader::ReadAnimationData(wstring name)
{
	FbxTime::EMode timeMode = scene->GetGlobalSettings().GetTimeMode();
	FbxTakeInfo* takeInfo = scene->GetTakeInfo(String::ToString(name).c_str());
	FbxTimeSpan timeSpan = takeInfo->mLocalTimeSpan;

	FbxLongLong start = timeSpan.GetStart().GetFrameCount(timeMode);
	FbxLongLong stop = timeSpan.GetStop().GetFrameCount(timeMode);

	SAFE_DELETE(clipData);

	clipData = new FbxClip();
	clipData->Name = String::ToString(name);
	clipData->FrameCount = (int)(stop - start + 1);
	clipData->FrameRate = (float)FbxTime::GetFrameRate(timeMode);
	clipData->Duration = (float)timeSpan.GetDuration().GetMilliSeconds();
	
	//루프노프로 부터 재귀타기 시작
	ReadKeyframeData(scene->GetRootNode(), start, stop);
}

/*
각 본에 해당하는 시간에 대한 srt 트랜스폼을 가져와서 저장을 한다.

원래는 내가 1초에 본이 어딘가로 이동했으면 그 초에 대한 이동만 저장하는데
우리는 매 프레임별로 srt를 다 저장한다
*/
void FbxLoader::ReadKeyframeData(FbxNode * node, FbxLongLong start, FbxLongLong end)
{
	FbxNodeAttribute* attribute = node->GetNodeAttribute();

	if (attribute != NULL)
	{
		if (attribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		{
			FbxKeyframe* keyframe = new FbxKeyframe();
			keyframe->BoneName = node->GetName();

			for (FbxLongLong i = start; i <= end; i++)
			{
				FbxTime curTime;
				curTime.SetFrame(i);

				FbxKeyframeData keyframeData;
				keyframeData.Time = (float)curTime.GetMilliSeconds();

				FbxAMatrix matrix = node->EvaluateLocalTransform(curTime);
				keyframeData.Transform = FbxUtility::ToMatrix(matrix);

				keyframe->Transforms.push_back(keyframeData);
			}

			clipData->Keyframes.push_back(keyframe);
		}
	}

	for (int i = 0; i < node->GetChildCount(); i++)
		ReadKeyframeData(node->GetChild(i), start, end);
}

void FbxLoader::WriteAnimationData(wstring saveFolder, wstring fileName)
{
	Path::CreateFolders(saveFolder);

	BinaryWriter* w = new BinaryWriter();
	w->Open(saveFolder + fileName);

	//frameRate * frameCount = duration
	w->String(clipData->Name);     //클립이름
	w->Float(clipData->Duration);  //클립전체시간
	w->Float(clipData->FrameRate); //클립프레임비율
	w->UInt(clipData->FrameCount); //클립프레임카운트
	
	w->UInt(clipData->Keyframes.size());
	for (UINT i = 0; i < clipData->Keyframes.size(); i++)
	{
		FbxKeyframe* frame = clipData->Keyframes[i];
		w->String(frame->BoneName); //프레임당 본의 이름넣고

		w->UInt(frame->Transforms.size());
		for (UINT k = 0; k < frame->Transforms.size(); k++)
		{
			FbxKeyframeData& data = frame->Transforms[k];
			w->Float(data.Time);

			D3DXVECTOR3 S, T;
			D3DXQUATERNION R;

			D3DXMatrixDecompose(&S, &R, &T, &data.Transform);

			w->Vector3(S);
			w->Quaternion(R);
			w->Vector3(T);
		}
	}

	w->Close();
	SAFE_DELETE(w);
}
