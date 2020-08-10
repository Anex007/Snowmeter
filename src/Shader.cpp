#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
    : m_RendererId(0)
{
    std::ifstream vShaderFile(vertexPath);
    std::ifstream fShaderFile(fragmentPath);
    std::string vertexSrcStr;
    std::string fragmentSrcStr;

    if (vShaderFile.fail() || fShaderFile.fail()) {
        std::cout << "Failed to open shaders" << std::endl;
        return;
    }

    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    vertexSrcStr = vShaderStream.str();
    fragmentSrcStr = fShaderStream.str();

    vShaderFile.close();
    fShaderFile.close();

    const char* vertexSrc = vertexSrcStr.c_str();
    const char* fragmentSrc = fragmentSrcStr.c_str();

    unsigned int vertexShader, fragmentShader;
    char infoLog[512];
    int success;

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "Error compiling vertex shader: \n\t" << infoLog << std::endl;
        return;
    }
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "Error compiling fragment shader: \n\t" << infoLog << std::endl;
        return;
    }

    m_RendererId = glCreateProgram();
    glAttachShader(m_RendererId, vertexShader);
    glAttachShader(m_RendererId, fragmentShader);
    glLinkProgram(m_RendererId);
    glGetProgramiv(m_RendererId, GL_LINK_STATUS, &success);

    glDetachShader(m_RendererId, vertexShader);
    glDetachShader(m_RendererId, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if (!success) {
        glGetProgramInfoLog(m_RendererId, 512, NULL, infoLog);
        std::cout << "Error Linking shaders: \n\t" << infoLog << std::endl;
        return;
    }
}

void Shader::Bind() const
{
    glUseProgram(m_RendererId);
}

int Shader::GetUniLocation(const char* name)
{
    if (!m_UniformLocations.count(name)) {
        int loc = glGetUniformLocation(m_RendererId, name);
        m_UniformLocations[name] = loc;
        return loc;
    } else {
        return m_UniformLocations[name];
    }
}

void Shader::UploadUniform1d(const char* name, double v1)
{
    glUniform1d(GetUniLocation(name), v1);
}

void Shader::UploadUniform2d(const char* name, double v1, double v2)
{
    glUniform2d(GetUniLocation(name), v1, v2);
}

void Shader::UploadUniform3d(const char* name, double v1, double v2, double v3)
{
    glUniform3d(GetUniLocation(name), v1, v2, v3);
}

void Shader::UploadUniform2f(const char* name, float v1, float v2)
{
    glUniform2f(GetUniLocation(name), v1, v2);
}

void Shader::UploadUniformMat4(const char* name, const float* value)
{
    glUniformMatrix4fv(GetUniLocation(name), 1, GL_FALSE, value);
}

void Shader::UploadUniform3f(const char* name, float v1, float v2, float v3)
{
    glUniform3f(GetUniLocation(name), v1, v2, v3);
}

void Shader::UploadUniform4f(const char* name, float v1, float v2, float v3, float v4)
{
    glUniform4f(GetUniLocation(name), v1, v2, v3, v4);
}

void Shader::UploadUniform1i(const char* name, int i)
{
    glUniform1i(GetUniLocation(name), i);
}

void Shader::UploadUniform1ui(const char* name, unsigned int i)
{
    glUniform1ui(GetUniLocation(name), i);
}

void Shader::UploadUniform1f(const char* name, float v1)
{
    glUniform1f(GetUniLocation(name), v1);
}

void Shader::UploadUniform2i(const char* name, int v1, int v2)
{
    glUniform2i(GetUniLocation(name), v1, v2);
}

Shader::~Shader()
{
    if (m_RendererId)
        glDeleteProgram(m_RendererId);
}
