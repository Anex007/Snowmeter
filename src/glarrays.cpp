#include "glarrays.h"

#include <vector>

static GLenum ToOpenGLDataType(ShaderDataType type)
{
    switch (type)
    {
    case ShaderDataType::Float:     return GL_FLOAT;
    case ShaderDataType::Float2:    return GL_FLOAT;
    case ShaderDataType::Float3:    return GL_FLOAT;
    case ShaderDataType::Float4:    return GL_FLOAT;
    case ShaderDataType::Mat3:      return GL_FLOAT;
    case ShaderDataType::Mat4:      return GL_FLOAT;
    case ShaderDataType::Int:       return GL_INT;
    case ShaderDataType::Int2:      return GL_INT;
    case ShaderDataType::Int3:      return GL_INT;
    case ShaderDataType::Int4:      return GL_INT;
    case ShaderDataType::Bool:      return GL_BOOL;
    }
    return 0;
}

static uint32_t ShaderDataTypeSize(ShaderDataType type)
{
    switch (type)
    {
    case ShaderDataType::Float:     return 4;
    case ShaderDataType::Float2:    return 4*2;
    case ShaderDataType::Float3:    return 4*3;
    case ShaderDataType::Float4:    return 4*4;
    case ShaderDataType::Mat3:      return 4*3*3;
    case ShaderDataType::Mat4:      return 4*4*4;
    case ShaderDataType::Int:       return 4;
    case ShaderDataType::Int2:      return 4*2;
    case ShaderDataType::Int3:      return 4*3;
    case ShaderDataType::Int4:      return 4*4;
    case ShaderDataType::Bool:      return 1;
    }
    return 0;
}

static std::pair<std::vector<uint32_t>, uint32_t> LayoutCalculations(const std::initializer_list<BufferElement>& elements)
{
    std::vector<uint32_t> offsets;
    uint32_t offset = 0;
    uint32_t stride = 0;
    offsets.reserve(elements.size());
    for (const auto& element: elements) {
        offsets.push_back(offset);
        const uint32_t size = ShaderDataTypeSize(element.Type);
        offset += size;
        stride += size;
    }
    return std::make_pair(offsets, stride);
}

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_Id);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_Id);
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_Id);
}

void VertexArray::UnBind() const
{
    glBindVertexArray(0);
}

void VertexArray::SetLayout(const std::initializer_list<BufferElement>& elements) const
{
    uint32_t i = 0;
    auto [offsets, stride] = LayoutCalculations(elements);
    glBindVertexArray(m_Id);
    for (const auto& element: elements) {
        glVertexAttribPointer(i, element.GetCount(), ToOpenGLDataType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE,
        stride, (const void*)offsets[i]);
        glEnableVertexAttribArray(i);
        i++;
    }
}

VertexBuffer::VertexBuffer()
{
	glGenBuffers(1, &m_Id);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_Id);
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Id);
}

void VertexBuffer::UnBind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::UploadData(const void* data, size_t size, unsigned int usage) const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Id);
	glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void VertexBuffer::UpdateData(long offset, const void* data, size_t size) const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Id);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

IndexBuffer::IndexBuffer(uint32_t* indicies, size_t nmemb, unsigned int usage)
{
    glGenBuffers(1, &m_Id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, nmemb * sizeof(uint32_t), indicies, usage);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &m_Id);
}

void IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Id);
}

void IndexBuffer::UnBind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}