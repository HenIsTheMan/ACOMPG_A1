#include "ShaderProg.h"

ShaderProg* ShaderProg::currShaderProg = nullptr;
uint ShaderProg::texRefIDs[32] = {0, };
std::unordered_map<cstr, uint> ShaderProg::shaderCache;

ShaderProg::ShaderProg(cstr const& vsPath, cstr const& fsPath, cstr const& gsPath) noexcept: refID(0){
	shaderFilePaths[0] = vsPath;
	shaderFilePaths[1] = fsPath;
	shaderFilePaths[2] = gsPath;
}

ShaderProg::~ShaderProg() noexcept{
	uniLocationCache.clear(); //For gd practice
	glDeleteProgram(refID);
}

void ShaderProg::ClearShaderCache() noexcept{
	for(const auto& shader: shaderCache){
		glDeleteShader(shader.second);
	}
	shaderCache.clear(); //For gd practice
}

int ShaderProg::GetUniLocation(cstr const& uniName) noexcept{
	if(!currShaderProg->uniLocationCache.count(str(uniName))){ //If not cached...
		currShaderProg->uniLocationCache[str(uniName)] = glGetUniformLocation(currShaderProg->refID, uniName); //Query location of uni
	}
	return currShaderProg->uniLocationCache[str(uniName)];
}

void ShaderProg::Init() noexcept{
	refID = glCreateProgram();
	for(short i = 0; i < sizeof(shaderFilePaths) / sizeof(shaderFilePaths[0]) - (shaderFilePaths[2] == ""); ++i){
		if(shaderCache.count(shaderFilePaths[i])){
			printf("Reusing \"%s\"...\n", shaderFilePaths[i]);
			glAttachShader(refID, shaderCache[shaderFilePaths[i]]);
		} else{
			uint shaderRefID = glCreateShader(i < 2 ? (~i & 1 ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER) : GL_GEOMETRY_SHADER);
			ParseShader(shaderFilePaths[i], shaderRefID);
			glAttachShader(refID, shaderRefID);
			shaderCache[shaderFilePaths[i]] = shaderRefID;
		}
	}
	Link();
}

void ShaderProg::ParseShader(cstr const& fPath, const uint& shaderID) const noexcept{
	int infoLogLength;
	str srcCodeStr, line;
	std::ifstream stream(fPath);

	if(!stream.is_open()){
		printf("Failed to open and read \"%s\"\n", fPath);
		return;
	}
	while(getline(stream, line)){
		srcCodeStr += "\n" + line;
	}
	stream.close();

	printf("Compiling \"%s\"...\n", fPath);
	cstr srcCodeCStr = srcCodeStr.c_str();
	glShaderSource(shaderID, 1, &srcCodeCStr, 0);
	glCompileShader(shaderID);
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if(infoLogLength){
		char* errorMsg = (char*)_malloca(infoLogLength * sizeof(char)); //Allocate memory on the stack dynamically
		glGetShaderInfoLog(shaderID, infoLogLength, &infoLogLength, errorMsg);
		printf("Failed to compile \"%s\"!\n%s\n", fPath, errorMsg);
	}
}

void ShaderProg::Link() const noexcept{
	int infoLogLength;
	printf("Linking programme...\n\n");
	glLinkProgram(refID); //Vars in diff shaders are linked here too

	glGetProgramiv(refID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if(infoLogLength){
		char* errorMsg = (char*)_malloca(infoLogLength * sizeof(char)); //Allocate memory on the stack dynamically
		glGetProgramInfoLog(refID, infoLogLength, &infoLogLength, errorMsg);
		printf("%s\n", errorMsg);
	}
}

void ShaderProg::LinkUniBlock(cstr const& name, const uint& bindingPtIndex, const bool&& warn) noexcept{ //Link uni interface block of shader to a binding pt
	uint uniBlockLocation = glGetUniformBlockIndex(currShaderProg->refID, name); //Indices of active uni blocks in a shader prog are assigned in consecutive/... order starting from 0
	if(uniBlockLocation != GL_INVALID_INDEX){
		glUniformBlockBinding(currShaderProg->refID, uniBlockLocation, bindingPtIndex); //Uni interface blocks, with the same def, linked to the same binding point share the same uni data from the same UBO
	} else if(warn){
		printf("%s: Failed to link '%s' to binding pt of index %u\n", std::to_string(currShaderProg->refID).c_str(), name, bindingPtIndex);
	}
}

void ShaderProg::UseTex(const int& texTarget, const Tex& tex, const cstr& samplerName) noexcept{
	try{
		const uint& texRefID = tex.GetRefID();
		for(short i = 0; i < 32; ++i){
			if(!texRefIDs[i]){
				glActiveTexture(GL_TEXTURE0 + i);
				ShaderProg::SetUni1i(samplerName, i); //Make sure each shader sampler uni corresponds to the correct tex unit
				glBindTexture(texTarget, texRefID);
				texRefIDs[i] = texRefID;
				return;
			}
		}
		throw std::runtime_error("Failed to use tex (RefID: " + std::to_string(texRefID) + ").\n");
	} catch(std::runtime_error err){
		printf(err.what());
	}
}

void ShaderProg::StopUsingTex(const int& texTarget, const Tex& tex) noexcept{
	try{
		const uint& texRefID = tex.GetRefID();
		for(short i = 0; i < 32; ++i){
			if(texRefIDs[i] == texRefID){
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(texTarget, 0);
				texRefIDs[i] = 0;
				return;
			}
		}
		throw std::runtime_error("Cannot find tex (RefID: " + std::to_string(texRefID) + ") to stop using.\n");
	} catch(std::runtime_error err){
		printf(err.what());
	}
}

void ShaderProg::StopUsingAllTexs() noexcept{
	for(short i = 0; i < 32; ++i){
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
		texRefIDs[i] = 0;
	}
}

void ShaderProg::Use() noexcept{
	if(!refID){
		Init();
	}
	if(currShaderProg != this){
		glUseProgram(refID);
		currShaderProg = this;
	}
}

void ShaderProg::SetUni1f(cstr const& uniName, const float& value, const bool&& warn) noexcept{
	int uniLocation = GetUniLocation(uniName);
	if(uniLocation != -1){
		glUniform1f(uniLocation, value); //Sets uniform on the currently active shader prog
	} else if(warn){
		printf("%s: Failed to find '%s'\n", std::to_string(currShaderProg->refID).c_str(), uniName);
	}
}

void ShaderProg::SetUni2f(cstr const& uniName, const float& value1, const float& value2, const bool&& warn) noexcept{
	int uniLocation = GetUniLocation(uniName);
	if(uniLocation != -1){
		glUniform2f(uniLocation, value1, value2); //Sets uniform on the currently active shader prog
	} else if(warn){
		printf("%s: Failed to find '%s'\n", std::to_string(currShaderProg->refID).c_str(), uniName);
	}
}

void ShaderProg::SetUni3f(cstr const& uniName, const float& value1, const float& value2, const float& value3, const bool&& warn) noexcept{
	int uniLocation = GetUniLocation(uniName);
	if(uniLocation != -1){
		glUniform3f(uniLocation, value1, value2, value3); //Sets uniform on the currently active shader prog
	} else if(warn){
		printf("%s: Failed to find '%s'\n", std::to_string(currShaderProg->refID).c_str(), uniName);
	}
}

void ShaderProg::SetUni3f(cstr const& uniName, const glm::vec3& vec, const bool&& warn) noexcept{
	int uniLocation = GetUniLocation(uniName);
	if(uniLocation != -1){
		glUniform3f(uniLocation, vec.x, vec.y, vec.z); //Sets uniform on the currently active shader prog
	} else if(warn){
		printf("%s: Failed to find '%s'\n", std::to_string(currShaderProg->refID).c_str(), uniName);
	}
}

void ShaderProg::SetUni4f(cstr const& uniName, const float values[4], const bool&& warn) noexcept{
	int uniLocation = GetUniLocation(uniName);
	if(uniLocation != -1){
		glUniform4f(uniLocation, values[0], values[1], values[2], values[3]); //Sets uniform on the currently active shader prog
	} else if(warn){
		printf("%s: Failed to find '%s'\n", std::to_string(currShaderProg->refID).c_str(), uniName);
	}
}

void ShaderProg::SetUni1i(cstr const& uniName, const int& value, const bool&& warn) noexcept{
	int uniLocation = GetUniLocation(uniName);
	if(uniLocation != -1){
		glUniform1i(uniLocation, value); //Sets uniform on the currently active shader prog
	} else if(warn){
		printf("%s: Failed to find '%s'\n", std::to_string(currShaderProg->refID).c_str(), uniName);
	}
}

void ShaderProg::SetUniMtx4fv(cstr const& uniName, const float* const& address, const bool&& warn) noexcept{
	int uniLocation = GetUniLocation(uniName);
	if(uniLocation != -1){
		glUniformMatrix4fv(uniLocation, 1, GL_FALSE, address); //Sets uniform on the currently active shader prog
	} else if(warn){
		printf("%s: Failed to find '%s'\n", std::to_string(currShaderProg->refID).c_str(), uniName);
	}
}