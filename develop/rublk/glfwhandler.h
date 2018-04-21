#ifndef GLFWHANDER_H
#define GLFWHANDER_H

float aspectGLFW();
void processInput(GLFWwindow *window);
GLFWwindow* initGLFW(const char* title);
void setDeltaTime(float dt);

#endif // GLFWHANDER_H
