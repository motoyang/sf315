#ifndef CAMERA_H
#define CAMERA_H

// Defines several possible options for camera movement.
// Used as abstraction to stay away from window-system specific input methods
enum class CameraMovement {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT
};

// An abstract camera class that processes input and calculates the corresponding
// Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
  friend class Singleton<Camera>;

  class Impl;
  std::unique_ptr<Impl> m_pImpl;

protected:
  Camera();

public:
  virtual ~Camera();

  void initialize(const glm::vec3& position = glm::vec3(3.0f, 4.0f, 5.0f),
                  const glm::vec3& target = glm::vec3(0.0f, 0.0f, 0.0f),
                  const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

  // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
  glm::mat4 GetViewMatrix();

  // Processes input received from any keyboard-like input system.
  // Accepts input parameter in the form of camera defined ENUM (to
  // abstract it from windowing systems)
  void ProcessKeyboard(CameraMovement direction, float deltaTime);

  // Processes input received from a mouse input system. Expects the offset
  // value in both the x and y direction.
  void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

  // Processes input received from a mouse scroll-wheel event. Only requires
  // input on the vertical wheel-axis
  void ProcessMouseScroll(float yoffset);

  void setTarget(const glm::vec3& target);
  glm::vec3 getPosition() const;
  void setPosition(const glm::vec3& pos);
  float getZoom() const;
};

#endif

