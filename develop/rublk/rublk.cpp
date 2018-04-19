#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "stb/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <unistd.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>

#include "shader_m.h"
#include "camera.h"
#include "cube.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(3.0f, 4.0f, 5.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Rublk* g_r = nullptr;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (action != GLFW_RELEASE)
    return;
  switch (key)
  {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, true);
    break;

  case GLFW_KEY_SPACE:
    // 从背面观察rublk
    camera.Position = -camera.Position;
    camera.Front = -camera.Front;
    break;
  case GLFW_KEY_L:
    if (mods & GLFW_MOD_SHIFT) {
      g_r->pushEvent('L');
    } else {
      g_r->pushEvent('l');
    }
    break;
  case GLFW_KEY_R:
    if (mods & GLFW_MOD_SHIFT) {
      g_r->pushEvent('R');
    } else {
      g_r->pushEvent('r');
    }
    break;
  case GLFW_KEY_F:
    if (mods & GLFW_MOD_SHIFT) {
      g_r->pushEvent('F');
    } else {
      g_r->pushEvent('f');
    }
    break;
  case GLFW_KEY_B:
    if (mods & GLFW_MOD_SHIFT) {
      g_r->pushEvent('B');
    } else {
      g_r->pushEvent('b');
    }
    break;
  case GLFW_KEY_U:
    if (mods & GLFW_MOD_SHIFT) {
      g_r->pushEvent('U');
    } else {
      g_r->pushEvent('u');
    }
    break;
  case GLFW_KEY_D:
    if (mods & GLFW_MOD_SHIFT) {
      g_r->pushEvent('D');
    } else {
      g_r->pushEvent('d');
    }
    break;
  default:
    break;
  }
}

GLFWwindow* initGLFW(const char* title)
{
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  // --------------------
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
// lighting
//glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int main(int argc, char** argv)
{
  struct Optargs
  {
    bool c_flag;
  };

  Optargs opt {false};
  char ch;
  while((ch = getopt(argc, argv, "ch")) != -1) {
    switch(ch) {
    case 'c':
      opt.c_flag = true;
      break;
    case 'h':
      std::cout << " Usage: rublk [-c]" << std::endl
                << "    -c: clean the rublk cube, otherwise there will be a confused rublk cube."
                << std::endl;
      return -1;
      break;
    default:
      break;
    }
  }

  // initialize GLFW
  GLFWwindow* window = initGLFW("LearnOpenGL");
  if (!window) {
    return -1;
  }

  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // configure global opengl state
  glEnable(GL_DEPTH_TEST);

  // build and compile our shader zprogram
  Shader shader("rublk.vs", "rublk.fs");

  // load textures (we now use a utility function to keep the code more organized)
  unsigned int skin = loadTexture("textures/rublk.png");

  // 创建rublk cube
  Rublk rbulk(3, skin);
  g_r = &rbulk;
  if (!opt.c_flag) {
    rbulk.confuse();
  }

  // render loop
  while (!glfwWindowShouldClose(window))
  {
    // per-frame time logic
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    processInput(window);

    // render
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // be sure to activate shader when setting uniforms/drawing objects
    shader.use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    // render the rbulk
    rbulk.render(shader);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // glfw: terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  if (firstMouse)
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}
