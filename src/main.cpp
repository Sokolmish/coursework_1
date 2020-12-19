#include "../include/util/glew.hpp"
#include "GLFW/glfw3.h"

#include "../include/util/utility.hpp"
#include "../include/util/camera.hpp"
#include "../include/util/image.hpp"

#include "../include/debugInformer.hpp"
#include "../include/waterMeshChunk.hpp"
#include "../include/envSky.hpp"

#include <iostream>
#include <string>

#define WINDOW_TITLE "Water visualization"
#define DEFAULT_WINDOW_WIDTH 1200
#define DEFAULT_WINDOW_HEIGHT 800
#define MIN_WINDOW_WIDTH 800
#define MIN_WINDOW_HEIGHT 400

// Config

static constexpr float coeffMovement = 79.0f;
static constexpr float coeffCameraKeyboard = 1.4f;
static constexpr float coeffCameraMouse = 0.6f;

static constexpr bool disableVsync = false;

// States

static Camera cam;
static double oldmx, oldmy; // Used for looking around using mouse

static bool isMesh = false;
static bool isCursorHided = false;
static bool isFreeze = false;

// Prototypes

static bool initGraphics(GLFWwindow *&window);

static void key_callback(GLFWwindow*, int, int, int, int);
static void mouse_button_callback(GLFWwindow*, int, int, int);
static void window_size_callback(GLFWwindow*, int, int);

static void move(GLFWwindow *window, float dt);
static void takeScreenshot(GLFWwindow *window, const std::string &path);

// Main

int main() {
    GLFWwindow *window;
    if (!initGraphics(window)) {
        return -1;
    }

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float ratio = (float) width / (float) height;

    cam.setPos(1191, 306, 1767);
    cam.setViewDeg(84, -23);

    glm::vec3 skyCol = glm::vec3(135, 206, 235) / 255.f * 0.8f;

    EnvSky sky("", glm::vec3(0.5f, 0.5f, 0.0f), 10000.f, 500.f);
    sky.setSunCol(glm::vec3(255.f, 255.f, 59.f) / 255.f);

    WaterMeshChunk mesh(512, 7.5f, 0, 0);
    mesh.setWind({ 1.f, 0.f, 0.2f }, 180.f);
    mesh.setAmplitude(700.f);
    mesh.setGlobalAmbient(glm::vec3(0.35f, 0.35f, 0.45f));
    mesh.setDiffuse(glm::vec3(0.03f, 0.04f, 0.05f));
    mesh.setAmbient(glm::vec3(0.02f, 0.07f, 0.10f));
    mesh.setSpecular(glm::vec3(0.13f, 0.25f, 0.40f), 290.f);
    mesh.setBaseColor(glm::vec3(0.02f, 0.03f, 0.04f), glm::vec3(0.99f, 0.79f, 0.65f));

    mesh.setSky(sky);
    mesh.setSkyColor(skyCol);
    mesh.update();

    DebugInformer debugger;

    glClearColor(skyCol.r, skyCol.g, skyCol.b, 1.f);
    if constexpr (disableVsync)
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
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ratio = (float) width / (float) height;

        move(window, dt);
        if (!isFreeze)
            mesh.computePhysics(timePhys);

        glm::mat4 m_proj_view =
            glm::perspective(45.f, ratio, 0.1f, 2500.f) *
            glm::scale(glm::mat4(1.f), glm::vec3(0.3, 0.3, 0.3)) *
            glm::scale(glm::mat4(1.f), glm::vec3(cam.zoom, cam.zoom, 1.f)) *
            glm::rotate(glm::mat4(1.f), cam.roll, glm::vec3(0, 0, -1)) *
            glm::rotate(glm::mat4(1.f), cam.pitch, glm::vec3(-1, 0, 0)) *
            glm::rotate(glm::mat4(1.f), cam.yaw, glm::vec3(0, 1, 0)) *
            glm::translate(glm::mat4(1.f), -cam.pos);
        glm::mat4 m_sun =
            glm::perspective(45.f, ratio, 100.f, 100000.f) *
            glm::scale(glm::mat4(1.f), glm::vec3(0.3, 0.3, 0.3)) *
            glm::scale(glm::mat4(1.f), glm::vec3(cam.zoom, cam.zoom, 1.f)) *
            glm::rotate(glm::mat4(1.f), cam.roll, glm::vec3(0, 0, -1)) *
            glm::rotate(glm::mat4(1.f), cam.pitch, glm::vec3(-1, 0, 0)) *
            glm::rotate(glm::mat4(1.f), cam.yaw, glm::vec3(0, 1, 0));
        glm::mat4 m_ortho = glm::ortho(0.0f, (float) width, 0.0f, (float) height);

        sky.show(m_sun);
        mesh.show(m_proj_view, isMesh, cam);
        
        // mesh.showDebugImage(m_ortho, timePhys);

        debugger.setPos(cam.pos);
        debugger.setView(cam.yaw, cam.pitch);
        debugger.setFPS(fps);
        debugger.setCustomMsg("WatViz");
        debugger.show(m_ortho, width, height);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// Window events

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        isCursorHided = !isCursorHided;
        if (isCursorHided)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        oldmx = mx;
        oldmy = my;
    }
    else if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        isMesh = !isMesh;
    }
    else if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        isFreeze = !isFreeze;
    }
    else if (key == GLFW_KEY_I && action == GLFW_PRESS) {
        std::string path = "./screenshots/screenshot.png";
        std::cout << "Taking screenshot..." << std::endl;
        takeScreenshot(window, path);
        std::cout << "Screenshot saved in " << path << std::endl;
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {}
void window_size_callback(GLFWwindow *window, int width, int height) {}

// Movement

void move(GLFWwindow *window, float dt) {
    // View
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cam.yaw = stepYaw(cam.yaw, coeffCameraKeyboard * dt);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cam.yaw = stepYaw(cam.yaw, -coeffCameraKeyboard * dt);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cam.pitch = stepPitch(cam.pitch, -coeffCameraKeyboard * dt);
    if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS)
        cam.pitch = stepPitch(cam.pitch, coeffCameraKeyboard * dt);

    // Position
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam.pos += glm::vec3(-coeffMovement * dt * cam.getMoveDir());
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam.pos  += glm::vec3(coeffMovement * dt * cam.getMoveDir());
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam.pos  += glm::vec3(-coeffMovement * dt * cam.getLeftDir());
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam.pos  += glm::vec3(coeffMovement * dt * cam.getLeftDir());
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        cam.pos.y += coeffMovement * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        cam.pos.y += -coeffMovement * dt;

    // Mouse
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    cam.yaw = stepYaw(cam.yaw, (mx - oldmx) * -coeffCameraMouse * dt);
    cam.pitch = stepPitch(cam.pitch, (my - oldmy) * coeffCameraMouse * dt);
    oldmx = mx;
    oldmy = my;
}

// Init

bool initGraphics(GLFWwindow *&window) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to initialize window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK || !GLEW_VERSION_2_1) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    return true;
}

// Misc

void takeScreenshot(GLFWwindow *window, const std::string &path) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    GLubyte *buff = new GLubyte[width * height * 3l];

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buff);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);

    int ret = ImageRGB::useArray(buff, width, height).writePNG(path);
    if (ret == 0)
        std::cerr << "Something went wrong while saving the screenshot!" << std::endl;

    delete[] buff;
}
