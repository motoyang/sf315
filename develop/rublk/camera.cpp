#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

#include "singleton.h"
#include "camera.h"

// --

class Camera::Impl
{
  const float SPEED       =  2.5f;
  const float SENSITIVITY =  0.1f;
  const float ZOOM        =  45.0f;

  // Calculates the front vector from the Camera's (updated) Euler Angles
  void updateCameraVectors()
  {
      // Calculate the new Front vector
      glm::vec3 front;
      front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
      front.y = sin(glm::radians(m_pitch));
      front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
      m_front = glm::normalize(front);

      // Also re-calculate the Right and Up vector
      m_right = glm::normalize(glm::cross(m_front, m_worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
      m_up    = glm::normalize(glm::cross(m_right, m_front));
  }

public:
  // Camera Attributes
  glm::vec3 m_pos;
  glm::vec3 m_front;
  glm::vec3 m_up;
  glm::vec3 m_right;
  glm::vec3 m_worldUp;

  // Euler Angles
  float m_yaw;
  float m_pitch;

  // Camera options
  float m_movementSpeed;
  float m_mouseSensitivity;
  float m_zoom;

  // Constructor with vectors
  void initialize(glm::vec3 position = glm::vec3(3.0f, 4.0f, 5.0f),
                  glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))

  {
      m_front = glm::vec3(0.0f, 0.0f, -1.0f);
      m_movementSpeed = SPEED;
      m_mouseSensitivity = SENSITIVITY;
      m_zoom = ZOOM;

      m_pos = position;
      m_worldUp = up;

      glm::vec3 p = target - position;
      // Yaw是在zx平面内，右手坐标系下，与x轴正方向的夹角
      m_yaw =  glm::degrees(glm::atan(p.z, p.x));
      float sin_pitch = p.y / glm::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
      // Pitch是与zx平面的夹角，y>0时是正值，y<0时是负值
      m_pitch = glm::degrees(glm::asin(sin_pitch));

      updateCameraVectors();
  }

  // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
  glm::mat4 GetViewMatrix() const
  {
      return glm::lookAt(m_pos, m_pos + m_front, m_up);
  }

  // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
  void ProcessKeyboard(CameraMovement direction, float deltaTime)
  {
      float velocity = m_movementSpeed * deltaTime;
      if (direction == CameraMovement::FORWARD)
          m_pos += m_front * velocity;
      if (direction == CameraMovement::BACKWARD)
          m_pos -= m_front * velocity;
      if (direction == CameraMovement::LEFT)
          m_pos -= m_right * velocity;
      if (direction == CameraMovement::RIGHT)
          m_pos += m_right * velocity;
  }

  // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
  void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
  {
      xoffset *= m_mouseSensitivity;
      yoffset *= m_mouseSensitivity;

      m_yaw   += xoffset;
      m_pitch += yoffset;

      // Make sure that when pitch is out of bounds, screen doesn't get flipped
      if (constrainPitch)
      {
          if (m_pitch > 89.0f)
              m_pitch = 89.0f;
          if (m_pitch < -89.0f)
              m_pitch = -89.0f;
      }

      // Update Front, Right and Up Vectors using the updated Euler angles
      updateCameraVectors();
  }

  // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
  void ProcessMouseScroll(float yoffset)
  {
      if (m_zoom >= 1.0f && m_zoom <= 45.0f)
          m_zoom -= yoffset;
      if (m_zoom <= 1.0f)
          m_zoom = 1.0f;
      if (m_zoom >= 45.0f)
          m_zoom = 45.0f;
  }

  glm::vec3 getPosition() const
  {
    return m_pos;
  }

  void setPosition(const glm::vec3& pos)
  {
    m_pos = pos;
  }

  glm::vec3 getFront() const
  {
    return m_front;
  }

  void setFront(const glm::vec3& front)
  {
    m_front = front;
  }

  float getZoom() const
  {
    return m_zoom;
  }
};

// --

Camera::Camera()
{
  m_pImpl = std::make_unique<Camera::Impl>();
}

Camera::~Camera()
{
}

void Camera::initialize(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
{
  m_pImpl->initialize(position, target, up);
}

glm::mat4 Camera::GetViewMatrix()
{
  return m_pImpl->GetViewMatrix();
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
  m_pImpl->ProcessKeyboard(direction, deltaTime);
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
  m_pImpl->ProcessMouseMovement(xoffset, yoffset, constrainPitch);
}

void Camera::ProcessMouseScroll(float yoffset)
{
  m_pImpl->ProcessMouseScroll(yoffset);
}

glm::vec3 Camera::getPosition() const
{
  return m_pImpl->getPosition();
}

void Camera::setPosition(const glm::vec3& pos)
{
  m_pImpl->setPosition(pos);
}

glm::vec3 Camera::getFront() const
{
  return m_pImpl->getFront();
}

void Camera::setFront(const glm::vec3 &front)
{
  m_pImpl->setFront(front);
}

float Camera::getZoom() const
{
  return m_pImpl->getZoom();
}


