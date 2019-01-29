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
	//Ŭ������ create()��� �Լ��� ȣ��, �̶� ù ��° �Ű� ������ ��ü�� ���� �� ��ü�� �����մϴ�.
	//fbx sdk �����ڰ�ü ����
	manager = FbxManager::Create();
	scene = FbxScene::Create(manager, "");

	//IOSettings ��ü ���� �� ����
	ios = FbxIOSettings::Create(manager, IOSROOT);
	ios->SetBoolProp(IMP_FBX_TEXTURE, true);
	manager->SetIOSettings(ios);

	//fbximpoter ��ü ����
	//����Ƽ�� importer�� ���� ��� fbx sdk�� �츮�� �� scene���� �ҷ��ð���
	importer = FbxImporter::Create(manager, "");

	//importer �ʱ�ȭ(Initialize�� �Ǿ� �Ű������� �ҷ��� ���� �̸�)
	string loadFile = String::ToString(file);
	bool b = importer->Initialize(loadFile.c_str(), -1, ios);
	assert(b == true);
	
	//fbx ���� ������ scene���� �����´�
	b = importer->Import(scene);
	assert(b == true);

	//�� ��������
	FbxAxisSystem axis = scene->GetGlobalSettings().GetAxisSystem();
	FbxUtility::bRightHand = axis.GetCoorSystem() == FbxAxisSystem::eRightHanded;
	
	//�ý��� ���� ������ ��������
	//FbxSystemUnit systemUnit = scene->GetGlobalSettings().GetSystemUnit();
	//if (systemUnit != FbxSystemUnit::m) //���� �ҷ����� ���� ������ �� m������ �ٲ��ִ°�
	//{
	//	FbxSystemUnit::ConversionOptions option =
	//	{
	//		/*
	//		1.�θ�ũ�⸦ ��� ��ӹ޾Ƽ� �Ұ��̰�? �츮�� ������ ������ ����ϱ� ������ false
	//		2.�ִ�ũ�⸦ ��ȯ?
	//		3.cluster �������� �ý������� ����Ȱ��� �ϳ��� �ý�������?
	//		4.���� ���� ��ֺ��͸� ��ȯ?
	//		5.��������Ʈ�Ӽ�
	//		6.ī�޶��� Ŭ������ ��ȯ?
	//		*/
	//		false, true, true, true, true, true
	//	};
	//	FbxSystemUnit::m.ConvertScene(scene, option);
	//}

	//�� ������ �ﰢ��ȭ �� �� �ִ� ��� ��带 �ﰢ��ȭ ��Ų��.
	converter = new FbxGeometryConverter(manager);

	/*
	FBX Nodes
	���� �ַ� �� ���� �� ����� ��ġ, ȸ�� �� ũ�⸦ �����ϴ� �� ���˴ϴ�. 
	���� FbxNode Ŭ������ ���� �߻�ȭ�˴ϴ�. FbxScene���� ����� �θ�-�ڽ� ������ �ֽ��ϴ�. 
	�� Ʈ���� ��Ʈ ���� ������ ���� �ߵ��� FbxScene::GetRootNode()�� ���� ���� �����մϴ�.
	*/
}

FbxLoader::~FbxLoader()
{
	//�޸� ���̱� ���ؼ� ����
	SAFE_DELETE(converter);
	ios->Destroy();
	importer->Destroy();
	scene->Destroy();
	manager->Destroy();
}

//FBX SDK�� ����ִ� �츮�� �ʿ��� material�� �����´�.(�ִ� xml������ �̿� ��� �������ؾ��� ��찡 ���Ƽ� ���)
void FbxLoader::ExportMaterial(wstring saveFolder, wstring fileName)
{
	ReadMaterial();

	//�׳� exportMaterial()��ȣ �ȿ� �ƹ��͵� �Ⱦ��� �ڽ��� ������ �̸��� ����.
	wstring tempFolder = saveFolder.size() < 1 ? this->saveFolder : saveFolder;
	wstring tempName = fileName.size() < 1 ? this->saveName : fileName;

	//tempName�ڿ� Ȯ���ڸ� �ڵ����� ������
	WriteMaterial(tempFolder, tempName + L".material");
}

//RootNode�� .mesh��� �������� �Լ�(�̰��� �������� text���Ϸ� ������ ������ ũ�Ⱑ ũ�� ������ ������ �������ϻ��)
void FbxLoader::ExportMesh(wstring saveFolder, wstring fileName)
{
	ReadBoneData(scene->GetRootNode(), -1, -1); //node���� Ʈ�����¿��� ����Լ��� ź��
	ReadSkinData();

	wstring tempFolder = saveFolder.size() < 1 ? this->saveFolder : saveFolder;
	wstring tempName = fileName.size() < 1 ? this->saveName : fileName;

	WriteMeshData(tempFolder, tempName + L".mesh");
}

void FbxLoader::GetClipList(vector<wstring>* list)
{
	list->clear();

	//fbx������ clip(�ִϸ��̼ǵ��۵�)�� animationStack�̶�� �θ�
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

//material�� scene�� �پ��ִ�.
void FbxLoader::ReadMaterial()
{
	//scene�� ������ �ִ� ��ü material����
	int count = scene->GetMaterialCount();

	for (int i = 0; i < count; i++)
	{
		//fbxMaterial�� ���ؼ� �츮�� ���ϴ� material ������ ���´�.
		FbxSurfaceMaterial* fbxMaterial = scene->GetMaterial(i);

		//������ ������ �츮�� ���ϴ� ���·� ����� �ְ� �װ� ��
		FbxMaterial* material = new FbxMaterial();
		//�̷��� �ϸ� �ش� scene�� ����Ǿ��ִ� material�� �̸��� �Ѿ�´�
		//�� �̸��� �߿� ���߿� �̸����� ��Ī
		material->Name = fbxMaterial->GetName();

		//Dynamic cast check����
		if (fbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId) == true)
		{
			//ĳ������ �����ϸ� ����
			//lambert -> diffuse(�ڱ���� ǥ��)
			//DiffuseFactor -> ���� �󸶳� ������?
			FbxSurfaceLambert* lambert = (FbxSurfaceLambert *)fbxMaterial;
			material->Diffuse = FbxUtility::ToColor(lambert->Diffuse, lambert->DiffuseFactor);
		}

		if (fbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId) == true)
		{
			//���ݻ簪
			FbxSurfacePhong* phong = (FbxSurfacePhong *)fbxMaterial;
			material->Specular = FbxUtility::ToColor(phong->Specular, phong->SpecularFactor);
			material->SpecularExp = (float)phong->Shininess;
		}

		//�ؽ��� �ҷ������� ��
		FbxProperty prop;

		//diffues, specular normal�� ���ϸ��� �ҷ����°���
		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		material->DiffuseFile = FbxUtility::GetTextureFile(prop);

		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
		material->SpecularFile = FbxUtility::GetTextureFile(prop);

		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
		material->NormalFile = FbxUtility::GetTextureFile(prop);

		//���� �����͵��� ����
		materials.push_back(material);
	}
}

void FbxLoader::WriteMaterial(wstring saveFolder, wstring fileName)
{
	//������ ������ �ڵ����� ������ ������ִ� �Լ�
	Path::CreateFolders(saveFolder);

	//xml���� �츮�� ������ ���� �ϱ����� text���Ϸ� �ҷ��� ����
	Xml::XMLDocument* document = new Xml::XMLDocument();
	Xml::XMLDeclaration *decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement* root = document->NewElement("Materials");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	//stl���� iterator�� ���� ��ü�� �ϳ��� ��ȸ���ִ� ��� foreach
	for (FbxMaterial* material : materials)
	{
		Xml::XMLElement* node = document->NewElement("Material");
		root->LinkEndChild(node);


		Xml::XMLElement* element = NULL;

		element = document->NewElement("Name");
		element->SetText(material->Name.c_str());
		node->LinkEndChild(element);

		//�츮�� �ʿ��� �͵��� xml�� ������� CopyTextureFile�Լ��� �̿��ؼ� _Model���Ϸ� �������ش�
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

	//�츮�� ���� xml���� ����
	string file = String::ToString(saveFolder + fileName);
	document->SaveFile(file.c_str());
}

void FbxLoader::ReadBoneData(FbxNode * node, int index, int parent)
{
	//���� �ڱⰡ ������ �Ӽ����� �ִ� �׷��� �츮�� �Ϻθ� ������
	FbxNodeAttribute* attribute = node->GetNodeAttribute();

	if (attribute != NULL)
	{
		//��� �Ӽ��߿� Ÿ���� �ִ�
		FbxNodeAttribute::EType nodeType = attribute->GetAttributeType();

		//�츮�� �� Ÿ�Ե�
		bool b = false;
		b |= (nodeType == FbxNodeAttribute::eSkeleton);  //��
		b |= (nodeType == FbxNodeAttribute::eMesh);		 //�츮�� �׸� �����͸� ������ �ִ°�
		b |= (nodeType == FbxNodeAttribute::eNull);      //�׳� ���Ƿ�
		b |= (nodeType == FbxNodeAttribute::eMarker);	 //�Ӹ����� ���̵� ������Ҷ� �Ӹ����� ��Ŀ��� ���� �־��

		if (b == true)
		{
			//�� ������ �����
			FbxBoneData* bone = new FbxBoneData();
			bone->Index = index;
			bone->Parent = parent;
			bone->Name = node->GetName();
			//FBX matrix�� DX matrix�� �ٲ���
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

	//��͸� ź�� �ڱ��� �ڽ��� ���� ��ŭ ź��.
	for (int i = 0; i < node->GetChildCount(); i++)
		ReadBoneData(node->GetChild(i), boneDatas.size(), index);
}

void FbxLoader::ReadMeshData(FbxNode * node, int parentBone)
{
	/*
	FbxMesh
	�޽��� �ٰ������� ������� ������Ʈ��.
	�޽��� ���� ����(vertices)���� �Ҹ��� ������(control point)�� ����Ʈ�� ���ǵ˴ϴ�.
	������(control point)�� XYZ��ǥ�̸� �������� �����ϴ�.
	FbxNode::GetMesh() ��� �Լ��� ȣ���� FbxMesh�� ĳ���õ� ��� �Ӽ��� �����͸� ������ �� �ֽ��ϴ�.
	*/
	//ControlPoint -> �ش������� ��ȣ�� ��ġ�� ������ �ִ°�
	//cp���� �߰��� vertexWeight, vertexIndices�� �����µ� �̰���
	//vertexIndices->������ ������ ����ġ�� ��ȣ, vertexWeight->��ȣ�� ��ġ�� �����ٰ� �󸶸�ŭ �̵�
	FbxMesh* mesh = node->GetMesh();

	//deformer->cluster(����)�� �Ѱ��ϴ� �� ex)���̸� ���������� �ִµ� �׳Ἦ���� �Ѱ�
	int deformerCount = mesh->GetDeformerCount();
	
	//mesh�� ������ ����
	//������ ��� ��带 �ﰢ��ȭ�ؼ� �޽��� polygon�� �ﰢ������ ����
	vector<FbxVertex *> vertices;
	for (int p = 0; p < mesh->GetPolygonCount(); p++) //�ﰢ���� ����
	{
		int vertexInPolygon = mesh->GetPolygonSize(p); //�ش� �����￡ �ִ� ������ ������ �Ѿ
		assert(vertexInPolygon == 3);

		for (int vi = 0; vi < 3; vi++) //�ﰢ���� 3���� �������� ����
		{
			//GetCoorSystem->��ǥ�ý����� �����´�(�޼�? ������?)
			//�޼���ǥ��� �ð�������� ȸ��
			int pvi = FbxUtility::bRightHand ? 2 - vi : vi;
			int cpIndex = mesh->GetPolygonVertex(p, pvi); //������ �ε����� ������

			FbxVertex* vertex = new FbxVertex();
			vertex->ControlPoint = cpIndex;

			FbxVector4 position = mesh->GetControlPointAt(cpIndex); //���� ������ ���� ��ġ
			vertex->Vertex.Position = FbxUtility::ToPosition(position);

			//�븻ȭ ���ֱ�
			FbxVector4 normal;
			mesh->GetPolygonVertexNormal(p, pvi, normal);
			vertex->Vertex.Normal = FbxUtility::ToNormal(normal);
			

			vertex->MaterialName = FbxUtility::GetMaterialName(mesh, p, cpIndex);

			int uvIndex = mesh->GetTextureUVIndex(p, pvi); //uv���� �ε���
			vertex->Vertex.Uv = FbxUtility::GetUv(mesh, cpIndex, uvIndex);

			vertices.push_back(vertex);
		}//for(vi)
	}//for(p)

	//������ ���� �޽��� ������ meshData�� �־��ִ� ��
	FbxMeshData* meshData = new FbxMeshData();
	meshData->Name = node->GetName();
	meshData->ParentBone = parentBone;
	meshData->Vertices = vertices;
	meshData->Mesh = mesh;
	meshDatas.push_back(meshData);
}

//�ٷ� ���� ����Ǿ��ִ� �޽��� ������� ��Ų�� ������(���� �Լ��� ���� �߿�)
void FbxLoader::ReadSkinData()
{
	for (FbxMeshData* meshData : meshDatas)
	{
		FbxMesh* mesh = meshData->Mesh;

		//������ �Ѱ��ϴ¾�(�Ǻζ�� ����)
		int deformerCount = mesh->GetDeformerCount();

		vector<FbxBoneWeights> boneWeights;
		boneWeights.assign(mesh->GetControlPointsCount(), FbxBoneWeights());

		for (int i = 0; i < deformerCount; i++)
		{
			FbxDeformer* deformer = mesh->GetDeformer(i, FbxDeformer::eSkin);

			//reinterpret_cast->������ ������ Ÿ�Գ��� ��ȯ�� ����ϴ� ĳ��Ʈ ������, �������� �����ͷ� �ٲ� �����ִ�
			//��Ӱ��谡 �ƴ϶�� �����ع�����
			FbxSkin* skin = reinterpret_cast<FbxSkin *>(deformer);
			if (skin == NULL) continue;

			for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); clusterIndex++)
			{
				//�ش� ���� ��������
				FbxCluster* cluster = skin->GetCluster(clusterIndex);
				assert(cluster->GetLinkMode() == FbxCluster::eNormalize);

				//�����ϰ� ����Ǿ��ִ°��� ���̱⶧����
				//GetLink�� �������ִ� ���� fbxBone �׷��� �� ���� �̸��� �����´�
				string linkName = cluster->GetLink()->GetName();

				UINT boneIndex = -1;
				for (UINT i = 0; i < boneDatas.size(); i++)
				{
					if (boneDatas[i]->Name == linkName)
					{
						boneIndex = i;

						break;        //���� ã�����ϴ� ���� ã����
					}
				}

				/*
				������ ���� �ȿ����̴� �𵨵� �ֱ⶧���� ������ �̿��ؼ�
				������ �츮�� ������ ����Ʈ�� �ҷ��� ���߿���
				cluster�� ����Ǿ��ִ� �������� ã�Ƽ� �̸��� ã�� �� ���� ��ġ�� ���
				*/
				FbxAMatrix transform;     //�ڱ� �ڽſ� ���� local
				FbxAMatrix linkTransform; //�θ� ���� world

				cluster->GetTransformMatrix(transform);
				cluster->GetTransformLinkMatrix(linkTransform);
				//������ ���ؼ� �ش� ���� ��Ʈ���� �ɾ��ִ°�
				boneDatas[boneIndex]->LocalTransform = FbxUtility::ToMatrix(transform);
				boneDatas[boneIndex]->GlobalTransform = FbxUtility::ToMatrix(linkTransform);

				for (int indexCount = 0; indexCount < cluster->GetControlPointIndicesCount(); indexCount++)
				{
					/*
					������ �������� ������ ������ų� �÷��� �Ǻθ� �Ų���ϱ� ������ �����̴� ����
					vertexIndices, vertexWeights�� ������ ����ִ�
					*/
					int temp = cluster->GetControlPointIndices()[indexCount];
					//������ ����ġ
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
			boneWeights[cpIndex].GetBlendWeights(weights); //����ġ�� ��������
			vertex->Vertex.BlendIndices = weights.Indices; //�ش� ������ ����ġ�� indices �־��ֱ�
			vertex->Vertex.BlendWeights = weights.Weights; //�ش� ������ ����ġ �־��ֱ�
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

	w->UInt(boneDatas.size());  //���� ������ ��� ������ ���� �������� ����� ���ش� �׷��� ���� ������ŭ �д´�
	for (FbxBoneData* bone : boneDatas)
	{
		//fbxBoneData�� �ִ� ������ ������� ���ش�
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

		//meshPartdata ����
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

	//�ݵ�� �ݾ������ �ٸ� ������ ����
	w->Close();
	SAFE_DELETE(w);
}

//�츮�� ���Ͽ� _Model������ ������µ� model���Ͼȿ� textureFile�� �������ַ��� ����
void FbxLoader::CopyTextureFile(OUT string & textureFile, wstring & saveFolder)
{
	//textureFile�� diffusefile�� ��ΰ� ���´�
	if (textureFile.length() < 1)
		return;

	
	wstring file = String::ToWString(textureFile);
	wstring fileName = Path::GetFileName(file);  //��ü��ο��� ���ϸ� �������� �Լ�

	//���� diffuseFile�� �������ִ°�
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
	
	//���������� ���� ���Ÿ�� ����
	ReadKeyframeData(scene->GetRootNode(), start, stop);
}

/*
�� ���� �ش��ϴ� �ð��� ���� srt Ʈ�������� �����ͼ� ������ �Ѵ�.

������ ���� 1�ʿ� ���� ��򰡷� �̵������� �� �ʿ� ���� �̵��� �����ϴµ�
�츮�� �� �����Ӻ��� srt�� �� �����Ѵ�
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
	w->String(clipData->Name);     //Ŭ���̸�
	w->Float(clipData->Duration);  //Ŭ����ü�ð�
	w->Float(clipData->FrameRate); //Ŭ�������Ӻ���
	w->UInt(clipData->FrameCount); //Ŭ��������ī��Ʈ
	
	w->UInt(clipData->Keyframes.size());
	for (UINT i = 0; i < clipData->Keyframes.size(); i++)
	{
		FbxKeyframe* frame = clipData->Keyframes[i];
		w->String(frame->BoneName); //�����Ӵ� ���� �̸��ְ�

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
