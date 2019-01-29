#pragma once

class GameAnimator : public GameModel
{
public:
	GameAnimator(wstring shaderFile, wstring matFile, wstring meshFile);
	~GameAnimator();

	void AddClip(wstring clipFile);

	void Update();
	void Render();


private:
	void UpdateWorld() override;
	void UpdateTransforms() override;

private:
	UINT currentClip;
	int currentKeyframe;
	int nextKeyframe;

	float frameTime;
	float frameFactor;  //보간할때 (증가감소)양

	vector<class ModelClip *> clips;
};