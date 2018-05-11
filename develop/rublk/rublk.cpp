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
#include <experimental/propagate_const>

#include "glfwhandler.h"
#include "texture.h"
#include "shader.h"

#include "singleton.h"
#include "camera.h"
#include "cube.h"
#include "lamp.h"

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
  camera.initialize(ORIGIN_CAMERA_POSITION);

  // positions of the point lights
  glm::vec3 pointLightPositions[] = {
      glm::vec3( 2.0f,  2.0f,  2.0f),
      glm::vec3( -2.0f, -2.0f, -2.0f),
      glm::vec3(2.0f,  -2.0f, -2.0f),
      glm::vec3( -2.0f,  2.0f, 2.0f)
  };

  // build and compile our shader zprogram
  Shader shader("shader/rublk.vs", "shader/rublk.fs");
  shader.use();
  shader.setInt("material.diffuse", 0);
  shader.setInt("material.specular", 1);
  shader.setFloat("material.shininess", 32.0f);

  // directional light
  shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
  shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
  shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
  shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
  // point light 1
  shader.setVec3("pointLights[0].position", pointLightPositions[0]);
  shader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
  shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
  shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
  shader.setFloat("pointLights[0].constant", 1.0f);
  shader.setFloat("pointLights[0].linear", 0.09);
  shader.setFloat("pointLights[0].quadratic", 0.032);
  // point light 2
  shader.setVec3("pointLights[1].position", pointLightPositions[1]);
  shader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
  shader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
  shader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
  shader.setFloat("pointLights[1].constant", 1.0f);
  shader.setFloat("pointLights[1].linear", 0.09);
  shader.setFloat("pointLights[1].quadratic", 0.032);
  // point light 3
  shader.setVec3("pointLights[2].position", pointLightPositions[2]);
  shader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
  shader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
  shader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
  shader.setFloat("pointLights[2].constant", 1.0f);
  shader.setFloat("pointLights[2].linear", 0.09);
  shader.setFloat("pointLights[2].quadratic", 0.032);
  // point light 4
  shader.setVec3("pointLights[3].position", pointLightPositions[3]);
  shader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
  shader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
  shader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
  shader.setFloat("pointLights[3].constant", 1.0f);
  shader.setFloat("pointLights[3].linear", 0.09);
  shader.setFloat("pointLights[3].quadratic", 0.032);
  // spotLight
  shader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
  shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
  shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
  shader.setFloat("spotLight.constant", 1.0f);
  shader.setFloat("spotLight.linear", 0.09);
  shader.setFloat("spotLight.quadratic", 0.032);
  shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
  shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

  // load textures (we now use a utility function to keep the code more organized)
  unsigned int diffuseMap = loadTexture("textures/rublk.png");
  unsigned int specularMap = loadTexture("textures/rublk_diffuse.png");

  // 初始化rublk cube
  Rublk& rublk = Singleton<Rublk>::instance();
  if (!rublk.initialize(3, diffuseMap, specularMap)) {
    std::cout << " rublk initialize failed." << std::endl;
    return -1;
  }

  if (!opt.c_flag) {
    rublk.confuse();
  }

  // lamps
  Shader lampShader("shader/lamp.vs", "shader/lamp.fs");
  std::vector<Lamp> lamps;
  for (const auto& p: pointLightPositions) {
    lamps.emplace_back(p);
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
    shader.setVec3("viewPos", camera.getPosition());
    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), aspectGLFW(), 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    // spotLight
    shader.setVec3("spotLight.position", camera.getPosition());
    shader.setVec3("spotLight.direction", camera.getFront());

    // render the rbulk
    rublk.render(shader);

    // also draw the lamp object
    lampShader.use();
    lampShader.setMat4("projection", projection);
    lampShader.setMat4("view", view);
    for (Lamp& lamp: lamps) {
      lamp.render(lampShader);
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // glfw: terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();
  return 0;
}
