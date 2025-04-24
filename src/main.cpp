#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace fs = std::filesystem;

// Lista todos arquivos executáveis em um diretório
std::vector<fs::path> listarDesafios(const fs::path& dir) {
    std::vector<fs::path> execs;
    if (!fs::exists(dir) || !fs::is_directory(dir)) return execs;

    for (auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        auto perms = entry.status().permissions();
        // Checa permissão de execução para owner
        if ((perms & fs::perms::owner_exec) != fs::perms::none) {
            execs.push_back(entry.path());
        }
    }
    return execs;
}

int main(int argc, char** argv) {
    // Caminho para os exemplos do CHAI3D
    fs::path desafiosDir = "/home/igor/chai3d-3.2.0-Makefiles/chai3d-3.2.0/bin/lin-x86_64";
    auto desafios = listarDesafios(desafiosDir);
    std::vector<std::string> nomes;
    for (auto& p : desafios) nomes.push_back(p.filename().string());

    // --- GLFW init ---
    if (!glfwInit()) {
        std::cerr << "Falha ao iniciar GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Menu de Desafios CHAI3D", nullptr, nullptr);
    if (!window) {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // --- GLEW init ---
    if (glewInit() != GLEW_OK) {
        std::cerr << "Falha ao inicializar GLEW" << std::endl;
        return -1;
    }

    // --- ImGui init ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    int selecionado = -1;
    bool executar = false;

    // Loop principal
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Desafios")) {
                for (int i = 0; i < (int)nomes.size(); ++i) {
                    if (ImGui::MenuItem(nomes[i].c_str())) {
                        selecionado = i;
                        executar = true;
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Render ImGui
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        // Executar o desafio selecionado
        if (executar && selecionado >= 0 && selecionado < (int)desafios.size()) {
            std::string cmd = desafios[selecionado].string() + " &";  // background
            std::system(cmd.c_str());
            executar = false;
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
