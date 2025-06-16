#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <camera.h>
#include <mesh.h>
#include <model.h>
#include <shader_s.h>

#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
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

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GL_SILENCE_DEPRECATION

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

std::string get_current_dir()
{
    char buff[FILENAME_MAX]; // create string buffer to hold path
    GetCurrentDir(buff, FILENAME_MAX);
    string current_working_dir(buff);
    return current_working_dir;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void movement();
void RenderCube();
void RenderQuad();
void RenderSphere();
void DSStencilPass(unsigned int PointLightIndex, Shader &shader, vector<glm::vec3> &lightpos,
                   vector<glm::vec3> &lightColors);
void DSPointLightPass(unsigned int PointLightIndex, Shader &shader, vector<glm::vec3> &lightpos,
                   vector<glm::vec3> &lightColors);
unsigned int loadTexture(char const *path);
unsigned int loadCubemap(std::vector<std::string> faces);

// 光源类型
enum LightType
{
    DIRECTIONAL,
    POINT,
    SPOT
};

struct Light
{
    LightType type;
    bool enabled = true;

    // 通用参数
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    // 定向光
    glm::vec3 direction;

    // 点光源/聚光灯
    glm::vec3 position;
    float constant;
    float linear;
    float quadratic;

    // 聚光灯专用
    float cutOff;
    float outerCutOff;
};

// 材质类型
enum MaterialType
{
    MATERIAL_COLOR,
    MATERIAL_TEXTURE
};

// 材质结构体
struct Material
{
    MaterialType type = MATERIAL_COLOR;

    // 通用参数
    float shininess = 32.0f;

    // 颜色模式
    glm::vec3 diffuseColor = glm::vec3(1.0f);
    glm::vec3 specularColor = glm::vec3(0.5f);

    // 贴图模式
    unsigned int diffuseMap = 0;
    unsigned int specularMap = 0;

    // 默认贴图路径
    static unsigned int defaultDiffuse;
    static unsigned int defaultSpecular;
};

// 默认贴图
unsigned int Material::defaultDiffuse = 0;
unsigned int Material::defaultSpecular = 0;

class MaterialManager
{
  public:
    std::unordered_map<std::string, unsigned int> loadedTextures;

    unsigned int LoadTexture(const char *path)
    {
        if (loadedTextures.find(path) != loadedTextures.end())
        {
            return loadedTextures[path];
        }

        unsigned int textureID = loadTexture(path);
        if (textureID != 0)
        {
            loadedTextures[path] = textureID;
        }
        return textureID;
    }
};

MaterialManager materialManager;

struct GameObject
{
    glm::vec3 position;
    glm::vec3 scale;
    Material material;
    bool showWireframe = false;
};

// 光源管理
std::vector<Light> lights;
int selectedLight = -1;

// 物体管理
std::vector<GameObject> gameObjects;
int selectedObject = -1;

// 摄像机控制
bool cameraMouseControl = true;
float cameraSpeed = 2.5f;
float MoveSpeed = 10.0f;

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// 视口大小
// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

// fbo
GLuint gBuffer;
GLuint gPosition, gNormal, gAlbedoSpec;
GLuint rbo;
int samples = 4;

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
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // 3.0+ only
    glfwWindowHint(GLFW_SAMPLES, 4);
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
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, key_callback);
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
    // Shader ourShader((path + "shader/shader.vs").c_str(),(path + "shader/shader.fs").c_str()); // you can name your
    // shader files however you like
    Shader shaderGeometryPass((path + "shader/g_buffer.vs").c_str(), (path + "shader/g_buffer.fs").c_str());
    Shader shaderLightPass((path + "shader/deferred_shading.vs").c_str(), (path + "shader/deferred_shading.fs").c_str());
    Shader shaderLightPassnull((path + "shader/deferred_shading.vs").c_str(),
                               (path + "shader/deferred_shading_null.fs").c_str());
    Shader shaderLightBox((path + "shader/deferred_light_box.vs").c_str(),
                          (path + "shader/deferred_light_box.fs").c_str());
    // Set samplers
    shaderLightPass.use();
    glUniform1i(glGetUniformLocation(shaderLightPass.ID, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(shaderLightPass.ID, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(shaderLightPass.ID, "gAlbedoSpec"), 2);

    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    // create a color attachment texture

    // - Position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // - Normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // - Color + Specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't
    // be sampling these)
    GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // load and create a texture
    // -------------------------
    // -------------------------------------------------------------------------------------------
    // tell opengl for each sampler to which texture unit it belongs to (only has
    // to be done once)

    // // either set it manually like so:
    // glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
    // // or set it via the texture class
    // ourShader.setInt("texture2", 1);
    //
    // stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);

    Material::defaultDiffuse = materialManager.LoadTexture((path + "/resources/textures/container2.png").c_str());
    Material::defaultSpecular =
        materialManager.LoadTexture((path + "/resources/textures/container2_specular.png").c_str());

    Model model = Model((path + "resources/model/xilian/xilian.pmx").c_str());
    Model model1 = Model((path + "resources/model/yunli/yunli.pmx").c_str());
    Model model2 = Model((path + "resources/model/nanosuit_reflection/nanosuit.obj").c_str());
    std::vector<glm::vec3> objectPositions;
    const GLuint NR_Positions = 6;
    srand(13);
    objectPositions.push_back(glm::vec3(-3.0, -3.0, -3.0));
    objectPositions.push_back(glm::vec3(0.0, -3.0, -3.0));
    objectPositions.push_back(glm::vec3(3.0, -3.0, -3.0));
    objectPositions.push_back(glm::vec3(-3.0, -3.0, 0.0));
    objectPositions.push_back(glm::vec3(0.0, -3.0, 0.0));
    objectPositions.push_back(glm::vec3(3.0, -3.0, 0.0));
    objectPositions.push_back(glm::vec3(-3.0, -3.0, 3.0));
    objectPositions.push_back(glm::vec3(0.0, -3.0, 3.0));
    objectPositions.push_back(glm::vec3(3.0, -3.0, 3.0));

    const GLuint NR_LIGHTS = 3000;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    for (GLuint i = 0; i < NR_LIGHTS; i++)
    {
        // Calculate slightly random offsets
        GLfloat xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
        GLfloat yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
        GLfloat zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
        lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
        // Also calculate random color
        GLfloat rColor = ((rand() % 100) / 200.0f) + 0.5; // Between 0.5 and 1.0
        GLfloat gColor = ((rand() % 100) / 200.0f) + 0.5; // Between 0.5 and 1.0
        GLfloat bColor = ((rand() % 100) / 200.0f) + 0.5; // Between 0.5 and 1.0
        lightColors.push_back(glm::vec3(1.0, gColor, bColor));
    }
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    int positionSize = 9, lightCount = 1000;

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
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        movement();
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // 摄像机控制窗口
        ImGui::Begin("Camera Settings");
        ImGui::Checkbox("Mouse Control", &cameraMouseControl);
        ImGui::DragFloat("Camera Speed", &cameraSpeed, 0.1f, 1.0f, 10.0f);
        ImGui::DragFloat("Move Speed", &MoveSpeed, 0.1f, 1.0f, 10.0f);
        ImGui::DragInt("object Size", &positionSize, 1, 0, 3000);
        ImGui::DragInt("light Size", &lightCount, 1, 0, 3000);
        ImGui::Text("Position: (%.1f, %.1f, %.1f)", myCamera.Position.x, myCamera.Position.y, myCamera.Position.z);
        ImGui::Text("Front: (%.2f, %.2f, %.2f)", myCamera.Front.x, myCamera.Front.y, myCamera.Front.z);
        ImGui::Text("Pitch/Yaw: %.1f/%.1f", myCamera.Pitch, myCamera.Yaw);
        ImGui::End();
        // render
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glm::mat4 projection =
            glm::perspective(glm::radians(myCamera.Zoom), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = myCamera.GetViewMatrix();
        glm::mat4 model(1.0f);
        shaderGeometryPass.use();
        glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.ID, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        for (GLuint i = 0; i < positionSize; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, objectPositions[i]);
            model = glm::scale(model, glm::vec3(0.25f));
            glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.ID, "model"), 1, GL_FALSE,
                               glm::value_ptr(model));
            model2.Draw(shaderGeometryPass);
        }
        glDepthMask(GL_FALSE);

        // 光体积渲染
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_STENCIL_TEST);
        glClear(GL_STENCIL_BUFFER_BIT);

        glClear(GL_COLOR_BUFFER_BIT);

        shaderLightPass.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);


        for (int i = 0; i < lightCount; i++)
        {
            DSStencilPass(i, shaderLightPassnull, lightPositions, lightColors);
            DSPointLightPass(i, shaderLightPass, lightPositions, lightColors);
        }
        // RenderQuad();
        
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glDepthMask(GL_TRUE);
        // 3. Render lights on top of scene, by blitting
        shaderLightBox.use();
        glUniformMatrix4fv(glGetUniformLocation(shaderLightBox.ID, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderLightBox.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        for (GLuint i = 0; i < lightCount; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);
            model = glm::scale(model, glm::vec3(0.25f));
            glUniformMatrix4fv(glGetUniformLocation(shaderLightBox.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(glGetUniformLocation(shaderLightBox.ID, "lightColor"), 1, &lightColors[i][0]);
            RenderCube();
        }

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
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
bool keys[1024];
bool keysPressed[1024];
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
        {
            keys[key] = true;
            keysPressed[key] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            keys[key] = false;
            keysPressed[key] = false;
        }
    }
}
void movement()
{
    if (keys[GLFW_KEY_W])
        myCamera.ProcessKeyboard(FORWARD, deltaTime * MoveSpeed);
    if (keys[GLFW_KEY_S])
        myCamera.ProcessKeyboard(BACKWARD, deltaTime * MoveSpeed);
    if (keys[GLFW_KEY_A])
        myCamera.ProcessKeyboard(LEFT, deltaTime * MoveSpeed);
    if (keys[GLFW_KEY_D])
        myCamera.ProcessKeyboard(RIGHT, deltaTime * MoveSpeed);
    if (keys[GLFW_KEY_Q])
        myCamera.ProcessKeyboard(UP, deltaTime * MoveSpeed);
    if (keys[GLFW_KEY_E])
        myCamera.ProcessKeyboard(DOWN, deltaTime * MoveSpeed);
    if (keys[GLFW_KEY_F])
        cameraMouseControl = !cameraMouseControl; // 切换摄像机控制模式
    if (keys[GLFW_KEY_R])
        myCamera.Reset(); // 重置摄像机位置和方向
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width, SCR_HEIGHT = height;
    // 更新帧缓冲对象的大小
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    vector<GLuint> changeTex = {gPosition, gNormal, gAlbedoSpec};
    for (int i = 0; i < changeTex.size(); i++)
    {
        glBindTexture(GL_TEXTURE_2D, changeTex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, (changeTex[i] == gAlbedoSpec) ? GL_RGBA : GL_RGB16F, width, height, 0,
                     (changeTex[i] == gAlbedoSpec) ? GL_RGBA : GL_RGB,
                     (changeTex[i] == gAlbedoSpec) ? GL_UNSIGNED_BYTE : GL_FLOAT, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, changeTex[i], 0);
    }
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
}
// glfw: whenever the mouse moves, this callback is called
// ----------------------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    // 获取 ImGui 的 IO 状态
    ImGuiIO &io = ImGui::GetIO();

    // 如果 ImGui 正在使用鼠标，或左键未按下时，不处理视角移动
    if (io.WantCaptureMouse || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS || !cameraMouseControl)
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

    float xoffset = (xpos - lastX);
    float yoffset = lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
    if (cameraMouseControl)
    {
        xoffset *= cameraSpeed;
        yoffset *= cameraSpeed;
    }
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

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                         data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// RenderCube() Renders a 1x1 3D cube in NDC.
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
void RenderCube()
{
    // Initialize (if necessary)
    if (cubeVAO == 0)
    {
        GLfloat vertices[] = {
            // Back face
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
            // Front face
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            // Left face
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
            -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
            // Right face
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-left
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-right
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-left
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-left
            // Bottom face
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            // Top face
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // Fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // Link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // Render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// RenderQuad() Renders a 1x1 XY quad in NDC.
GLuint quadVAO = 0;
GLuint quadVBO = 0;
void RenderQuad()
{
    // Initialize (if necessary)
    if (quadVAO == 0)
    {
        GLfloat quadVertices[] = {
            // Positions        // Texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        // Fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        // Link vertex attributes
        glBindVertexArray(quadVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // Render Quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

GLuint sphereVAO = 0;
GLuint indexCount;

void RenderSphere()
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359;

        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
            }
        }

        bool oddRow = false;
        for (int i = 0; i < Y_SEGMENTS; i++)
        {
            for (int j = 0; j < X_SEGMENTS; j++)
            {

                indices.push_back(i * (X_SEGMENTS + 1) + j);
                indices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
                indices.push_back((i + 1) * (X_SEGMENTS + 1) + j);

                indices.push_back(i * (X_SEGMENTS + 1) + j);
                indices.push_back(i * (X_SEGMENTS + 1) + j + 1);
                indices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
            }
        }
        indexCount = indices.size();

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        unsigned int stride = (3 + 2) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

void DSStencilPass(unsigned int PointLightIndex, Shader &shader, vector<glm::vec3> &lightpos,
                 vector<glm::vec3> &lightColors)
{
    shader.use();
    glUniform3fv(glGetUniformLocation(shader.ID, "viewPos"), 1, &myCamera.Position[0]);
    glm::mat4 projection =
        glm::perspective(glm::radians(myCamera.Zoom), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = myCamera.GetViewMatrix();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec2("screenSize", SCR_WIDTH, SCR_HEIGHT);

    auto Position = lightpos[PointLightIndex];
    auto Color = lightColors[PointLightIndex];
    auto Linear = 0.7f;
    auto Quadratic = 1.8f;

    // 计算光源半径
    const float maxBrightness = std::fmaxf(std::fmaxf(lightColors[PointLightIndex].r, lightColors[PointLightIndex].g), lightColors[PointLightIndex].b);
    auto Radius = (-Linear + std::sqrt(Linear * Linear - 4 * Quadratic * (1.0 - (256.0f / 3.0f) * maxBrightness))) /
                  (2 * Quadratic);
    glm::mat4 model(1.0f);
    model = glm::translate(model, Position);
    model = glm::scale(model, glm::vec3(Radius));
    shader.setMat4("model", model);
    shader.setVec3("pointlight.Position", Position);
    shader.setVec3("pointlight.Color", Color);
    shader.setFloat("pointlight.Linear", Linear);
    shader.setFloat("pointlight.Quadratic", Quadratic);

    glEnable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE);

    glClear(GL_STENCIL_BUFFER_BIT);

    glStencilFunc(GL_ALWAYS, 0, 0);

    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

    RenderSphere();
}
void DSPointLightPass(unsigned int PointLightIndex, Shader &shader, vector<glm::vec3> &lightpos,
                      vector<glm::vec3> &lightColors)
{
    shader.use();
    glUniform3fv(glGetUniformLocation(shader.ID, "viewPos"), 1, &myCamera.Position[0]);
    glm::mat4 projection =
        glm::perspective(glm::radians(myCamera.Zoom), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = myCamera.GetViewMatrix();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec2("screenSize", SCR_WIDTH, SCR_HEIGHT);

    auto Position = lightpos[PointLightIndex];
    auto Color = lightColors[PointLightIndex];
    auto Linear = 0.7f;
    auto Quadratic = 1.8f;

    // 计算光源半径
    const float maxBrightness = std::fmaxf(std::fmaxf(lightColors[PointLightIndex].r, lightColors[PointLightIndex].g),
                                           lightColors[PointLightIndex].b);
    auto Radius = (-Linear + std::sqrt(Linear * Linear - 4 * Quadratic * (1.0 - (256.0f / 3.0f) * maxBrightness))) /
                  (2 * Quadratic);
    glm::mat4 model(1.0f);
    model = glm::translate(model, Position);
    model = glm::scale(model, glm::vec3(Radius));
    shader.setMat4("model", model);
    shader.setVec3("pointlight.Position", Position);
    shader.setVec3("pointlight.Color", Color);
    shader.setFloat("pointlight.Linear", Linear);
    shader.setFloat("pointlight.Quadratic", Quadratic);

    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    RenderSphere();
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
}