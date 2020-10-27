#include "../../include/util/image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../../include/util/stb_image.hpp"
#include "../../include/util/stb_image_write.hpp"

#include <cstring> // Used for memset and memcpy

//
// Loaders
//

ImageRGB::ImageRGB(int width, int height, uint8_t *data, ReleaseType rtype) :
        width(width), height(height), data(data), releaseType(rtype) {}

ImageRGB::ImageRGB(int width, int height) :
        width(width), height(height), releaseType(ReleaseType::DELETE) {
    data = new uint8_t[width * height * 3];
}

ImageRGB ImageRGB::fromFile(const std::string &path) {
    int width, height, n;
    uint8_t *data = stbi_load(path.c_str(), &width, &height, &n, 3);
    return ImageRGB(width, height, data, ReleaseType::STBI);
}

ImageRGB ImageRGB::copyFromBuff(const uint8_t *buff, int width, int height) {
    uint8_t *data = new uint8_t[width * height * 3];
    std::memcpy(data, buff, width * height * 3);
    return ImageRGB(width, height, data, ReleaseType::DELETE);
}

ImageRGB ImageRGB::useArray(uint8_t *buff, int width, int height) {
    return ImageRGB(width, height, buff, ReleaseType::NONE);
}

//
// Basic getters
//

const uint8_t* ImageRGB::getData() const {
    return data;
}

uint8_t* ImageRGB::getData() {
    return data;
}

int ImageRGB::getWidth() const {
    return width;
}

int ImageRGB::getHeight() const {
    return height;
}

//
// Operations with pixels
//

glm::vec3 ImageRGB::getPixel(int x, int y) const {
    int code = getPixelCode(x, y);
    return glm::vec3(code & R_MASK, code & G_MASK, code & B_MASK);
}

void ImageRGB::getPixelCmp(int x, int y, int *rgb) const {
    *rgb = getPixelCode(x, y);
}

void ImageRGB::getPixelCmp(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b) const {
    int code = getPixelCode(x, y);
    *r = code & R_MASK;
    *g = code & G_MASK;
    *b = code & B_MASK;
}

int ImageRGB::getPixelCode(int x, int y) const {
    return static_cast<int32_t>(data[(width * y + x) * 3]) & 0xFFFFFF;
}


void ImageRGB::setPixel(int x, int y, const glm::vec3 &color) {
    setPixel(x, y, color.r, color.g, color.b);
}

void ImageRGB::setPixel(int x, int y, int code) {
    setPixel(x, y, code & R_MASK, code & G_MASK, code & B_MASK);
}

void ImageRGB::setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    int baseInd = (width * y + x) * 3;
    data[baseInd + 0] = r;
    data[baseInd + 1] = g;
    data[baseInd + 2] = b;
}


void ImageRGB::fill(const glm::vec3 &color) {
    fill(color.r, color.g, color.b);
}

void ImageRGB::fill(int code) {
    fill(code & R_MASK, code & G_MASK, code & B_MASK);
}

void ImageRGB::fill(uint8_t r, uint8_t g, uint8_t b) {
    size_t ind = 0;
    for (int i = 0; i < width * height; i++) {
        data[ind++] = r;
        data[ind++] = g;
        data[ind++] = b;
    }
}

void ImageRGB::clear(bool isWhite) {
    if (isWhite)
        std::memset(data, 0xFF, width * height * 3);
    else
        std::memset(data, 0, width * height * 3);
}

//
// Saving to file
//

int ImageRGB::writePNG(const std::string &path, bool flipVert) const {
    stbi_flip_vertically_on_write(flipVert);
    return stbi_write_png(path.c_str(), width, height, 3, data, width * 3);
}

int ImageRGB::writeBMP(const std::string &path, bool flipVert) const {
    stbi_flip_vertically_on_write(flipVert);
    return stbi_write_bmp(path.c_str(), width, height, 3, data);
}


//
// Utility
//

ImageRGB::~ImageRGB() {
    if (releaseType == ReleaseType::STBI)
        stbi_image_free(data);
    else if (releaseType == ReleaseType::DELETE)
        delete[] data;
    else if (releaseType == ReleaseType::FREE)
        free(data);
}
