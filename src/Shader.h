#pragma once

#include <glad/glad.h>

#include <string>
#include <map>

class Shader
{
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    void Bind() const;
    inline unsigned int GetProgram() const { return m_RendererId; }
    void UploadUniform1d(const char* name, double v1);
    void UploadUniform2d(const char* name, double v1, double v2);
    void UploadUniform3d(const char* name, double v1, double v2, double v3);
    void UploadUniformMat4(const char* name, const float* value);
    void UploadUniform2f(const char* name, float v1, float v2);
    void UploadUniform3f(const char* name, float v1, float v2, float v3);
    void UploadUniform4f(const char* name, float v1, float v2, float v3, float v4);
    void UploadUniform1i(const char* name, int i);
    void UploadUniform1ui(const char* name, unsigned int i);
    void UploadUniform2i(const char* name, int v1, int v2);
    void UploadUniform1f(const char* name, float v1);
    ~Shader();
private:
    int GetUniLocation(const char* name);
private:
    unsigned int m_RendererId;
    std::map<std::string, int> m_UniformLocations;
};
