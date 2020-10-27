#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <string>
#include <stdint.h>
#include <glm/vec3.hpp>

class ImageRGB {
private:
    int width, height;
    uint8_t *data;

    enum class ReleaseType { NONE, STBI, DELETE, FREE };
    ReleaseType releaseType;

    ImageRGB(int width, int height, uint8_t *data, ReleaseType rtype);
    
public:
    ImageRGB(int width, int height);
    ~ImageRGB();

    static ImageRGB fromFile(const std::string &path);
    static ImageRGB copyFromBuff(const uint8_t *buff, int width, int height);
    static ImageRGB useArray(uint8_t *buff, int width, int height);

    const uint8_t* getData() const;
    uint8_t* getData();
    
    int getWidth() const;
    int getHeight() const;

    glm::vec3 getPixel(int x, int y) const;
    int getPixelCode(int x, int y) const;
    void getPixelCmp(int x, int y, int *rgb) const;
    void getPixelCmp(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b) const;

    void setPixel(int x, int y, const glm::vec3 &color);
    void setPixel(int x, int y, int code);
    void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

    void fill(const glm::vec3 &color);
    void fill(int code);
    void fill(uint8_t r, uint8_t g, uint8_t b);
    
    void clear(bool isWhite = false);

    int writePNG(const std::string &path, bool flipVert = true) const;
    int writeBMP(const std::string &path, bool flipVert = true) const;

    static constexpr int R_MASK = (0xFF << 16);
    static constexpr int G_MASK = (0xFF << 8);
    static constexpr int B_MASK = 0xFF;
};

#endif