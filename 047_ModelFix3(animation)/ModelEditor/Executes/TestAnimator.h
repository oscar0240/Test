#pragma once
#include "Systems/IExecute.h"

class TestAnimator : public IExecute
{
public:
	// IExecute��(��) ���� ��ӵ�
	void Initialize() override;
	void Ready() override;
	void Destroy() override;

	void Update() override;

	void PreRender() override;
	void Render() override;
	void PostRender() override {}

	void ResizeScreen() override {}

private:
	GameAnimator* gameModel;

	Material * terrainMaterial;
	class Terrain* terrain;

};