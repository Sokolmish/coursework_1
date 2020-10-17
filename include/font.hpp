#ifndef __FONT_H__
#define __FONT_H__

#include "glew.hpp"
#include <string>
#include <vector>
#include <map>
#include <glm/vec2.hpp>
#include "shader.hpp"

class Font {
private:
    struct Character {
        int         width, height;
        int         bearingX, bearingY;
        GLuint      Advance;
        int         atlasX, atlasY;
    };
    friend struct RawChar;
    std::map<GLchar, Font::Character> characters;
    
    GLuint texture;
    GLuint VAO, VBO;
    
    uint32_t atlasWidth;
    uint32_t atlasHeight;
    uint32_t tileWidth;
    uint32_t tileHeight;
    uint8_t *atlas;
public:
    Font(const std::string &path, uint32_t width, uint32_t height);
    ~Font();
    void RenderText(const Shader &s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) const;
    void ShowAtlas(int x, int y, int width, int height) const;
};

#endif