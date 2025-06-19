#include "glm/fwd.hpp"
#include <codecvt>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <camera.h>

#include <locale>
#include <mesh.h>
#include <model.h>
#include <shader_s.h>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#define IMGUI_USE_WCHAR32
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl3_loader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GL_SILENCE_DEPRECATION

#include <iostream>
#include <random>
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
vector<glm::vec3> halfSphere();
GLuint make_ssao_noise();
void DSStencilPass(int PointLightIndex, Shader &shader, vector<glm::vec3> &lightpos,
                   vector<glm::vec3> &lightColors);
void DSPointLightPass(int PointLightIndex, Shader &shader, vector<glm::vec3> &lightpos,
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

// 字体管理
#define FONT_SIZE 60
vector<string> font_paths;
struct Character
{
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
    glm::vec4 Offset;
};
map<char32_t, Character> Characters;
int m_elementCount;
GLuint mtextureID, m_VAO = 0, m_VBO = 0, m_EBO = 0;
GLfloat mtextureWidth = 1000.0f, mtextureHeight = 1000.0f;
GLuint ChToTexture(u32string u32str);
void RenderText(Shader &m_shader, glm::mat4 &model, glm::mat4 vp, float thickness, float softness,
                float outline_thickness, float outline_softness, glm::vec2 text_shadow_offset);
void TextLoadString(u32string u32str, glm::vec2 screenPos, glm::vec2 typography);
GLuint ChToTexture(u32string u32str);

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
GLuint gBuffer, ssaoFBO, ssaoBlurFBO;
GLuint gPositionDepth, gNormal, gAlbedoSpec, ssaoColorBuffer, ssaoColorBufferBlur;
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
    Shader shaderSSAO((path + "shader/ssao.vs").c_str(), (path + "shader/ssao.fs").c_str());
    Shader shaderSSAO_blur((path + "shader/ssao_blur.vs").c_str(), (path + "shader/ssao_blur.fs").c_str());
    Shader shaderSSAO_last((path + "shader/ssaolast.vs").c_str(), (path + "shader/ssaolast.fs").c_str());
    Shader textShader((path + "shader/fontshader.vs").c_str(), (path + "shader/fontshader.fs").c_str());
    // Set samplers
    shaderLightPass.use();
    glUniform1i(glGetUniformLocation(shaderLightPass.ID, "gPositionDepth"), 0);
    glUniform1i(glGetUniformLocation(shaderLightPass.ID, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(shaderLightPass.ID, "gAlbedoSpec"), 2);
    glUniform1i(glGetUniformLocation(shaderLightPass.ID, "ssao"), 3);

    shaderSSAO.use();
    glUniform1i(glGetUniformLocation(shaderSSAO.ID, "gPositionDepth"), 0);
    glUniform1i(glGetUniformLocation(shaderSSAO.ID, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(shaderSSAO.ID, "texNoise"), 2);

    shaderSSAO_blur.use();
    glUniform1i(glGetUniformLocation(shaderSSAO_blur.ID, "ssaoInput"), 0);

    shaderSSAO_last.use();
    glUniform1i(glGetUniformLocation(shaderLightPass.ID, "gAlbedoSpec"), 0);
    glUniform1i(glGetUniformLocation(shaderLightPass.ID, "ssao"), 1);

    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);



    // create a color attachment texture

    // - Position color buffer
    glGenTextures(1, &gPositionDepth);
    glBindTexture(GL_TEXTURE_2D, gPositionDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionDepth, 0);
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


    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);

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

    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // 随机浮点数，范围0.0 - 100.0
    std::default_random_engine generator;

    const GLuint NR_LIGHTS = 3000;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    for (GLuint i = 0; i < NR_LIGHTS; i++)
    {
        // Calculate slightly random offsets
        GLfloat xPos = randomFloats(generator) * 6.0 - 3.0;
        GLfloat yPos = randomFloats(generator) * 6.0 - 4.0;
        GLfloat zPos = randomFloats(generator) * 6.0 - 3.0;
        lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
        // Also calculate random color
        GLfloat rColor = randomFloats(generator) / 2 + 0.5;   // Between 0.5 and 1.0
        GLfloat gColor = randomFloats(generator) / 2 + 0.5;   // Between 0.5 and 1.0
        GLfloat bColor = randomFloats(generator) / 2 + 0.5;   // Between 0.5 and 1.0
        lightColors.push_back(glm::vec3(rColor, gColor, bColor));
    }
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    int positionSize = 9, lightCount = 100;

    font_paths = {
        path + "resources/fonts/base-split.woff",
        path + "resources/fonts/HarmonyOS_Sans_SC_Medium.ttf",
        path + "resources/fonts/NotoSansArabic-Medium.ttf",
        path + "resources/fonts/NotoSansCanadianAboriginal-Medium.ttf",
        path + "resources/fonts/NotoSansCuneiform-Regular.ttf",
        path + "resources/fonts/NotoSansSymbols2-Regular.ttf",
        path + "resources/fonts/NotoSans-ExtraBold.ttf",
    };

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
        shaderGeometryPass.setFloat("NEAR", 0.1f);
        shaderGeometryPass.setFloat("FAR", 100.0f);
        glUniformMatrix4fv(
            glGetUniformLocation(shaderGeometryPass.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        for (GLuint i = 0; i < positionSize; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, objectPositions[i]);
            model = glm::scale(model, glm::vec3(0.25f));
            glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.ID, "model"), 1, GL_FALSE,
                               glm::value_ptr(model));
            model1.Draw(shaderGeometryPass);
        }
        glDepthMask(GL_FALSE);

        // -------------------------------------------------------------------------------------------
        // SSAO
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        shaderSSAO.use();
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPositionDepth);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        GLuint noiseTexture=make_ssao_noise();
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        shaderSSAO.setVec2("screenSize", SCR_WIDTH, SCR_HEIGHT);
        shaderSSAO.setMat4("view", view);
        shaderSSAO.setMat4("projection", projection);
        auto ssaoKernel = halfSphere();
        for (GLuint i = 0; i < 64; ++i)
            glUniform3fv(glGetUniformLocation(shaderSSAO.ID, ("samples[" + std::to_string(i) + "]").c_str()), 1,
                         &ssaoKernel[i][0]);
        RenderQuad();

        // -------------------------------------------------------------------------------------------
        // ssao + blur
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        shaderSSAO_blur.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
        RenderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // -------------------------------------------------------------------------------------------
        // 光体积渲染
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_STENCIL_TEST);
        glClear(GL_STENCIL_BUFFER_BIT);

        glClear(GL_COLOR_BUFFER_BIT);

        shaderSSAO_last.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
        RenderQuad();
        shaderLightPass.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPositionDepth);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);

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

        // -------------------------------------------------------------------------------------------
        // Render lights on top of scene, by blitting
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
        static char inputText[250] = "Ciallo~(∠・ω< )⌒★";
        static glm::vec2 textPosition(10.0f, 50.0f);  // 文本位置
        static glm::vec2 typography(0.0f, 0.0f);      // 行间距和字间距
        static float textThickness = 0.5f;            // 文本厚度
        static float textSoftness = 0.1f;             // 文本软化
        static float outlineThickness = 0.0f;         // 轮廓厚度
        static float outlineSoftness = 0.0f;          // 轮廓软化
        static glm::vec3 textColor(1.0f, 1.0f, 1.0f); // 文本颜色
        static glm::vec2 shadowOffset(0.0f, 0.0f);    // 阴影偏移

        ImGui::Begin("Text Settings");
        ImGui::InputText("Text", inputText, sizeof(inputText));
        ImGui::DragFloat2("Position", &textPosition[0], 1.0f, 0.0f, 1000.0f);
        ImGui::ColorEdit3("Color", &textColor[0]);
        ImGui::DragFloat("Thickness", &textThickness, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Softness", &textSoftness, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Outline Thickness", &outlineThickness, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Outline Softness", &outlineSoftness, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat2("Shadow Offset", &shadowOffset[0], 0.1f, -10.0f, 10.0f);
        ImGui::End();

        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        std::u32string u32str = conv.from_bytes(inputText);
        TextLoadString(u32str, textPosition, typography);
        glm::mat4 projectiontext =
            glm::ortho(0.0f, static_cast<float>(SCR_WIDTH),0.0f,static_cast < float > (SCR_HEIGHT));
        glm::mat4 modeltext = glm::mat4(1.0f);
        glm::mat4 vp = projectiontext;

        // 渲染文本
        textShader.use();
        textShader.setVec3("textColor", textColor);
        RenderText(textShader, modeltext, vp, textThickness, textSoftness, outlineThickness, outlineSoftness, shadowOffset);

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
    vector<GLuint> changeTex = {gPositionDepth, gNormal, gAlbedoSpec};
    vector<vector<GLuint>> changeType = {
        {GL_RGBA16F, GL_RGBA, GL_FLOAT}, {GL_RGB16F, GL_RGB, GL_FLOAT}, {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE}};

    for (int i = 0; i < changeTex.size(); i++)
    {
        glBindTexture(GL_TEXTURE_2D, changeTex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, changeType[i][0], width, height, 0, changeType[i][1], changeType[i][2], NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, changeTex[i], 0);
    }
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
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

vector<glm::vec3> halfSphere()
{
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // 随机浮点数，范围0.0 - 1.0
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for (GLuint i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0,
                         randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        GLfloat scale = GLfloat(i) / 64.0;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }
    return ssaoKernel;
}
GLuint make_ssao_noise()
{
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // 随机浮点数，范围0.0 - 1.0
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoNoise;
    for (GLuint i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
        ssaoNoise.push_back(noise);
    }
    GLuint noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return noiseTexture;
}

void DSStencilPass(int PointLightIndex, Shader &shader, vector<glm::vec3> &lightpos,
                 vector<glm::vec3> &lightColors)
{
    shader.use();
    glUniform3fv(glGetUniformLocation(shader.ID, "viewPos"), 1, &myCamera.Position[0]);
    glm::mat4 projection =
        glm::perspective(glm::radians(myCamera.Zoom), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = myCamera.GetViewMatrix();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    //shader.setVec2("screenSize", SCR_WIDTH, SCR_HEIGHT);

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
    // shader.setVec3("pointlight.Position", Position);
    // shader.setVec3("pointlight.Color", Color);
    // shader.setFloat("pointlight.Linear", Linear);
    // shader.setFloat("pointlight.Quadratic", Quadratic);

    glEnable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE);

    glClear(GL_STENCIL_BUFFER_BIT);

    glStencilFunc(GL_ALWAYS, 0, 0);

    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

    RenderSphere();
}
void DSPointLightPass(int PointLightIndex, Shader &shader, vector<glm::vec3> &lightpos,
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

GLuint ChToTexture(u32string u32str)
{
    bool texture_needs_update = false;
    for (const auto &ch : u32str)
    {
        if (Characters.find(ch) == Characters.end())
        {
            texture_needs_update = true;
            break;
        }
    }
    if (!texture_needs_update)
        return -1;

    static bool first = true;
    if (first)
    {
        glGenTextures(1, &mtextureID);
        glBindTexture(GL_TEXTURE_2D, mtextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, mtextureWidth, mtextureHeight, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        first = false;
    }

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    vector<FT_Face> faces;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (const string &font_path : font_paths)
    {
        FT_Face face;
        if (FT_New_Face(ft, font_path.c_str(), 0, &face))
        {
            cout << "ERROR::FREETYPE: Failed to load font" << font_path << std::endl;
            return -1;
        }
        FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);
        faces.push_back(face);
    }

    glBindTexture(GL_TEXTURE_2D, mtextureID);

    for (const auto &ch : u32str)
    {
        if (Characters.find(ch) != Characters.end())
            continue;

        static float texture_sub_x = 0.0f, texture_sub_y = 0.0f;
        static unsigned int row_height = 0;

        for (const auto &face : faces)
        {
            FT_UInt glyphIndex = FT_Get_Char_Index(face, ch);
            if (glyphIndex == 0)
                continue;
            else
            {
                // FT_Set_Transform(face, &matrix, &pen);
                FT_Load_Char(face, ch, FT_LOAD_RENDER);
                FT_GlyphSlot slot = face->glyph;
                // if (hasSDF)
                // {
                FT_Render_Glyph(slot, FT_RENDER_MODE_SDF);
                // }

                Character character;
                if (ch == ' ')
                {
                    character.Advance = static_cast<unsigned int>((face->glyph->advance.x) >> 6);
                    Characters.insert(std::pair<char32_t, Character>(ch, character));
                }
                else
                {
                    if (texture_sub_x + slot->bitmap.width + 1 >= mtextureWidth)
                    {
                        texture_sub_y += row_height;
                        texture_sub_x = 0;
                        row_height = 0;
                        if (texture_sub_y >= (mtextureHeight - FONT_SIZE))
                            cout << "text texture no enough" << endl;
                    }

                    character = {
                        glm::ivec2(slot->bitmap.width, slot->bitmap.rows),
                        glm::ivec2(slot->bitmap_left, slot->bitmap_top),
                        static_cast<unsigned int>((slot->advance.x) >> 6),
                        glm::vec4(texture_sub_x / 1000.0f, texture_sub_y / 1000.0f, slot->bitmap.width / 1000.0f,
                                  slot->bitmap.rows / 1000.0f)}; // 26.6 or // 2^6=64 ((face)->glyph->advance.x)/64)
                    Characters.insert(std::pair<FT_ULong, Character>(ch, character));

                    glTexSubImage2D(GL_TEXTURE_2D, 0, texture_sub_x, texture_sub_y, slot->bitmap.width,
                                    slot->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, slot->bitmap.buffer);
                    texture_sub_x += slot->bitmap.width + 1;

                    row_height = std::max(row_height, slot->bitmap.rows);
                }
                break;
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    for (auto &face : faces)
    {
        FT_Done_Face(face);
    }
    FT_Done_FreeType(ft);

    return 0;
}

void TextLoadString(u32string u32str, glm::vec2 screenPos, glm::vec2 typography)
{
    vector<float> vao_str;
    vector<GLuint> ibo_str;
    ChToTexture(u32str);

    unsigned int charsAdded = 0;
    for (auto c : u32str)
    {
        static float x_pos = screenPos.x;
        Character &ch = Characters[c];
        if (c == '\n')
        {
            screenPos.y -= (FONT_SIZE + typography.x);
            screenPos.x = x_pos;
        }
        else
        {
            if (c != ' ')
            {
                float xpos = (screenPos.x + ch.Bearing.x);
                float ypos = (screenPos.y - (ch.Size.y - ch.Bearing.y));
                float w = ch.Size.x;
                float h = ch.Size.y;
                
                vector<float> vertices = {xpos,
                                          ypos,
                                          ch.Offset.x,
                                          ch.Offset.y + ch.Offset.w,
                                          0.0f,
                                          1.0f,
                                          xpos,
                                          ypos + h,
                                          ch.Offset.x,
                                          ch.Offset.y,
                                          0.0f,
                                          0.0f,
                                          xpos + w,
                                          ypos,
                                          ch.Offset.x + ch.Offset.z,
                                          ch.Offset.y + ch.Offset.w,
                                          1.0f,
                                          1.0f,
                                          xpos + w,
                                          ypos + h,
                                          ch.Offset.x + ch.Offset.z,
                                          ch.Offset.y,
                                          1.0f,
                                          0.0f};
                vao_str.insert(vao_str.end(), vertices.begin(), vertices.end());

                unsigned int startIndex = charsAdded * 4;
                ibo_str.push_back(startIndex + 2);
                ibo_str.push_back(startIndex + 1);
                ibo_str.push_back(startIndex);
                ibo_str.push_back(startIndex + 3);
                ibo_str.push_back(startIndex + 1);
                ibo_str.push_back(startIndex + 2);
                ++charsAdded;
            }
            screenPos.x += ((ch.Advance) + typography.y);
        }
    }
    if (m_VAO == 0)
    {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);
    }
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vao_str.size(), vao_str.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibo_str.size() * sizeof(GLuint), ibo_str.data(), GL_DYNAMIC_DRAW);
    // 位置属性 (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 纹理坐标属性 (location = 1)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    m_elementCount = ibo_str.size(); // 索引数量
    glBindVertexArray(0);
}

void RenderText(Shader& m_shader, glm::mat4 &model , glm::mat4 vp, float thickness, float softness, float outline_thickness, float outline_softness, glm::vec2 text_shadow_offset)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    m_shader.use();
    // m_shader.SetUniform3f("texColor", AppControl::text_color);
    static float movement = 0.1;
    movement += 0.3 * deltaTime;
    m_shader.setFloat("deltaTime", (movement));
    m_shader.setMat4("mvp", vp * model);
    m_shader.setFloat("thickness", thickness);
    m_shader.setFloat("softness", softness);
    m_shader.setFloat("outline_thickness", outline_thickness);
    m_shader.setFloat("outline_softness", outline_softness);
    m_shader.setVec2("shadow_offset", text_shadow_offset / 1000.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mtextureID);

    glBindVertexArray(m_VAO);
    glActiveTexture(GL_TEXTURE0);
    glDrawElements(GL_TRIANGLES, m_elementCount, GL_UNSIGNED_INT, 0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}