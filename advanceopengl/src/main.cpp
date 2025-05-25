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
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
unsigned int loadTexture(char const *path);

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
float MoveSpeed = 2.5f;

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
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // 3.0+ only
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
    Shader ourShader((path + "shader/shader.vs").c_str(), (path + "shader/myshader.fs").c_str());
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
    // -------------------------------------------------------------------------------------------
    // tell opengl for each sampler to which texture unit it belongs to (only has
    // to be done once)
    ourShader.use(); // don't forget to activate/use the shader before setting
                     // uniforms!
    ourShader.setInt("material.diffuse", 0);
    ourShader.setInt("material.specular", 1);

    // // either set it manually like so:
    // glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
    // // or set it via the texture class
    // ourShader.setInt("texture2", 1);
    //stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);
    glm::vec3 cubePositions[] = {glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
                                 glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
                                 glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
                                 glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
                                 glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};

    Material::defaultDiffuse = materialManager.LoadTexture((path + "/resources/textures/container2.png").c_str());
    Material::defaultSpecular =
        materialManager.LoadTexture((path + "/resources/textures/container2_specular.png").c_str());

    Model model = Model((path + "resources/model/xilian/xilian.pmx").c_str());
    Model model1 = Model((path + "resources/model/yunli/yunli.pmx").c_str());

    glEnable(GL_STENCIL_TEST);
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // input
        // -----
        processInput(window);
        // glm::mat4 model(1.0f);
        // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

        glm::mat4 view;
        view = myCamera.GetViewMatrix();
        glm::mat4 projection(1.0f);
        projection = glm::perspective(glm::radians(myCamera.Zoom), SCR_WIDTH * 1.2f / SCR_HEIGHT, 0.1f, 100.0f);

        // 光源管理窗口
        ImGui::Begin("Light Manager");

        // 添加光源按钮
        if (ImGui::Button("Add Directional Light"))
        {
            lights.push_back({.type = DIRECTIONAL,
                              .ambient = glm::vec3(0.1f),
                              .diffuse = glm::vec3(0.5f),
                              .specular = glm::vec3(1.0f),
                              .direction = myCamera.Front});
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Point Light"))
        {
            lights.push_back({.type = POINT,
                              .ambient = glm::vec3(0.1f),
                              .diffuse = glm::vec3(0.8f),
                              .specular = glm::vec3(1.0f),
                              .position = myCamera.Position,
                              .constant = 1.0f,
                              .linear = 0.09f,
                              .quadratic = 0.032f});
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Spot Light"))
        {
            lights.push_back({.type = SPOT,

                              .ambient = glm::vec3(0.1f),
                              .diffuse = glm::vec3(0.8f),
                              .specular = glm::vec3(1.0f),
                              .direction = myCamera.Front,
                              .position = myCamera.Position,
                              .constant = 1.0f,
                              .linear = 0.09f,
                              .quadratic = 0.032f,
                              .cutOff = glm::cos(glm::radians(12.5f)),
                              .outerCutOff = glm::cos(glm::radians(15.0f))});
        }

        // 光源列表
        ImGui::BeginChild("Light List", ImVec2(200, 200), true);
        for (int i = 0; i < lights.size(); ++i)
        {
            char label[32];
            sprintf(label, "Light %d", i);
            if (ImGui::Selectable(label, selectedLight == i))
            {
                selectedLight = i;
            }
        }
        ImGui::EndChild();

        // 删除选中光源
        if (selectedLight >= 0 && selectedLight < lights.size())
        {
            if (ImGui::Button("Delete Selected Light"))
            {
                lights.erase(lights.begin() + selectedLight);
                selectedLight = -1;
            }

            // 光源参数编辑
            Light &light = lights[selectedLight];
            ImGui::ColorEdit3("Ambient", glm::value_ptr(light.ambient));
            ImGui::ColorEdit3("Diffuse", glm::value_ptr(light.diffuse));
            ImGui::ColorEdit3("Specular", glm::value_ptr(light.specular));

            switch (light.type)
            {
            case DIRECTIONAL:
                ImGui::DragFloat3("Direction", glm::value_ptr(light.direction), 0.1f, -1.0f, 1.0f);
                break;
            case POINT:
                ImGui::DragFloat3("Position", glm::value_ptr(light.position), 0.1f);
                ImGui::DragFloat("Constant", &light.constant, 0.01f);
                ImGui::DragFloat("Linear", &light.linear, 0.001f);
                ImGui::DragFloat("Quadratic", &light.quadratic, 0.0001f);
                break;
            case SPOT:
                float angle = glm::degrees(acos(light.cutOff));
                float outerAngle = glm::degrees(acos(light.outerCutOff));
                ImGui::DragFloat3("Position", glm::value_ptr(light.position), 0.1f);
                ImGui::DragFloat3("Direction", glm::value_ptr(light.direction), 0.1f, -1.0f, 1.0f);
                ImGui::DragFloat("Cutoff", &angle, 1.0f, 0.0f, 90.0f);
                ImGui::DragFloat("Outer Cutoff", &outerAngle, 1.0f, 0.0f, 90.0f);
                light.cutOff = glm::cos(glm::radians(angle));
                light.outerCutOff = glm::cos(glm::radians(outerAngle));
                break;
            }
        }
        ImGui::End();

        {
            // 物体管理窗口
            ImGui::Begin("Object Manager");
            if (ImGui::Button("Add Cube"))
            {
                gameObjects.push_back(
                    {.position = myCamera.Position + myCamera.Front * 2.0f,
                     .scale = glm::vec3(1),
                     .material = {.shininess = 32.0f, .diffuseColor = glm::vec3(1), .specularColor = glm::vec3(0.5f)}});
            }

            ImGui::BeginChild("Object List", ImVec2(200, 200), true);
            for (int i = 0; i < gameObjects.size(); ++i)
            {
                char label[32];
                sprintf(label, "Object %d", i);
                if (ImGui::Selectable(label, selectedObject == i))
                {
                    selectedObject = i;
                }
            }
            ImGui::EndChild();

            if (selectedObject >= 0 && selectedObject < gameObjects.size())
            {
                // 删除选中物体
                if (ImGui::Button("Delete Selected Object"))
                {
                    gameObjects.erase(gameObjects.begin() + selectedObject);
                    selectedObject = -1;
                }
                GameObject &obj = gameObjects[selectedObject];
                Material &mat = obj.material;

                // 材质类型选择
                ImGui::SeparatorText("Material Settings");
                ImGui::Combo("Material Type", (int *)&mat.type, "Color\0Texture\0");

                if (mat.type == MATERIAL_COLOR)
                {
                    ImGui::ColorEdit3("Diffuse Color", glm::value_ptr(mat.diffuseColor));
                    ImGui::ColorEdit3("Specular Color", glm::value_ptr(mat.specularColor));
                }
                else
                {
                    // 漫反射贴图选择
                    static char diffusePath[128] = "";
                    ImGui::InputText("Diffuse Map", diffusePath, IM_ARRAYSIZE(diffusePath));
                    if (ImGui::Button("Load Diffuse"))
                    {
                        mat.diffuseMap = materialManager.LoadTexture(diffusePath);
                    }

                    // 镜面贴图选择
                    static char specularPath[128] = "";
                    ImGui::InputText("Specular Map", specularPath, IM_ARRAYSIZE(specularPath));
                    if (ImGui::Button("Load Specular"))
                    {
                        mat.specularMap = materialManager.LoadTexture(specularPath);
                    }
                }
                ImGui::DragFloat3("Position", glm::value_ptr(obj.position), 0.1f);
                ImGui::DragFloat3("Scale", glm::value_ptr(obj.scale), 0.1f, 0.1f, 10.0f);
                ImGui::Checkbox("Show Wireframe", &obj.showWireframe);
                ImGui::DragFloat("Shininess", &mat.shininess, 1.0f, 1.0f, 256.0f);
            }
            ImGui::End();

            // 摄像机控制窗口
            ImGui::Begin("Camera Settings");
            ImGui::Checkbox("Mouse Control", &cameraMouseControl);
            ImGui::DragFloat("Camera Speed", &cameraSpeed, 0.1f, 1.0f, 10.0f);
            ImGui::DragFloat("Move Speed", &MoveSpeed, 0.1f, 1.0f, 10.0f);
            ImGui::Text("Position: (%.1f, %.1f, %.1f)", myCamera.Position.x, myCamera.Position.y, myCamera.Position.z);
            ImGui::Text("Front: (%.2f, %.2f, %.2f)", myCamera.Front.x, myCamera.Front.y, myCamera.Front.z);
            ImGui::Text("Pitch/Yaw: %.1f/%.1f", myCamera.Pitch, myCamera.Yaw);
            ImGui::End();
            // render container
            lightShader.use();
            for (int i = 0; i < lights.size(); i++)
            {
                if (!lights[i].enabled || lights[i].type == DIRECTIONAL)
                    continue;
                glm::mat4 model(1.0f);
                model = glm::translate(model, lights[i].position);
                model = glm::scale(model, glm::vec3(0.2f));
                lightShader.setMat4("model", model);
                lightShader.setMat4("view", view);
                lightShader.setMat4("projection", projection);
                glBindVertexArray(lightVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

            ourShader.use();

            // 在着色器中设置动态光源数量
            ourShader.setInt("numLights", (int)lights.size());

            // 传递所有光源数据
            for (int i = 0; i < lights.size(); ++i)
            {
                const Light &light = lights[i];
                std::string prefix = "lights[" + std::to_string(i) + "]";

                ourShader.setInt(prefix + ".type", light.type);
                ourShader.setVec3(prefix + ".ambient", light.ambient);
                ourShader.setVec3(prefix + ".diffuse", light.diffuse);
                ourShader.setVec3(prefix + ".specular", light.specular);

                switch (light.type)
                {
                case DIRECTIONAL:
                    ourShader.setVec3(prefix + ".direction", light.direction);
                    break;
                case POINT:
                    ourShader.setVec3(prefix + ".position", light.position);
                    ourShader.setFloat(prefix + ".constant", light.constant);
                    ourShader.setFloat(prefix + ".linear", light.linear);
                    ourShader.setFloat(prefix + ".quadratic", light.quadratic);
                    break;
                case SPOT:
                    ourShader.setVec3(prefix + ".position", light.position);
                    ourShader.setFloat(prefix + ".constant", light.constant);
                    ourShader.setFloat(prefix + ".linear", light.linear);
                    ourShader.setFloat(prefix + ".quadratic", light.quadratic);
                    ourShader.setVec3(prefix + ".direction", light.direction);
                    ourShader.setFloat(prefix + ".cutOff", light.cutOff);
                    ourShader.setFloat(prefix + ".outerCutOff", light.outerCutOff);
                    break;
                }
            }

            // material properties
            ourShader.setVec3("viewPos", myCamera.Position);
            ourShader.setFloat("material.shininess", 64.0f);
            glBindVertexArray(VAO);
            for (const auto &obj : gameObjects)
            {
                const Material &mat = obj.material;
                ourShader.setInt("material.useDiffuseTexture", mat.type == MATERIAL_TEXTURE ? 1 : 0);
                ourShader.setInt("material.useSpecularTexture", mat.type == MATERIAL_TEXTURE ? 1 : 0);

                if (mat.type == MATERIAL_TEXTURE)
                {
                    // 绑定贴图
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, mat.diffuseMap ? mat.diffuseMap : Material::defaultDiffuse);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, mat.specularMap ? mat.specularMap : Material::defaultSpecular);
                }
                else
                {
                    // 设置颜色值
                    ourShader.setVec3("material.diffuseColor", mat.diffuseColor);
                    ourShader.setVec3("material.specularColor", mat.specularColor);
                }

                ourShader.setFloat("material.shininess", mat.shininess);
                glm::mat4 model(1.0f);
                model = glm::translate(model, obj.position);
                model = glm::scale(model, obj.scale);
                ourShader.setMat4("model", model);
                ourShader.setMat4("view", view);
                ourShader.setMat4("projection", projection);
                glBindVertexArray(VAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            // 绘制模型
            ourShader.use();
            ourShader.setMat4("model", glm::mat4(1.0f));
            ourShader.setMat4("view", view);
            ourShader.setMat4("projection", projection);
            ourShader.setInt("material.useDiffuseTexture", 1);
            ourShader.setInt("material.useSpecularTexture", 1);
            ourShader.setInt("material.diffuse", 0);
            ourShader.setInt("material.specular", 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Material::defaultDiffuse);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, Material::defaultSpecular);
            //model.Draw(ourShader);
            // 绘制模型
            ourShader.use();
            glm::mat4 model1Mat(1.0f);
            model1Mat = glm::translate(model1Mat, glm::vec3(0.0f, 0.0f, -8.0f));
            ourShader.setMat4("model", model1Mat);
            ourShader.setMat4("view", view);
            ourShader.setMat4("projection", projection);
            ourShader.setInt("material.useDiffuseTexture", 1);
            ourShader.setInt("material.useSpecularTexture", 1);
            ourShader.setInt("material.diffuse", 0);
            ourShader.setInt("material.specular", 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Material::defaultDiffuse);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, Material::defaultSpecular);
            model1.Draw(ourShader);

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
            // etc.)
            // -------------------------------------------------------------------------------
            // Rendering
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }
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
        myCamera.ProcessKeyboard(FORWARD, deltaTime * MoveSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        myCamera.ProcessKeyboard(BACKWARD, deltaTime * MoveSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        myCamera.ProcessKeyboard(LEFT, deltaTime * MoveSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        myCamera.ProcessKeyboard(RIGHT, deltaTime * MoveSpeed);
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