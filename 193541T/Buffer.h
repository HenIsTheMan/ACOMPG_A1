#pragma once
#include "Src.h"

class Tex final{
	bool activeOnMesh;
	uint refID;
	str type;
public:
	Tex();
	Tex(const int&, const int&, const int&, const int&, const int&, const int&, const int&);
	const bool& GetActiveOnMesh() const noexcept;
	const uint& GetRefID() const noexcept;
	const str& GetType() const noexcept;
	void Del() const noexcept;
	void SetActiveOnMesh(const bool&) noexcept;
	void Create(const int&, const int&, const int&, const int&, const int&, const int&, const int&, const str&, const std::vector<cstr>* const& = nullptr, const bool& = 1);
};

class Renderbuffer final{
	uint refID;
	void Create(const int&, const int&, const int&);
public:
	Renderbuffer(const int&, const int&, const int&);
	const uint& GetRefID() const noexcept;
	void Del() const noexcept;
};

class Framebuffer final{
	uint refID;
	Tex tex;
	Renderbuffer* RBO;
public:
	Framebuffer(const int&&, const int&&, const int&&, const int&&, const int&&, const int&&, const int&&);
	void Del() const noexcept;
	const uint& GetRefID() const noexcept;
	const Tex& GetTex() const noexcept;
	const Renderbuffer& GetRenderbuffer() const noexcept;
};

class UniBuffer final{
	uint currOffset;
	uint refID;
	void Init(const size_t&);
	template<typename T>
	void StoreData(const T&);
	void Bind(const uint&, const uint&, const size_t&);
public:
	template<typename T>
	UniBuffer(const T&, const uint&);
	~UniBuffer() noexcept;
};

template<typename T>
UniBuffer::UniBuffer(const T& data, const uint& bindingPtIndex): currOffset(0){
	Init(sizeof(T));
	StoreData(data);
	Bind(bindingPtIndex, 0, sizeof(T));
}

template<typename T>
void UniBuffer::StoreData(const T& data){ //Store global uni vars in a fixed piece of GPU mem
	glBindBuffer(GL_UNIFORM_BUFFER, refID); {
		try{
			int size;
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
			if(currOffset + sizeof(T) > size){
				throw std::runtime_error("");
			}
		} catch(const std::runtime_error&){
			std::cerr << "Failed to store " << data << " in uni buffer";
		}
		glBufferSubData(GL_UNIFORM_BUFFER, currOffset, sizeof(T), &data); //Can also add all data with 1 byte arr //0 is aligned byte offset of 1 of the uni vars declared in a uni block
	} glBindBuffer(GL_UNIFORM_BUFFER, 0);
	currOffset += sizeof(T);
}