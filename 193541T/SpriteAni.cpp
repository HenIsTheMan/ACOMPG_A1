#include "SpriteAni.h"

extern float dt;

SpriteAnimation::SpriteAnimation(int row, int col): row(row), col(col), currentTime(0), currentFrame(0), playCount(0), currentAnimation(""){}

SpriteAnimation::~SpriteAnimation(){
	for(auto iter = animationList.begin(); iter != animationList.end(); ++iter){
		if(iter->second){
			delete iter->second;
		}
	}
}

void SpriteAnimation::Update(){
	if(animationList[currentAnimation]->animActive){ //Check if the current animation is active
		currentTime += dt;
		int numFrame = (int)animationList[currentAnimation]->frames.size();
		float frameTime = animationList[currentAnimation]->animTime / numFrame;

		currentFrame = animationList[currentAnimation]->frames[std::min((int)animationList[currentAnimation]->frames.size() - 1, int(currentTime / frameTime))]; //Set curr frame based on curr time
		if(currentTime >= animationList[currentAnimation]->animTime){ //If curr time >= total animated time...
			if(playCount < animationList[currentAnimation]->repeatCount){
				///Increase count and repeat
				++playCount;
				currentTime = 0;
				currentFrame = animationList[currentAnimation]->frames[0];
			} else{ //If repeat count is 0 || play count == repeat count...
				animationList[currentAnimation]->animActive = false;
				animationList[currentAnimation]->ended = true;
			}
			if(animationList[currentAnimation]->repeatCount == -1){ //If ani is infinite...
				currentTime = 0.f;
				currentFrame = animationList[currentAnimation]->frames[0];
				animationList[currentAnimation]->animActive = true;
				animationList[currentAnimation]->ended = false;
			}
		}
	}
}

void SpriteAnimation::AddAnimation(std::string anim_name, int start, int end){
	Animation* anim = new Animation();
	for(int i = start; i < end; ++i){ //Ad in all the frames in the range
		anim->AddFrame(i);
	}
	animationList[anim_name] = anim; //Link anim to animation list
	if(currentAnimation == ""){ //Set the current animation if it does not exist
		currentAnimation = anim_name;
	}
	animationList[anim_name]->animActive = false;
}

void SpriteAnimation::AddSequeneAnimation(std::string anim_name, int count...){
	Animation* anim = new Animation();
	va_list args;
	va_start(args, count);
	for(int i = 0; i < count; ++i){
		int value = va_arg(args, int);
		anim->AddFrame(value);
	}
	va_end(args);
	animationList[anim_name] = anim; //Link the animation to the animation list
	if(currentAnimation == ""){ //Set the current animation if it does not exisit
		currentAnimation = anim_name;
	}
	animationList[anim_name]->animActive = false;
}

void SpriteAnimation::PlayAnimation(std::string anim_name, int repeat, float time){
	if(animationList[anim_name] != nullptr){ //Check if the anim name exist
		currentAnimation = anim_name;
		animationList[anim_name]->Set(repeat, time, true);
	}
}

void SpriteAnimation::Resume(){
	animationList[currentAnimation]->animActive = true;
}

void SpriteAnimation::Pause(){
	animationList[currentAnimation]->animActive = false;
}

void SpriteAnimation::Reset(){
	currentFrame = animationList[currentAnimation]->frames[0];
	playCount = 0;
}

SpriteAnimation* const SpriteAnimation::CreateSpriteAni(const unsigned& numRow, const unsigned& numCol){
	SpriteAnimation* mesh = new SpriteAnimation(numRow, numCol);
	mesh->indices = new std::vector<uint>;
	short myArr[6]{0, 1, 2, 0, 2, 3};

	float width = 1.f / numCol, height = 1.f / numRow;
	int offset = 0;
	for(unsigned i = 0; i < numRow; ++i){
		for(unsigned j = 0; j < numCol; ++j){
			float U = j * width;
			float V = 1.f - height - i * height;
			mesh->vertices.emplace_back(Vertex(glm::vec3(-.5f, -.5f, 0.f), glm::vec4(1.f), glm::vec2(U, V), glm::vec3(0.f, 0.f, 1.f)));
			mesh->vertices.emplace_back(Vertex(glm::vec3(.5f, -.5f, 0.f), glm::vec4(1.f), glm::vec2(U + width, V), glm::vec3(0.f, 0.f, 1.f)));
			mesh->vertices.emplace_back(Vertex(glm::vec3(.5f, .5f, 0.f), glm::vec4(1.f), glm::vec2(U + width, V + height), glm::vec3(0.f, 0.f, 1.f)));
			mesh->vertices.emplace_back(Vertex(glm::vec3(-.5f, .5f, 0.f), glm::vec4(1.f), glm::vec2(U, V + height), glm::vec3(0.f, 0.f, 1.f)));

			for(short k = 0; k < 6; ++k){
				(*(mesh->indices)).emplace_back(offset + myArr[k]);
			}
			offset += 4;
		}
	}
	return mesh;
}

void SpriteAnimation::DrawSpriteAni(const int& primitive, const uint& instanceAmt){
	try{
		if(instanceAmt == 0){
			throw std::runtime_error("Invalid amt of instances!\n");
		}
	} catch(std::runtime_error err){
		printf(err.what());
	}
	if(!VAO && !VBO && !EBO){ //Init on 1st draw/...
		Init(instanceAmt);
	}
	for(uint i = 0; i < textures.size(); ++i){
		if(textures[i].GetActiveOnMesh()){
			ShaderProg::UseTex(GL_TEXTURE_2D, textures[i], ("material." + textures[i].GetType() + "Map").c_str());
		}
	}
	glBindVertexArray(VAO); {
		if(indices){
			instanceAmt == 1 ? glDrawElements(primitive, 6, GL_UNSIGNED_INT, (void*)(dynamic_cast<SpriteAnimation*>(this)->currentFrame * 6 * sizeof(GLuint))) :
				glDrawElementsInstanced(primitive, 6, GL_UNSIGNED_INT, (void*)(dynamic_cast<SpriteAnimation*>(this)->currentFrame * 6 * sizeof(GLuint)), instanceAmt);
		}
	} glBindVertexArray(0); //Break the existing VAO binding
	for(uint i = 0; i < textures.size(); ++i){
		if(textures[i].GetActiveOnMesh()){
			ShaderProg::StopUsingTex(GL_TEXTURE_2D, textures[i]);
			textures[i].SetActiveOnMesh(0);
		}
	}
}