#pragma once

#include <unordered_map>
#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>


class ShaderProgram
{
	std::unordered_map<GLenum, std::string> paths;
	std::unordered_map<GLenum, GLuint> ids;

	GLuint id;

	GLuint findUniformLocation(const std::string& name);

	std::unordered_map<std::string, GLuint> uniform_locations;
public:
	ShaderProgram();
	~ShaderProgram();

	void add(GLenum type, const std::string& path);
	void compile();


	void use();

	void uniformv(const std::string& name, GLuint count, const glm::mat4* matrices);
	void uniform(const std::string& name, const glm::mat4& matrix);

	void uniformv(const std::string& name, GLuint count, const GLfloat* values);
	void uniform(const std::string& name, const GLfloat value);

	void uniformv(const std::string& name, GLuint count, const glm::vec2* vectors);
	void uniform(const std::string& name, const glm::vec2& vector);

	void uniformv(const std::string& name, GLuint count, const glm::vec3* vectors);
	void uniform(const std::string& name, const glm::vec3& vector);

	void uniformv(const std::string& name, GLuint count, const glm::vec4* vectors);
	void uniform(const std::string& name, const glm::vec4& vector);

	void uniformv(const std::string& name, GLuint count, const GLint* values);
	void uniform(const std::string& name, const GLint value);

	GLuint getId()
	{
		return this->id;
	}
};