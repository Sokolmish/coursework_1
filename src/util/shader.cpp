#include "../../include/util/shader.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <dirent.h>

#include <glm/gtc/type_ptr.hpp>

#define SHADER_INFOLOG_BUFSIZE 1024

inline std::string getShaderTypeName(Shader::ShaderType type) {
    switch (type) {
    case Shader::ShaderType::VERT:
        return "vertex";
    case Shader::ShaderType::FRAG:
        return "fragment";
    case Shader::ShaderType::GEOM:
        return "geometry";
    case Shader::ShaderType::COMP:
        return "compute";
    case Shader::ShaderType::TESC:
        return "tesselation control";
    case Shader::ShaderType::TESE:
        return "tesselation evaluation";
    default:
        return "error_type";
    }
}

static std::string readFile(const std::string &path) {
    std::ifstream in;
    in.open(path);
    std::stringstream ss;
    ss << in.rdbuf();
    in.close();
    return ss.str();
}

GLuint Shader::compileShader(ShaderType type, const std::string &path) const {
    std::string src = readFile(path);
    GLint srcLen = src.length();
    const GLchar *ptr = src.c_str();
    GLuint shaderId = glCreateShader(static_cast<GLenum>(type));
    glShaderSource(shaderId, 1, &ptr, &srcLen);
    glCompileShader(shaderId);

    GLint succ;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &succ);
    if (!succ) {
        GLchar infoLog[SHADER_INFOLOG_BUFSIZE];
        GLsizei logSize;
        glGetShaderInfoLog(shaderId, SHADER_INFOLOG_BUFSIZE, &logSize, infoLog);
        std::cerr << "Shader compile error:" << std::endl ;
        std::cerr << "Type: " << getShaderTypeName(type) << " Path: " << path << std::endl;
        std::cerr << infoLog << std::endl;
        if (logSize >= SHADER_INFOLOG_BUFSIZE)
            std::cerr << "======== infolog cutted ========" << std::endl;
        throw std::runtime_error(getShaderTypeName(type) + " shader compile error");
    }

    return shaderId;
}

void Shader::linkProgram(GLuint programId) const {
    GLint succ;
    glLinkProgram(programId);
    glGetProgramiv(programId, GL_LINK_STATUS, &succ);
    if (!succ) {
        GLchar infoLog[SHADER_INFOLOG_BUFSIZE];
        GLsizei logSize;
        glGetProgramInfoLog(programId, SHADER_INFOLOG_BUFSIZE, &logSize, infoLog);
        std::cerr << "Shader linking error:" << std::endl << infoLog << std::endl;
        if (logSize >= SHADER_INFOLOG_BUFSIZE)
            std::cerr << "======== infolog cutted ========" << std::endl;
        throw std::runtime_error("Shader linking error");
    }
}

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath, const std::string &geometryPath) {
    GLuint vertexId = vertexPath.empty() ? 0 : compileShader(ShaderType::VERT, vertexPath);
    GLuint fragmentId = fragmentPath.empty() ? 0 : compileShader(ShaderType::FRAG, fragmentPath);
    GLuint geometryId = geometryPath.empty() ? 0 : compileShader(ShaderType::GEOM, geometryPath);

    programId = glCreateProgram();

    if (!vertexPath.empty())
        glAttachShader(programId, vertexId);
    if (!fragmentPath.empty())
        glAttachShader(programId, fragmentId);
    if (!geometryPath.empty())
        glAttachShader(programId, geometryId);

    linkProgram(programId);

    if (!vertexPath.empty())
        glDeleteShader(vertexId);
    if (!fragmentPath.empty())
        glDeleteShader(fragmentId);
    if (!geometryPath.empty())
        glDeleteShader(geometryId);

    initialized = true;
}

Shader::Shader(const std::string &computePath) {
    GLuint compId = compileShader(ShaderType::COMP, computePath);
    programId = glCreateProgram();
    glAttachShader(programId, compId);
    linkProgram(programId);
    glDeleteShader(compId);
    initialized = true;
}

Shader::Shader() {
    initialized = false;
}

//

void Shader::use() const {
    glUseProgram(programId);
}

GLuint Shader::getProgramId() const {
    return programId;
}

// Uniforms

void Shader::setUniform(const std::string &name, bool val) const {
    GLuint uniformLoc = glGetUniformLocation(programId, name.c_str());
    glUniform1i(uniformLoc, val ? 1 : 0);
}

void Shader::setUniform(const std::string &name, GLint val) const {
    GLuint uniformLoc = glGetUniformLocation(programId, name.c_str());
    glUniform1i(uniformLoc, val);
}

void Shader::setUniform(const std::string &name, GLfloat val) const {
    GLuint uniformLoc = glGetUniformLocation(programId, name.c_str());
    glUniform1f(uniformLoc, val);
}

void Shader::setUniform(const std::string &name, GLfloat v1, GLfloat v2) const {
    GLuint uniformLoc = glGetUniformLocation(programId, name.c_str());
    glUniform2f(uniformLoc, v1, v2);
}
void Shader::setUniform(const std::string &name, GLfloat v1, GLfloat v2, GLfloat v3) const {
    GLuint uniformLoc = glGetUniformLocation(programId, name.c_str());
    glUniform3f(uniformLoc, v1, v2, v3);
}
void Shader::setUniform(const std::string &name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) const {
    GLuint uniformLoc = glGetUniformLocation(programId, name.c_str());
    glUniform4f(uniformLoc, v1, v2, v3, v4);
}

void Shader::setUniform(const std::string &name, const glm::vec2 &val) const {
    GLuint uniformLoc = glGetUniformLocation(programId, name.c_str());
    glUniform2f(uniformLoc, val.x, val.y);
}

void Shader::setUniform(const std::string &name, const glm::vec3 &val) const {
    GLuint uniformLoc = glGetUniformLocation(programId, name.c_str());
    glUniform3f(uniformLoc, val.x, val.y, val.z);
}

void Shader::setUniform(const std::string &name, const glm::vec4 &val) const {
    GLuint uniformLoc = glGetUniformLocation(programId, name.c_str());
    glUniform4f(uniformLoc, val[0], val[1], val[2], val[3]);
}

void Shader::setUniform(const std::string &name, const glm::mat4 &val) const {
    GLuint uniformLoc = glGetUniformLocation(programId, name.c_str());
    glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(val));
}
