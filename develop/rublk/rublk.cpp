#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <unistd.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>

#include "glfwhandler.h"
#include "texture.h"
#include "shader_m.h"

#include "singleton.h"
#include "camera.h"
#include "cube.h"

// lighting
//glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int main(int argc, char** argv)
{
  struct Optargs {
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

  // camera
  Camera& camera = Singleton<Camera>::instance();
  camera.initialize(glm::vec3(3.0f, 4.0f, 5.0f));

  // build and compile our shader zprogram
  Shader shader("shader/rublk.vs", "shader/rublk.fs");

  // load textures (we now use a utility function to keep the code more organized)
  unsigned int skin = loadTexture("textures/rublk.png");

  // 初始化rublk cube
  Rublk& rublk = Singleton<Rublk>::instance();
  if (!rublk.initialize(3, skin)) {
    std::cout << " rublk initialize failed." << std::endl;
    return -1;
  }

  if (!opt.c_flag) {
    rublk.confuse();
  }

  // render loop
  float lastFrame = 0.0f;
  while (!glfwWindowShouldClose(window))
  {
    // per-frame time logic
    float currentFrame = glfwGetTime();
    setDeltaTime(currentFrame - lastFrame);
    lastFrame = currentFrame;

    // input
    processInput(window);

    // render
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // be sure to activate shader when setting uniforms/drawing objects
    shader.use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), aspectGLFW(), 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    // render the rbulk
    rublk.render(shader);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // glfw: terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();
  return 0;
}
