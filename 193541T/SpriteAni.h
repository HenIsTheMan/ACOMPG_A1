#pragma once
#include "Src.h"
#include "Mesh.h"

class Animation final{
	friend class SpriteAnimation;
	Animation(): repeatCount(0), animTime(0.f), ended(false), animActive(false){}

	std::string animationName;
	std::vector<int> frames;
	int repeatCount;
	float animTime;
	bool ended;
	bool animActive;

	void Set(int repeat, float time, bool active){
		this->repeatCount = repeat;
		this->animTime = time;
		this->animActive = active;
	}
	void AddFrame(int i){
		frames.push_back(i);
	}
};

class SpriteAnimation final: public Mesh{
	int row;
	int col;
	float currentTime;
	int playCount;
	std::string currentAnimation;
	std::unordered_map<std::string, Animation*> animationList;
public:
	SpriteAnimation(int row, int col);
	~SpriteAnimation();
	void Update();
	int currentFrame;
	static SpriteAnimation* const CreateSpriteAni(const unsigned& numRow, const unsigned& numCol);
	void AddAnimation(std::string name, int start, int end);
	void AddSequeneAnimation(std::string name, int count ...);
	void PlayAnimation(std::string name, int repeat, float time);
	void Pause();
	void Resume();
	void Reset();
	void DrawSpriteAni(const int&, const uint&);
};