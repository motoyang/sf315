#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <queue>

#include "shader_m.h"

#include "singleton.h"
#include "camera.h"
#include "cube.h"
#include "glfwhandler.h"

// --

// settings
static const unsigned int SCR_WIDTH = 800;
static const unsigned int SCR_HEIGHT = 600;
static float deltaTime = 0.0f;

// --

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  static bool firstMouse = true;
  static float lastX = SCR_WIDTH / 2.0f;
  static float lastY = SCR_HEIGHT / 2.0f;

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  Singleton<Camera>::instance().ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  Singleton<Camera>::instance().ProcessMouseScroll(yoffset);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (action != GLFW_RELEASE) {
    return;
  }

  Camera& camera = Singleton<Camera>::instance();
  Rublk& rublk = Singleton<Rublk>::instance();
  switch (key)
  {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, true);
    break;

  case GLFW_KEY_SPACE:
    // 从背面观察rublk
    camera.setPosition(glm::vec3(0.0f, 0.0f, 0.0f) - camera.getPosition());
    camera.setFront(glm::vec3(0.0f, 0.0f, 0.0f) - camera.getFront());
    break;

  case GLFW_KEY_L:
    if (mods & GLFW_MOD_SHIFT) {
      rublk.pushEvent('L');
    } else {
      rublk.pushEvent('l');
    }
    break;
  case GLFW_KEY_R:
    if (mods & GLFW_MOD_SHIFT) {
      rublk.pushEvent('R');
    } else {
      rublk.pushEvent('r');
    }
    break;
  case GLFW_KEY_F:
    if (mods & GLFW_MOD_SHIFT) {
      rublk.pushEvent('F');
    } else {
      rublk.pushEvent('f');
    }
    break;
  case GLFW_KEY_B:
    if (mods & GLFW_MOD_SHIFT) {
      rublk.pushEvent('B');
    } else {
      rublk.pushEvent('b');
    }
    break;
  case GLFW_KEY_U:
    if (mods & GLFW_MOD_SHIFT) {
      rublk.pushEvent('U');
    } else {
      rublk.pushEvent('u');
    }
    break;
  case GLFW_KEY_D:
    if (mods & GLFW_MOD_SHIFT) {
      rublk.pushEvent('D');
    } else {
      rublk.pushEvent('d');
    }
    break;
  default:
    break;
  }
}

// --

float aspectGLFW()
{
  return (float)SCR_WIDTH / (float)SCR_HEIGHT;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
  Camera& camera = Singleton<Camera>::instance();

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
}

GLFWwindow* initGLFW(const char* title)
{
  // glfw: initialize and configure
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, title, NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return nullptr;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetKeyCallback(window, key_callback);

  return window;
}

void setDeltaTime(float dt)
{
  deltaTime = dt;
}
