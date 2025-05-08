#include <csignal>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include "config_parser.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <jsoncpp/json/json.h>

// Controle háptico condicional
#ifndef DISABLE_HAPTICS
#include "haptics.h"
#else
#include "haptic_simulator.h"
#endif

namespace fs = std::filesystem;

// --- Variáveis Globais ---
volatile sig_atomic_t emergency_stop = 0;
std::vector<GameInfo> games;

// --- Handlers ---
void emergency_handler(int sig) {
    emergency_stop = 1;
    std::cerr << "\nPARADA DE EMERGÊNCIA! Sinal: " << sig << std::endl;
}

std::vector<fs::path> listarDesafios(const fs::path& dir) {
    std::vector<fs::path> executaveis;
    if (!fs::exists(dir)) return executaveis;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file() &&
            (entry.status().permissions() & fs::perms::owner_exec) != fs::perms::none) {
            executaveis.push_back(entry.path());
        }
    }
    return executaveis;
}

// --- Inicialização do Sistema ---
bool inicializarSistema() {
    // Configurar sinais
    std::signal(SIGINT, emergency_handler);
    std::signal(SIGSEGV, emergency_handler);
    std::signal(SIGTERM, emergency_handler);

    // Inicialização háptica
#ifndef DISABLE_HAPTICS
    if (!Haptics::initDevice()) {
        std::cerr << "Falha na inicialização do dispositivo háptico" << std::endl;
        return false;
    }
#else
    HapticSimulator::init(); // Namespace corrigido
    std::cout << "Modo simulação ativado" << std::endl;
#endif

    // Carregar configurações
    std::string configPath = "games_config.json";

    // Verificação explícita
    if (!fs::exists(configPath)) {
        std::cerr << "Arquivo de configuração não encontrado em: "
                  << fs::absolute(configPath) << std::endl;
        return false;
    }
    auto configs = loadGameConfigs(configPath);
    if (configs.empty()) {
        std::cerr << "Nenhuma configuração de jogo encontrada!" << std::endl;
        return false;
    }

    // Mapear executáveis
    const fs::path diretorioDesafios = "/home/igor/chai3d-3.2.0-Makefiles/chai3d-3.2.0/bin/lin-x86_64";
    for (const auto& exec : listarDesafios(diretorioDesafios)) {
        std::string nome = exec.stem().string();
        if (configs.count(nome)) {
            games.push_back({exec, configs[nome]});
        }
    }

    return !games.empty();
}

// --- UI ---
void criarInterface(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io; // Corrige warning

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

// --- Loop Principal ---
void executarLoop(GLFWwindow* window) {
    char filtro[128] = {0};
    std::string materiaSelecionada = "Todas";
    std::string habilidadeSelecionada = "Todas";

    while (!glfwWindowShouldClose(window) && !emergency_stop) {
        glfwPollEvents();

        // Nova frame ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Janela de Filtros
        ImGui::Begin("Filtros", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::InputText("Pesquisar", filtro, sizeof(filtro));

        // Matérias
        std::set<std::string> materias;
        for (const auto& game : games) materias.insert(game.cfg.subject);

        if (ImGui::BeginCombo("Matéria", materiaSelecionada.c_str())) {
            if (ImGui::Selectable("Todas", materiaSelecionada == "Todas"))
                materiaSelecionada = "Todas";

            for (const auto& m : materias) {
                if (ImGui::Selectable(m.c_str(), m == materiaSelecionada))
                    materiaSelecionada = m;
            }
            ImGui::EndCombo();
        }

        // Habilidades
        std::set<std::string> habilidades;
        for (const auto& game : games)
            for (const auto& h : game.cfg.skills)
                habilidades.insert(h);

        if (ImGui::BeginCombo("Habilidade", habilidadeSelecionada.c_str())) {
            if (ImGui::Selectable("Todas", habilidadeSelecionada == "Todas"))
                habilidadeSelecionada = "Todas";

            for (const auto& h : habilidades) {
                if (ImGui::Selectable(h.c_str(), h == habilidadeSelecionada))
                    habilidadeSelecionada = h;
            }
            ImGui::EndCombo();
        }
        ImGui::End();

        // Lista de Jogos
        ImGui::Begin("Jogos Disponíveis", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        for (const auto& game : games) {
            bool filtroMateria = (materiaSelecionada == "Todas") || (game.cfg.subject == materiaSelecionada);
            bool filtroHabilidade = (habilidadeSelecionada == "Todas") ||
                                    (std::find(game.cfg.skills.begin(), game.cfg.skills.end(), habilidadeSelecionada) != game.cfg.skills.end());
            bool filtroTexto = (strlen(filtro) == 0) || (game.cfg.description.find(filtro) != std::string::npos);

            if (filtroMateria && filtroHabilidade && filtroTexto) {
                if (ImGui::Button(game.cfg.description.c_str())) {
                    (void)std::system((game.path.string() + " &").c_str()); // Corrige warning
                }
            }
        }
        ImGui::End();

        // Renderização
        ImGui::Render();
        int largura, altura;
        glfwGetFramebufferSize(window, &largura, &altura);
        glViewport(0, 0, largura, altura);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

// --- Ponto de Entrada ---
int main(int argc, char** argv) {
    (void)argc; (void)argv; // Corrige warnings de parâmetros não usados

    if (!inicializarSistema()) return EXIT_FAILURE;

    // Inicializar GLFW
    if (!glfwInit()) return EXIT_FAILURE;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Menu APAE", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glewInit();
    criarInterface(window);
    executarLoop(window);

    // Limpeza
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

#ifndef DISABLE_HAPTICS
    Haptics::shutdown();
#else
    HapticSimulator::shutdown(); // Namespace corrigido
#endif

    return EXIT_SUCCESS;
}
