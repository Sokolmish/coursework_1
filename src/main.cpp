#include "../include/glew.hpp"
#include "GLFW/glfw3.h"

#include <iostream>
#include <string>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../include/font.hpp"
#include "../include/waterMesh.hpp"

#define WINDOW_TITLE "Water visualization"

#define DEFAULT_WINDOW_WIDTH 1200
#define DEFAULT_WINDOW_HEIGHT 800
#define MIN_WINDOW_WIDTH 800
#define MIN_WINDOW_HEIGHT 400

#define SHADER_DIRECTORY "./shaders/"
#define FONT_TTF_PATH "./resources/consola.ttf"

void key_callback(GLFWwindow*, int, int, int, int);
void mouse_button_callback(GLFWwindow*, int, int, int);
void window_size_callback(GLFWwindow*, int, int);

void move(GLFWwindow *window, float dt);
std::string formatFloat(const std::string &format, float num);

struct Camera {
    glm::vec3 pos;
    float yaw, pitch, roll;
    float zoom;
    Camera() : 
        pos(glm::vec3(0.f)), yaw(0), pitch(0), roll(0), zoom(1.f) {}
    Camera(const glm::vec3 &pos, float yaw, float pitch) : 
        pos(pos), yaw(yaw), pitch(pitch), roll(0), zoom(1.f) {} 
} cam;

int main() {
    GLFWwindow* window;
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to initialize window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK || !GLEW_VERSION_2_1) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    Shader fontShader("./shaders/font.vert", "./shaders/font.frag");
    Font font(FONT_TTF_PATH, 0, 36);

    WaterMesh mesh(10, 10, 5.f);
    cam.pos.y = 1;

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float ratio = (float) width / (float) height;

    glfwSwapInterval(0);
    float timePhys = glfwGetTime();  // Used for physics, updates every frame
    float timeFPS = timePhys;        // Used for fps counting, updates every second
    uint framesCounter = 0;
    uint fps = 0;

    while (!glfwWindowShouldClose(window)) {
        // Time deltas
        float nTime = glfwGetTime();
        float dt = nTime - timePhys;
        timePhys = nTime;

        // FPS counting
        framesCounter++;
        if (nTime - timeFPS >= 1.0) {
            fps = framesCounter;
            framesCounter = 0;
            timeFPS = nTime;
        }

        glfwPollEvents();
        glfwGetWindowSize(window, &width, &height);
        ratio = (float) width / (float) height;
        glViewport(0, 0, width, height);
        glClearColor(0.509f, 0.788f, 0.902f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        move(window, dt);

        glm::mat4 m_proj_view =
            glm::perspective(45.f, ratio, 0.005f, 100.f) *
            glm::scale(glm::mat4(1.f), glm::vec3(0.3, 0.3, 0.3)) *
            glm::scale(glm::mat4(1.f), glm::vec3(cam.zoom, cam.zoom, 1.f)) *
            glm::rotate(glm::mat4(1.f), cam.roll, glm::vec3(0, 0, -1)) *
            glm::rotate(glm::mat4(1.f), cam.pitch, glm::vec3(-1, 0, 0)) *
            glm::rotate(glm::mat4(1.f), cam.yaw, glm::vec3(0, 1, 0)) *
            glm::translate(glm::mat4(1.f), -cam.pos);
        glm::mat4 m_ortho = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
        
        mesh.show(m_proj_view);
        
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        fontShader.use();
        fontShader.setUniform("projection", m_ortho);
        std::string debug = formatFloat("%.2f", glm::degrees(cam.yaw)) + 
            ":" + formatFloat("%.2f", glm::degrees(cam.pitch));
        font.RenderText(fontShader, debug, 10, height - 38, 0.6, glm::vec3(0.f));

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// Window events

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

}

void window_size_callback(GLFWwindow* window, int width, int height) {

}

// Movement

inline float stepYaw(float yaw, float d) {
    yaw = fmodf(yaw - d, 2 * M_PI);
    if (yaw < 0)
        return 2 * M_PI + yaw;
    else
        return yaw;
}

inline float stepPitch(float pitch, float d) {
    pitch -= d;
    if (pitch > M_PI_2)
        return M_PI_2;
    else if (pitch < -M_PI_2)
        return -M_PI_2;
    else
        return pitch;
}

void move(GLFWwindow *window, float dt) {
    float coeffMovement = 9.0f;
    float coeffCameraKeyboard = 2.1f;
    float coeffCameraMouse = 3.0f;

    // glm::vec3 viewDir(cosf(cam.pitch) * sinf(cam.yaw), sinf(cam.pitch), -cosf(cam.pitch) * cosf(cam.yaw));
    glm::vec3 moveDir(-sinf(cam.yaw), 0, cosf(cam.yaw));
    glm::vec3 leftDir = glm::vec3(moveDir.z, 0, -moveDir.x);

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        cam.yaw = stepYaw(cam.yaw, coeffCameraKeyboard * dt);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        cam.yaw = stepYaw(cam.yaw, -coeffCameraKeyboard * dt);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        cam.pitch = stepPitch(cam.pitch, -coeffCameraKeyboard * dt);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) {
        cam.pitch = stepPitch(cam.pitch, coeffCameraKeyboard * dt);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cam.pos += glm::vec3(-coeffMovement * dt * moveDir);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cam.pos  += glm::vec3(coeffMovement * dt * moveDir);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cam.pos  += glm::vec3(-coeffMovement * dt * leftDir);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cam.pos  += glm::vec3(coeffMovement * dt * leftDir);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cam.pos += coeffMovement * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cam.pos  += -coeffMovement * dt;
    }
}

// Misc

std::string formatFloat(const std::string &format, float num) {
    char str[16];
    snprintf(str, 16, format.c_str(), num);
    return std::string(str);
}