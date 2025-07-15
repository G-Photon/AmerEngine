#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <glad/glad.h>


class Camera
{
  public:
    enum Movement
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f);

    void ProcessInput(GLFWwindow *window, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

    void Update(float deltaTime);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspectRatio) const;
    glm::vec3 GetPosition() const
    {
        return Position;
    }
    float GetFov() const
    {
        return Zoom;
    }

    float GetNearPlane() const
    {
        return 0.1f;
    }

    float GetFarPlane() const
    {
        return 100.0f;
    }


    // 相机属性
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // 欧拉角
    float Yaw;
    float Pitch;

    // 相机选项
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

  private:
    void UpdateCameraVectors();

    // 默认值
    static constexpr float SPEED = 5.0f;
    static constexpr float SENSITIVITY = 0.1f;
    static constexpr float ZOOM = 45.0f;
};