#include "../include/util/glew.hpp"
#include "GLFW/glfw3.h"

#include "../include/util/utility.hpp"

#include "../include/util/camera.hpp"
#include "../include/util/font.hpp"
#include "../include/util/image.hpp"

#include "../include/debugInformer.hpp"
#include "../include/waterMeshChunk.hpp"

#include "../include/dumbPhysics.hpp"
#include "../include/sineSumPhysics.hpp"

#include <iostream>
#include <string>

#define WINDOW_TITLE "Water visualization"
#define DEFAULT_WINDOW_WIDTH 1200
#define DEFAULT_WINDOW_HEIGHT 800
#define MIN_WINDOW_WIDTH 800
#define MIN_WINDOW_HEIGHT 400

// Prototypes

bool initGraphics(GLFWwindow *&window);

void key_callback(GLFWwindow*, int, int, int, int);
void mouse_button_callback(GLFWwindow*, int, int, int);
void window_size_callback(GLFWwindow*, int, int);

void move(GLFWwindow *window, float dt);

void takeScreenshot(GLFWwindow *window, const std::string &path);

// States

Camera cam;
double oldmx, oldmy; // Used for looking around using mouse

bool isMesh = false;
bool isCursorHided = false;
bool isFreeze = true;

// Main

int main() {
    GLFWwindow* window;
    if (!initGraphics(window)) {
        return -1;
    }

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float ratio = (float) width / (float) height;

    // WaterMeshChunk mesh(121, 121, 3.f);
    WaterMeshChunk mesh(61, 61, 3.f, WaterMeshChunk::OUTER);
    DebugInformer debugger;
    AbstractPhysics *phys;

    // phys = new DumbPhysics(3.2f, 0.87f, 3.1f);

    phys = new SineSumPhysics{
        SineSumPhysics::Wave(glm::vec3{ 1.f, 0.f, 0.08f }, 2.87f, 2.19f, 0.71f, 2.47f),
        SineSumPhysics::Wave(glm::vec3{ .5f, 0.f, 0.6f }, 1.76f, 3.12f, 0.52f, 4.47f),
        SineSumPhysics::Wave(glm::vec3{ 0.17f, 0.f, -0.24f }, 1.1f, 5.f, 0.7f, 0.85f),
        // SineSumPhysics::Wave(glm::vec3{ 0.37f, 0.f, 0.24f }, 0.4f, 5.5f, 1.2f, 6.f),
        SineSumPhysics::Wave(glm::vec3{ .15f, 0.f, 0.54f }, 1.02f, 2.91f, 0.32f, 3.17f),

        // SineSumPhysics::Wave(glm::vec3{ 1.f, 0.f, 0.08f }, 2.87f, 2.19f, 0.71f, 1.64f),
        // SineSumPhysics::Wave(glm::vec3{ .5f, 0.f, 0.6f }, 1.76f, 3.12f, 0.52f, 4.47f),
        // SineSumPhysics::Wave(glm::vec3{ 0.17f, 0.f, 0.54f }, 0.9f, 4.f, 0.5f, 0.85f),
        // SineSumPhysics::Wave(glm::vec3{ 0.37f, 0.f, 0.24f }, 0.4f, 4.5f, 0.9f, 6.f)
    };

    cam.pos = { 34.4, 40.09, 8.05 };
    cam.yaw = glm::radians(326.06);
    cam.pitch = glm::radians(-27.52);

    glClearColor(0.539f, 0.788f, 0.89f, 1.f);
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
            phys->process(mesh, timePhys);

        glm::mat4 m_proj_view =
            glm::perspective(45.f, ratio, 0.01f, 250.f) *
            glm::scale(glm::mat4(1.f), glm::vec3(0.3, 0.3, 0.3)) *
            glm::scale(glm::mat4(1.f), glm::vec3(cam.zoom, cam.zoom, 1.f)) *
            glm::rotate(glm::mat4(1.f), cam.roll, glm::vec3(0, 0, -1)) *
            glm::rotate(glm::mat4(1.f), cam.pitch, glm::vec3(-1, 0, 0)) *
            glm::rotate(glm::mat4(1.f), cam.yaw, glm::vec3(0, 1, 0)) *
            glm::translate(glm::mat4(1.f), -cam.pos);
        glm::mat4 m_ortho = glm::ortho(0.0f, (float)width, 0.0f, (float)height);

        mesh.show(m_proj_view, isMesh, cam);

        debugger.setPos(cam.pos);
        debugger.setView(cam.yaw, cam.pitch);
        debugger.setFPS(fps);
        debugger.setCustomMsg("WatViz");
        debugger.show(m_ortho, width, height);

        glfwSwapBuffers(window);
    }

    delete phys;
    glfwTerminate();
    return 0;
}

// Window events

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {

}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        // TODO: menu
    }
    else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
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

void window_size_callback(GLFWwindow *window, int width, int height) {

}

// Movement

void move(GLFWwindow *window, float dt) {
    float coeffMovement = 16.0f;
    float coeffCameraKeyboard = 1.4f;
    float coeffCameraMouse = 1.5f;

    glm::vec3 moveDir = cam.getMoveDir();
    glm::vec3 leftDir = cam.getLeftDir();

    // View
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

    // Position
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
        cam.pos.y += coeffMovement * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cam.pos.y += -coeffMovement * dt;
    }

    // Mouse
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    double dmx = mx - oldmx;
    double dmy = my - oldmy;
    oldmx = mx;
    oldmy = my;

    cam.yaw = stepYaw(cam.yaw, dmx * -coeffCameraMouse * dt);
    cam.pitch = stepPitch(cam.pitch, dmy * coeffCameraMouse * dt);
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
        std::cerr << "Something went wrong!" << std::endl;

    delete[] buff;
}
