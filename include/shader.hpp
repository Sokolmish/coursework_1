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
    bool initialized = false;
    GLuint programId;

    std::string readFile(const std::string &path);

    static std::map<std::string, Shader> shaderStorage;
public:
    Shader();
    Shader(const std::string &vertexPath, const std::string &fragmentPath, const std::string &geometryPath = "");
    
    static std::string shaderDirectory;
    static Shader loadShader(const std::string &name);

    static void saveShader(const std::string &name, const Shader &shader);
    static Shader getShader(const std::string &name);
    
    void use() const;
    GLuint getProgramId() const;

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