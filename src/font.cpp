#include "../include/font.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GLFW/glfw3.h>
#include <iostream>
#include <exception>

struct RawChar {
    Font::Character ch;
    uint8_t *buff;
};

Font::Font(const std::string &path, uint32_t width, uint32_t height) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        throw std::runtime_error("Freetype: Could not init FreeType Library");
    }
    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face)) {
        throw std::runtime_error("Freetype: Failed to load font");
    }

    FT_Set_Pixel_Sizes(face, width, height);

    std::map<uint8_t, RawChar> rawChars;
    uint8_t xi = 0, yi = 0;
    tileWidth = 0;    
    tileHeight = 0;
    for (uint8_t ch = 0; ch < 128; ch++) {
        if (FT_Load_Char(face, ch, FT_LOAD_RENDER)) {
            std::cout << path << ": Failed to load Glyph " << ch << std::endl;
            continue;
        }

        tileWidth = std::max(tileWidth, face->glyph->bitmap.width);
        tileHeight = std::max(tileHeight, face->glyph->bitmap.rows);

        size_t tbuffSize = face->glyph->bitmap.width * face->glyph->bitmap.rows;
        uint8_t *tbuff = new uint8_t[tbuffSize];
        std::memcpy(tbuff, face->glyph->bitmap.buffer, tbuffSize);

        Character character{
            (int)face->glyph->bitmap.width, (int)face->glyph->bitmap.rows,
            face->glyph->bitmap_left, face->glyph->bitmap_top,
            (uint32_t)face->glyph->advance.x,
            xi, yi
        };
        rawChars.insert({ ch, RawChar{ character, tbuff } });

        xi++;
        if (xi == 16) {
            xi = 0;
            yi++;
        }
    }
    
    atlasWidth = tileWidth * 16;
    atlasHeight = tileHeight * 8;
    atlas = new uint8_t[atlasWidth * atlasHeight];
    std::memset(atlas, 0, atlasWidth * atlasHeight); // TODO: mark as debug feature

    for (const auto &ch : rawChars) {
        int ind = 0;
        const Font::Character &curCh = ch.second.ch;
        for (int j = 0; j < ch.second.ch.height; j++) {
            for (int i = 0; i < ch.second.ch.width; i++) {
                size_t index = (curCh.atlasY * tileHeight + j) * atlasWidth + (curCh.atlasX * tileWidth + i);
                atlas[index] = ch.second.buff[ind++];
            }
        }
        characters.insert({ ch.first, ch.second.ch });
        delete[] ch.second.buff;
    }
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft); 
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, atlas);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Font::~Font() {
    delete[] atlas;
}

void Font::RenderText(const Shader &s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) const {
    s.setUniform("textColor", color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    
    for (auto c = text.cbegin(); c != text.end(); c++) {
        Font::Character ch = characters.find(*c)->second;

        GLfloat xpos = x + ch.bearingX * scale;
        GLfloat ypos = y - (ch.height - ch.bearingY) * scale;

        GLfloat w = ch.width * scale;
        GLfloat h = ch.height * scale;

        float tex0x = (ch.atlasX * tileWidth) / (float)atlasWidth;
        float tex0y = (ch.atlasY * tileHeight) / (float)atlasHeight;
        float tex1x = (ch.atlasX * tileWidth + ch.width) / (float)atlasWidth;
        float tex1y = (ch.atlasY * tileHeight + ch.height) / (float)atlasHeight;

        GLfloat vertices[6][4] = {
            { xpos, ypos + h, tex0x, tex0y },
            { xpos, ypos, tex0x, tex1y },
            { xpos + w, ypos, tex1x, tex1y },
            { xpos, ypos + h, tex0x, tex0y },
            { xpos + w, ypos, tex1x, tex1y },
            { xpos + w, ypos + h, tex1x, tex0y }
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Font::ShowAtlas(int x, int y, int w, int h) const {
    GLfloat vertices[6][4] = {
        { (float)x, (float)y + h,     0, 0 },
        { (float)x, (float)y,         0, 1 },
        { (float)x + w, (float)y,     1, 1 },
        { (float)x, (float)y + h,     0, 0 },
        { (float)x + w, (float)y,     1, 1 },
        { (float)x + w, (float)y + h, 1, 0 }
    };
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}