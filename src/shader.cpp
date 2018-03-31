#include "shader.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>


void checkLinkError(GLuint id, const std::string& paths)
{
	GLint success = 0;
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint log_size = 0;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_size);
		if (log_size > 0)
		{
			std::vector<GLchar> error(log_size);
			glGetProgramInfoLog(id, log_size, &log_size, &error[0]);
			std::string errorstr{ &error[0] };

			std::cout << "Error in:\n" << paths << "\n" << errorstr << "\n";

			glDeleteProgram(id);
		}
		//system("pause");
	}
}


GLuint compileShader(GLenum type, const std::string& name)
{
	GLuint shader = glCreateShader(type);

	//std::string path = "assets/shaders/" + name;
	std::string path = name;
	std::ifstream shaderFile(path);
	std::string shaderText((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
	if (!shaderFile.is_open())
	{
		std::cout << "[ERROR] Could not open file: " << path << "\n";
		//system("pause");
		//exit(1);
	}

	shaderFile.close();
	const char* shaderTextPtr = shaderText.c_str();

	glShaderSource(shader, 1, &shaderTextPtr, nullptr);
	glCompileShader(shader);

	// Check for compile error
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint log_size = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
		std::vector<GLchar> error(log_size);
		glGetShaderInfoLog(shader, log_size, &log_size, &error[0]);
		std::string errorstr{ &error[0] };

		std::cout << "Error in '" << name << "':\n" << errorstr << "\n";

		glDeleteShader(shader);
		//system("pause");
	}

	return shader;
}

ShaderProgram::ShaderProgram()
{
	id = 0;
}

ShaderProgram::~ShaderProgram()
{
	for (auto& i : ids)
	{
		glDeleteShader(i.second);
	}
	glDeleteProgram(id);
}
void ShaderProgram::add(GLenum type, const std::string& path)
{
	glDeleteShader(ids[type]);
	paths[type] = path;
	ids[type] = 0;
}

void ShaderProgram::compile()
{
	//std::cout << "[DEBUG] Compiling Shaders\n";

	std::string paths_str;

	glDeleteProgram(id);
	for (auto& path : paths)
	{
		glDeleteShader(ids[path.first]);
		ids[path.first] = compileShader(path.first, path.second);

		paths_str += path.second + "\n";
	}

	id = glCreateProgram();
	for (auto& i : ids)
	{
		glAttachShader(id, i.second);
	}
	glLinkProgram(id);

	checkLinkError(id, paths_str);

}

void ShaderProgram::use()
{
	glUseProgram(id);
}


GLuint ShaderProgram::findUniformLocation(const std::string & name)
{
	auto it = uniform_locations.find(name);
	GLuint uniform_location;
	if (it == uniform_locations.end())
	{
		uniform_location = glGetUniformLocation(id, name.c_str());
		if (uniform_location == -1)
		{
			std::cout << "ERROR: could not find '" << name << "' in shader\n";
		}
		uniform_locations[name] = uniform_location;
	}
	else
		uniform_location = it->second;
	return uniform_location;
}


void ShaderProgram::uniformv(const std::string & name, GLuint count, const glm::mat4* matrices)
{
	glUniformMatrix4fv(findUniformLocation(name), count, GL_FALSE, (GLfloat*)matrices);
}
void ShaderProgram::uniform(const std::string & name, const glm::mat4& matrix)
{
	uniformv(name, 1, &matrix);
}


void ShaderProgram::uniformv(const std::string & name, GLuint count, const GLfloat* values)
{
	glUniform1fv(findUniformLocation(name), count, (GLfloat*)values);
}
void ShaderProgram::uniform(const std::string & name, GLfloat value)
{
	uniformv(name, 1, &value);
}


void ShaderProgram::uniformv(const std::string & name, GLuint count, const glm::vec2* vectors)
{
	glUniform2fv(findUniformLocation(name), count, (GLfloat*)vectors);
}
void ShaderProgram::uniform(const std::string & name, const glm::vec2& vector)
{
	uniformv(name, 1, &vector);
}


void ShaderProgram::uniformv(const std::string & name, GLuint count, const glm::vec3* vectors)
{
	glUniform3fv(findUniformLocation(name), count, (GLfloat*)vectors);
}
void ShaderProgram::uniform(const std::string & name, const glm::vec3& vector)
{
	uniformv(name, 1, &vector);
}

void ShaderProgram::uniformv(const std::string & name, GLuint count, const glm::vec4* vectors)
{
	glUniform4fv(findUniformLocation(name), count, (GLfloat*)vectors);
}
void ShaderProgram::uniform(const std::string & name, const glm::vec4& vector)
{
	uniformv(name, 1, &vector);
}




void ShaderProgram::uniformv(const std::string & name, GLuint count, const GLint* values)
{
	glUniform1iv(findUniformLocation(name), count, values);
}
void ShaderProgram::uniform(const std::string & name, GLint value)
{
	uniformv(name, 1, &value);
}
