#include "stdafx.h"
#include "TestAnimator.h"
#include "Fbx/FbxLoader.h"
#include "Environment/Terrain.h"

void TestAnimator::Initialize()
{
	terrainMaterial = new Material(Shaders + L"016_TerrainBrush.fx");

	FbxLoader* loader = NULL;
	vector<wstring> clipList;

	//loader = new FbxLoader
	//(
	//	Assets + L"Kachujin/Mesh.fbx",
	//	Models + L"Kachujin/", L"Kachujin"
	//);
	//loader->ExportMaterial();
	//loader->ExportMesh();
	//SAFE_DELETE(loader);

	//loader = new FbxLoader
	//(
	//	Assets + L"Kachujin/Idle.fbx",
	//	Models + L"Kachujin/", L"Idle"
	//);
	//loader->ExportAnimation(0);
	//SAFE_DELETE(loader);
}

void TestAnimator::Ready()
{
	gameModel = new GameAnimator
	(
		Shaders + L"046_Model.fx",
		Models + L"Kachujin/Kachujin.material",
		Models + L"Kachujin/Kachujin.mesh"
	);
	gameModel->AddClip(Models + L"Kachujin/Idle.animation");
	gameModel->Ready();
	gameModel->Scale(0.01f, 0.01f, 0.01f);

	terrainMaterial->SetDiffuseMap(Textures + L"Dirt.png");
	terrain = new Terrain(terrainMaterial, Textures + L"HeightMap256.png");
	
}

void TestAnimator::Destroy()
{
	SAFE_DELETE(gameModel);
	SAFE_DELETE(terrain);
}

void TestAnimator::Update()
{
	gameModel->Update();
	terrain->Update();
}

void TestAnimator::PreRender()
{

}

void TestAnimator::Render()
{
	gameModel->Render();
	terrain->Render();
}