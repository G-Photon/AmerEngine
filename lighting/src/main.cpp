#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <camara.h>
#include <shader_s.h>
#include <stb_image.h>

#include <iostream>
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl3_loader.h"
#define GL_SILENCE_DEPRECATION

#include <iostream>
using namespace std;

std::string get_current_dir()
{
    char buff[FILENAME_MAX]; // create string buffer to hold path
    GetCurrentDir(buff, FILENAME_MAX);
    string current_working_dir(buff);
    return current_working_dir;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
unsigned int loadTexture(char const *path);

struct DirLight
{
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct PointLight
{
    glm::vec3 position;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct SpotLight
{
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

void set_Uniform_DirLight(Shader &shader, DirLight &light);
void set_Uniform_PointLight(Shader &shader, PointLight &light ,int index);
void set_Uniform_SpotLight(Shader &shader, SpotLight &light);
void createLight();

PointLight pointLight[4];
DirLight dirLight;
SpotLight spotLight;

// positions of the point lights
glm::vec3 pointLightPositions[] = {glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
                                   glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(0.0f, 0.0f, -3.0f)};

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// 视口大小
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
// 鼠标控制摄像机
float yaw = -90.0f;     // yaw is initialized to -90.0 degrees, facing towards -Z
float pitch = 0.0f;     // pitch is initialized to 0.0 degrees, meaning looking straight up
bool firstMouse = true; // if first mouse input, set lastX and lastY to center of screen
static float lastX = SCR_WIDTH / 2.0f;
static float lastY = SCR_HEIGHT / 2.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
Camera myCamera(cameraPos, cameraUp, yaw, pitch);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
    string path = get_current_dir();
    path += "/../../../../";
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our zprogram
    // ------------------------------------
    //Shader ourShader((path + "shader/shader.vs").c_str(),(path + "shader/shader.fs").c_str()); // you can name your shader files however you like
    Shader ourShader((path + "shader/shader.vs").c_str(), (path + "shader/manyshader.fs").c_str());
    Shader lightShader((path + "shader/lightshader.vs").c_str(), (path + "shader/lightshader.fs").c_str());

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  0.0f, 1.0f, -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  1.0f, 1.0f,
        0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 1.0f,

        -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 1.0f};
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // position attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // 只需要绑定VBO不用再次设置VBO的数据，因为箱子的VBO数据中已经包含了正确的立方体顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 设置灯立方体的顶点属性（对我们的灯来说仅仅只有位置数据）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // load and create a texture
    // -------------------------
    unsigned int diffuseMap,SpecularMap,EmissionMap;
    diffuseMap = loadTexture((path + "resources/textures/container2.png").c_str());
    SpecularMap = loadTexture((path + "resources/textures/container2_specular.png").c_str());
    EmissionMap = loadTexture((path + "resources/textures/matrix.jpg").c_str());
    // -------------------------------------------------------------------------------------------
    // tell opengl for each sampler to which texture unit it belongs to (only has
    // to be done once)
    ourShader.use(); // don't forget to activate/use the shader before setting
                     // uniforms!
    ourShader.setInt("material.diffuse", 0);
    ourShader.setInt("material.specular", 1);
    ourShader.setInt("material.emission", 2);
    // // either set it manually like so:
    // glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
    // // or set it via the texture class
    // ourShader.setInt("texture2", 1);

    glEnable(GL_DEPTH_TEST);
    glm::vec3 cubePositions[] = {glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
                                 glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
                                 glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
                                 glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
                                 glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};

    createLight();

    // pass transformation matrix to shader (4 different ways)
    // -------------------------------------------------
    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the
    // imgui.ini file. You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        glfwPollEvents();
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // render
        // ------
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // input
        // -----
        processInput(window);
        // glm::mat4 model(1.0f);
        // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        static float xAngle = 0.0f;
        static float yAngle = 0.0f;
        static float zAngle = 0.0f;
        static float scale = 1.0f;
        {
            ImGui::Begin("Rotate Cube", 0, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::ColorEdit3("clear Color", (float *)&clear_color);
            ImGui::SliderFloat("X-Axis Angle", &xAngle, 0.0f, 360.0f);
            ImGui::SliderFloat("Y-Axis Angle", &yAngle, 0.0f, 360.0f);
            ImGui::SliderFloat("Z-Axis Angle", &zAngle, 0.0f, 360.0f);
            ImGui::SliderFloat("Model Scale", &scale, 0.1f, 5.0f);
            ImGui::End();
        }
        glm::mat4 view;
        view = myCamera.GetViewMatrix();
        glm::mat4 projection(1.0f);
        projection = glm::perspective(glm::radians(myCamera.Zoom), SCR_WIDTH * 1.2f / SCR_HEIGHT, 0.1f, 100.0f);

        static glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);    // 光源颜色变量
        {
            // 在 ImGui 的 NewFrame() 后添加
            ImGui::Begin("Light Settings");
            // 控制光源颜色
            ImGui::ColorEdit3("Light Color", (float *)&lightColor);
            ImGui::End();
        }
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, texture2);
        // glm::mat4 trans(1.0f);
        // trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
        // trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));

        // unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
        // glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
        // render container
        lightShader.use();
        for (int i = 0; i < 4; i++)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            lightShader.setMat4("model", model);
            lightShader.setMat4("view", view);
            lightShader.setMat4("projection", projection);
            lightShader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);
            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        ourShader.use();
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, SpecularMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, EmissionMap);

        
        spotLight.diffuse = lightColor * glm::vec3(0.5f);   // 降低影响
        spotLight.ambient = spotLight.diffuse * glm::vec3(0.2f); // 很低的影响
        spotLight.position = myCamera.Position;
        spotLight.direction = myCamera.Front;

        // // light properties
        // ourShader.setVec3("light.position", lightPosition);
        
        // ourShader.setVec3("light.ambient", ambientColor);
        // ourShader.setVec3("light.diffuse", diffuseColor);
        // ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        // ourShader.setFloat("light.constant", 1.0f);
        // ourShader.setFloat("light.linear", 0.09f);
        // ourShader.setFloat("light.quadratic", 0.032f);
        // ourShader.setVec3("light.position", myCamera.Position);
        // ourShader.setVec3("light.direction", myCamera.Front);
        // ourShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
        // ourShader.setFloat("light.outerCutOff", glm::cos(glm::radians(15.0f)));
        // ourShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
        set_Uniform_DirLight(ourShader, dirLight);
        for (int i = 0; i < 4; i++)
        {
            set_Uniform_PointLight(ourShader, pointLight[i],i);
        }
        set_Uniform_SpotLight(ourShader, spotLight);

        // material properties
        ourShader.setVec3("viewPos", myCamera.Position);
        ourShader.setFloat("material.shininess", 64.0f);
        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::scale(model, glm::vec3(scale));
            model = glm::rotate(model, glm::radians(xAngle), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(yAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(zAngle), glm::vec3(0.0f, 0.0f, 1.0f));
            ourShader.setMat4("model", model);
            ourShader.setMat4("view", view);
            ourShader.setMat4("projection", projection);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
        // etc.)
        // -------------------------------------------------------------------------------
        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        myCamera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        myCamera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        myCamera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        myCamera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
// glfw: whenever the mouse moves, this callback is called
// ----------------------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    // 获取 ImGui 的 IO 状态
    ImGuiIO &io = ImGui::GetIO();

    // 如果 ImGui 正在使用鼠标，或左键未按下时，不处理视角移动
    if (io.WantCaptureMouse || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
    {
        firstMouse = true; // 重置初始位置标记
        return;
    }
    if (firstMouse) // 这个bool变量初始时是设定为true的
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
    lastX = xpos;
    lastY = ypos;
    myCamera.ProcessMouseMovement(xoffset, yoffset);
}
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    myCamera.ProcessMouseScroll(yoffset);
}
// load a cubemap texture from 6 individual texture faces
unsigned int loadTexture(char const *path)
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

void set_Uniform_DirLight(Shader &shader, DirLight &light)
{
    shader.setVec3("dirLight.direction", light.direction);
    shader.setVec3("dirLight.ambient", light.ambient);
    shader.setVec3("dirLight.diffuse", light.diffuse);
    shader.setVec3("dirLight.specular", light.specular);
}

void set_Uniform_PointLight(Shader &shader, PointLight &light, int index)
{
    std::string prefix = "pointLights[" + std::to_string(index) + "].";
    shader.setVec3(prefix + "position", light.position);
    shader.setFloat(prefix + "constant", light.constant);
    shader.setFloat(prefix + "linear", light.linear);
    shader.setFloat(prefix + "quadratic", light.quadratic);
    shader.setVec3(prefix + "ambient", light.ambient);
    shader.setVec3(prefix + "diffuse", light.diffuse);
    shader.setVec3(prefix + "specular", light.specular);
}

void set_Uniform_SpotLight(Shader &shader, SpotLight &light)
{
    shader.setVec3("spotLight.position", light.position);
    shader.setVec3("spotLight.direction", light.direction);
    shader.setFloat("spotLight.cutOff", light.cutOff);
    shader.setFloat("spotLight.outerCutOff", light.outerCutOff);
    shader.setFloat("spotLight.constant", light.constant);
    shader.setFloat("spotLight.linear", light.linear);
    shader.setFloat("spotLight.quadratic", light.quadratic);
    shader.setVec3("spotLight.ambient", light.ambient);
    shader.setVec3("spotLight.diffuse", light.diffuse);
    shader.setVec3("spotLight.specular", light.specular);
}

void createLight()
{
    for (int i = 0; i < 4; i++)
    {
        pointLight[i].position = pointLightPositions[i];
        pointLight[i].ambient = glm::vec3(0.05f, 0.05f, 0.05f);
        pointLight[i].diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
        pointLight[i].specular = glm::vec3(1.0f, 1.0f, 1.0f);
        pointLight[i].constant = 1.0f;
        pointLight[i].linear = 0.09;
        pointLight[i].quadratic = 0.032;
    }

    dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    dirLight.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
    dirLight.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    dirLight.specular = glm::vec3(0.5f, 0.5f, 0.5f);

    spotLight.position = myCamera.Position;
    spotLight.direction = myCamera.Front;
    spotLight.cutOff = glm::cos(glm::radians(12.5f));
    spotLight.outerCutOff = glm::cos(glm::radians(15.0f));
    spotLight.constant = 1.0f;
    spotLight.linear = 0.09f;
    spotLight.quadratic = 0.032f;
    spotLight.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    spotLight.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
    spotLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
}