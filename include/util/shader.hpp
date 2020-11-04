#ifndef __SHADER_H__
#define __SHADER_H__

#include "glew.hpp"

#include <string>
#include <map>
#include <exception>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class Shader {
private:
    enum class ShaderType {
        VERT = GL_VERTEX_SHADER,
        FRAG = GL_FRAGMENT_SHADER,
        GEOM = GL_GEOMETRY_SHADER,
        COMP = GL_COMPUTE_SHADER,
        TESC = GL_TESS_CONTROL_SHADER,
        TESE = GL_TESS_EVALUATION_SHADER,
    };
    friend std::string getShaderTypeName(ShaderType type);

    bool initialized = false;
    GLuint programId;

    GLuint compileShader(ShaderType type, const std::string &path) const;
    void linkProgram(GLuint programId) const;

public:
    Shader();
    Shader(const std::string &computePath);
    Shader(const std::string &vertexPath, const std::string &fragmentPath, const std::string &geometryPath = "");

    void use() const;
    GLuint getProgramId() const;

    void setUniform(const std::string &name, bool val) const;
    void setUniform(const std::string &name, GLint val) const;
    void setUniform(const std::string &name, GLfloat val) const;
    void setUniform(const std::string &name, GLfloat v1, GLfloat v2) const;
    void setUniform(const std::string &name, GLfloat v1, GLfloat v2, GLfloat v3) const;
    void setUniform(const std::string &name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) const;
    void setUniform(const std::string &name, const glm::vec2 &val) const;
    void setUniform(const std::string &name, const glm::vec3 &val) const;
    void setUniform(const std::string &name, const glm::vec4 &val) const;
    void setUniform(const std::string &name, const glm::mat4 &val) const;
};

#endif
