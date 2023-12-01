#include <modules/ImGuiNodes.h>
#include <includes/ObjectRelationLayout.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

struct DemoObject
{
    std::string name;
    std::vector<DemoObject *> childrens;
};

struct ObjectProxy : ObjectRelationLayout::ObjectInfoProxy
{
    DemoObject *object;
    ImGui::ImGuiNodesNode *node;
};

std::vector<DemoObject> g_objects;
std::vector<ObjectProxy> g_objectProxies;

void MakeDemoObjectsData()
{
    g_objects = {
        {"Object 1"},
        {"Object 2"},
        {"Object 3"},
        {"Object 4"},
        {"Object 5"},
        {"Object 6"},
        {"Object 7"},
        {"Object 8"},
        {"Object 9"},
        {"Object 10"},
        {"Object 11"},
        {"Object 12"},
        {"Object 13"},
        {"Object 14"},
        {"Object 15"},
        {"Object 16"},
        {"Object 17"},
        {"Object 18"},
        {"Object 19"},
        {"Object 20"},
    };

    g_objects[0].childrens.push_back(&g_objects[1]);
    g_objects[0].childrens.push_back(&g_objects[2]);
    g_objects[0].childrens.push_back(&g_objects[3]);

    g_objects[1].childrens.push_back(&g_objects[4]);
    g_objects[1].childrens.push_back(&g_objects[6]);
    g_objects[1].childrens.push_back(&g_objects[8]);

    g_objects[2].childrens.push_back(&g_objects[5]);
    g_objects[2].childrens.push_back(&g_objects[7]);
    g_objects[2].childrens.push_back(&g_objects[9]);

    g_objects[3].childrens.push_back(&g_objects[10]);
    g_objects[3].childrens.push_back(&g_objects[11]);

    g_objects[10].childrens.push_back(&g_objects[12]);
    g_objects[10].childrens.push_back(&g_objects[13]);
    g_objects[10].childrens.push_back(&g_objects[14]);
    g_objects[10].childrens.push_back(&g_objects[15]);
    g_objects[10].childrens.push_back(&g_objects[16]);
    g_objects[10].childrens.push_back(&g_objects[17]);

    g_objects[11].childrens.push_back(&g_objects[19]);

    g_objects[19].childrens.push_back(&g_objects[18]);
}

void MakeObjectProxiesData()
{
    std::unordered_map<DemoObject *, ObjectProxy *> objectProxyRelationMapping;

    g_objectProxies.resize(g_objects.size());
    for (size_t i = 0; i < g_objects.size(); ++i)
    {
        auto currentObject = &g_objects[i];
        auto currentObjectProxy = &g_objectProxies[i];

        currentObjectProxy->object = currentObject;
        objectProxyRelationMapping.emplace(currentObject, currentObjectProxy);
    }

    for (auto &objectProxy : g_objectProxies)
    {
        for (auto objectChild : objectProxy.object->childrens)
            objectProxy.childrens.push_back(objectProxyRelationMapping.at(objectChild));
    }
}

void RegisterObjectRelationNodeDesc(ImGui::ImGuiNodes &nodes)
{
    nodes.AddNodeDesc(
        {
            .name_ = "RelationShip",
            .type_ = ImGui::ImGuiNodesNodeType_Generic,
            .color_ = ImColor(0.2f, 0.3f, 0.6f, 0.0f),
            .inputs_ = {
                {
                    .name_ = "Parent",
                    .type_ = ImGui::ImGuiNodesConnectorType_Generic,
                },
            },
            .outputs_ = {
                {
                    .name_ = "Childrens",
                    .type_ = ImGui::ImGuiNodesConnectorType_Generic,
                },
            },
        });
}

void MakeLayout()
{
    static ObjectRelationLayout layout;

    layout.MakeLayout(&g_objectProxies[0]);
}

void AddNodes(ImGui::ImGuiNodes &nodes)
{
    for (auto &objectProxy : g_objectProxies)
    {
        auto node = nodes.AddNode("RelationShip");

        node->user_data_ = objectProxy.object;
        node->SetName(objectProxy.object->name.data());
        node->ToggleCollapse();
        node->MoveNode(objectProxy.position);

        objectProxy.node = node;
    }
}

void AddConnections(ImGui::ImGuiNodes &nodes)
{
    for (auto &objectProxy : g_objectProxies)
    {
        auto parentNode = objectProxy.node;

        for (auto child : objectProxy.childrens)
        {
            auto childNode = static_cast<ObjectProxy *>(child)->node;

            nodes.AddConnection(parentNode, childNode);
        }
    }
}

void Render()
{
    static ImGui::ImGuiNodes nodes(true);
    static bool initialized = false;

    if (!initialized)
    {
        MakeDemoObjectsData();
        MakeObjectProxiesData();

        RegisterObjectRelationNodeDesc(nodes);

        MakeLayout();

        AddNodes(nodes);
        AddConnections(nodes);

        initialized = true;
    }

    ImGui::SetNextWindowPos({}, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2{static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)}, ImGuiCond_Once);

    ImGui::Begin("Nodes", nullptr, ImGuiWindowFlags_NoMove);
    nodes.Update();
    nodes.ProcessNodes();
    nodes.ProcessContextMenu();
    ImGui::End();
}

int main()
{
    // Initialize glfw
    glfwSetErrorCallback(
        [](int error, const char *description)
        {
            std::cout << "[-] GLFW Error " << error << ": " << description << std::endl;
        });

    if (!glfwInit())
    {
        std::cout << "[-] GLFW init failed" << std::endl;
        return 1;
    }

    const char *glslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize imgui
    IMGUI_CHECKVERSION();

    auto imguiContext = ImGui::CreateContext();
    if (nullptr == imguiContext)
    {
        std::cout << "[-] ImGui create context failed" << std::endl;
        return 1;
    }

    {
        auto &imguiIO = ImGui::GetIO();

        imguiIO.IniFilename = nullptr;

        ImFontConfig fontConfig;
        fontConfig.SizePixels = 22.f;
        imguiIO.Fonts->AddFontDefault(&fontConfig);

        ImGui::StyleColorsDark();
        ImGui::GetStyle().ScaleAllSizes(3.f);
    }

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        std::cout << "[-] ImGui init GLFW failed" << std::endl;
        return 1;
    }
    if (!ImGui_ImplOpenGL3_Init(glslVersion))
    {
        std::cout << "[-] ImGui init OpenGL3 failed" << std::endl;
        return 1;
    }

    ImVec4 clearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Render();

        // Rendering
        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(imguiContext);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}