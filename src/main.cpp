#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <thread>
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
#include <ctime>
#include <cstdint> // Para uintptr_t
#include "config_parser.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <jsoncpp/json/json.h>

#if DISABLE_HAPTICS == 0
#include "haptics.h"
#else
#include "haptic_simulator.h"
#endif

namespace fs = std::filesystem;

// --- Declarações Antecipadas ---
std::vector<fs::path> listarDesafios(const fs::path& dir);
bool loadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);

// --- Configurações e Constantes Globais ---
volatile sig_atomic_t emergency_stop = 0;
std::vector<GameInfo> games;

const char* FONT_DIR = "fonts/";
const char* ROBOTO_FONT_FILE = "Roboto-Medium.ttf";
const char* ICONS_FONT_FILE = "Font Awesome 6 Free-Solid-900.otf";
// ATUALIZE O CAMINHO DA IMAGEM SE NECESSÁRIO
const char* BACKGROUND_IMAGE_PATH = "assets/pankaj-shah-1ff_i7jO-4g-unsplash.jpg";

const fs::path CHAI3D_EXAMPLES_DIR = "/home/igor/chai3d-3.2.0-Makefiles/chai3d-3.2.0/bin/lin-x86_64";

std::set<std::string> g_availableSubjects;
std::set<std::string> g_availableSkills;

GLuint background_texture_id = 0;
int background_width = 0;
int background_height = 0;


// --- Handlers ---
void emergency_handler(int sig) {
    emergency_stop = 1;
    std::cerr << "\nPARADA DE EMERGÊNCIA! Sinal: " << sig << std::endl;
}

// --- Funções Auxiliares UI ---
namespace ImGui {
void Tag(const char* label, const ImVec4& color) {
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted("["); ImGui::SameLine(0.0f, 0.0f); ImGui::TextUnformatted(label); ImGui::SameLine(0.0f, 0.0f); ImGui::TextUnformatted("]");
    ImGui::PopStyleColor();
}
bool ButtonCustom(const char* label, const ImVec2& size) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    bool clicked = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    return clicked;
}
}

// --- Carregador de Textura com Debug Detalhado ---
bool loadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height) {
    std::cout << "Tentando carregar imagem de: ";
    // Tenta imprimir o caminho absoluto. fs::absolute pode falhar se o arquivo não existir ou o caminho for inválido.
    try {
        // Certifique-se que 'filename' não é nulo antes de chamar fs::absolute
        if (filename) {
            std::cout << fs::absolute(fs::path(filename)).string() << std::endl;
        } else {
            std::cout << "[caminho nulo]" << std::endl;
            return false; // Não pode carregar um arquivo nulo
        }
    } catch (const fs::filesystem_error& e) {
        std::cout << filename << " (erro ao obter caminho absoluto: " << e.what() << ")" << std::endl;
            // Continua tentando carregar com o caminho relativo, pois fs::absolute pode falhar em alguns cenários
    }

    int image_width_test = 0;
    int image_height_test = 0;
    int channels_in_file_test = 0;

    // 1. Tenta obter informações sobre a imagem sem decodificar os pixels
    if (stbi_info(filename, &image_width_test, &image_height_test, &channels_in_file_test)) {
        std::cout << "STB INFO: Arquivo '" << filename << "' parece ser uma imagem válida. Dimensões: "
                  << image_width_test << "x" << image_height_test << ", Canais no arquivo: " << channels_in_file_test << std::endl;
    } else {
        std::cerr << "STB INFO: Não foi possível obter informações do arquivo '" << filename << "'. Razão: " << stbi_failure_reason() << std::endl;
        // Não retorna false aqui, pois stbi_load pode ter mais sorte ou dar um erro diferente.
    }

    // 2. Tenta carregar a imagem, forçando 4 canais (RGBA)
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);

    if (image_data == NULL) {
        std::cerr << "STB LOAD (forçando 4 canais): Erro ao carregar imagem '" << filename << "'. Razão: " << stbi_failure_reason() << std::endl;
        return false;
    }

    std::cout << "STB LOAD (forçando 4 canais): Imagem carregada com sucesso. Dimensões: "
              << image_width << "x" << image_height << ". Dados em: " << (void*)image_data << std::endl;

    // Criar textura OpenGL
    glGenTextures(1, out_texture);
    glBindTexture(GL_TEXTURE_2D, *out_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "Erro OpenGL após glTexImage2D: " << err << std::endl;
        // Mesmo com erro OpenGL, precisamos liberar image_data
    }

    stbi_image_free(image_data);

    *out_width = image_width;
    *out_height = image_height;

    if (err != GL_NO_ERROR) { // Se houve erro no glTexImage2D, a textura não é válida
        glDeleteTextures(1, out_texture); // Limpa a textura gerada
        *out_texture = 0; // Zera o ID
        return false;
    }

    std::cout << "Textura OpenGL criada e dados carregados para: " << filename << std::endl;
    return true;
}


// Implementação da listarDesafios
std::vector<fs::path> listarDesafios(const fs::path& dir) {
    std::vector<fs::path> executaveis;
    if (!fs::exists(dir) || !fs::is_directory(dir)) { std::cerr << "Diretório de desafios não encontrado: " << dir << std::endl; return executaveis; }
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            try {
                if ((entry.status().permissions() & fs::perms::owner_exec) != fs::perms::none ||
                    (entry.status().permissions() & fs::perms::group_exec) != fs::perms::none ||
                    (entry.status().permissions() & fs::perms::others_exec) != fs::perms::none) {
                    executaveis.push_back(entry.path());
                }
            } catch (const fs::filesystem_error& e) { std::cerr << "Erro ao verificar permissões para " << entry.path() << ": " << e.what() << std::endl; }
        }
    }
    return executaveis;
}

// --- Inicialização do Sistema ---
bool inicializarSistema() {
    std::signal(SIGINT, emergency_handler); std::signal(SIGTERM, emergency_handler); std::signal(SIGSEGV, emergency_handler);
#if DISABLE_HAPTICS == 0
    // ... (haptics init)
#else
    HapticSimulator::init(); std::cout << "Modo simulação de hápticos ATIVADO." << std::endl;
#endif
    std::string configPath = "games_config.json";
    if (!fs::exists(configPath)) { std::cerr << "Config não encontrado: " << fs::absolute(configPath) << std::endl; return false; }
    auto configs = loadGameConfigs(configPath);
    if (configs.empty()) { std::cerr << "Nenhuma config de jogo carregada: " << configPath << std::endl; }
    std::cout << "Procurando desafios em: " << CHAI3D_EXAMPLES_DIR << std::endl;
    for (const auto& execPath : listarDesafios(CHAI3D_EXAMPLES_DIR)) {
        std::string nomeBaseExecutavel = execPath.stem().string();
        if (configs.count(nomeBaseExecutavel)) { games.push_back(GameInfo{execPath, configs[nomeBaseExecutavel]}); }
    }
    if (games.empty()) { std::cerr << "Nenhum jogo carregado." << std::endl; return false; }
    for (const auto& game : games) {
        g_availableSubjects.insert(game.cfg.subject);
        for (const auto& skill : game.cfg.skills) g_availableSkills.insert(skill);
    }
    std::cout << "Sistema inicializado com " << games.size() << " jogos." << std::endl;
    return true;
}

// Variável global para a fonte do título (ou passe como parâmetro para executarLoop se preferir)
ImFont* g_TitleFont = nullptr;

void criarInterface(GLFWwindow* window) {
    IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    std::string robotoPathStr = std::string(FONT_DIR) + ROBOTO_FONT_FILE;
    std::string iconsPathStr = std::string(FONT_DIR) + ICONS_FONT_FILE;

    // Fonte Padrão
    ImFont* fontRoboto = io.Fonts->AddFontFromFileTTF(robotoPathStr.c_str(), 18.0f);
    if (!fontRoboto) std::cerr << "AVISO: Falha ao carregar fonte Roboto." << std::endl;

    // Fonte para o Título (maior)
    g_TitleFont = io.Fonts->AddFontFromFileTTF(robotoPathStr.c_str(), 32.0f); // Ex: Roboto tamanho 32 para o título
    if (!g_TitleFont) {
        std::cerr << "AVISO: Falha ao carregar fonte do título. Usando fonte padrão para o título." << std::endl;
        g_TitleFont = fontRoboto; // Fallback para a fonte padrão se a do título falhar
    }

    // Ícones (merge com a fonte padrão ou com a primeira fonte carregada)
    ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
    // Adicionar os ícones à ÚLTIMA fonte adicionada antes de Build ou à fonte padrão.
    // Se fontRoboto foi a primeira, e titleFont a segunda, e você quer ícones na fonte principal:
    // Precisamos garantir que o merge seja com a fonte correta.
    // Para simplificar, vamos assumir que queremos merge com a primeira (fontRoboto).
    // Se a fonte principal for a primeira adicionada (index 0), podemos fazer o merge com ela.
    // Se g_TitleFont foi adicionada depois, e fontRoboto é a default:
    if (fontRoboto) { // Se a fonte principal carregou
        io.Fonts->AddFontFromFileTTF(iconsPathStr.c_str(), 16.0f, &icons_config, io.Fonts->GetGlyphRangesDefault()); // Merge com a última fonte adicionada antes desta linha
    } else { // Se a fonte principal falhou, tenta adicionar os ícones à default que o ImGui usará.
        ImFontConfig cfg; cfg.MergeMode = true; cfg.PixelSnapH = true;
        io.Fonts->AddFontDefault(&cfg); // Adiciona ícones à fonte default do ImGui.
        io.Fonts->AddFontFromFileTTF(iconsPathStr.c_str(), 16.0f, &cfg, io.Fonts->GetGlyphRangesDefault());
        std::cerr << "AVISO: Falha ao carregar fonte Ícones com a principal. Tentando com default." << std::endl;
    }
    // Ou, se você sempre quer os ícones na fonte Roboto principal (que foi carregada primeiro com tamanho 18):
    // Se fontRoboto != nullptr:
    // ImGui::PushFont(fontRoboto); // Define temporariamente como a fonte ativa para o merge
    // io.Fonts->AddFontFromFileTTF(iconsPathStr.c_str(), 16.0f, &icons_config);
    // ImGui::PopFont();

    io.Fonts->Build(); // CHAME APENAS UMA VEZ APÓS ADICIONAR TODAS AS FONTES

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg]    = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_ChildBg]     = ImVec4(0.15f, 0.17f, 0.20f, 0.75f);
    style.Colors[ImGuiCol_Text]        = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_Button]      = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_Header]      = ImVec4(0.29f, 0.34f, 0.40f, 1.00f);
    style.Colors[ImGuiCol_FrameBg]     = ImVec4(0.20f, 0.22f, 0.25f, 1.00f);
    style.ItemSpacing = ImVec2(12, 8); style.FrameRounding = 4.0f; style.WindowRounding = 4.0f; style.ChildRounding = 4.0f;
    ImGui_ImplGlfw_InitForOpenGL(window, true); ImGui_ImplOpenGL3_Init("#version 130");
    if (!loadTextureFromFile(BACKGROUND_IMAGE_PATH, &background_texture_id, &background_width, &background_height)) {
        std::cerr << "AVISO FINAL: Não foi possível carregar e criar a textura de fundo para '" << BACKGROUND_IMAGE_PATH << "'." << std::endl;
    }
}

void mostrarBarraStatus() {
    ImGuiViewport* viewport = ImGui::GetMainViewport(); float statusBarHeight = 28.0f;
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - statusBarHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, statusBarHeight));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.12f, 1.0f)); // Fundo opaco para status bar
    ImGui::Begin("StatusBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings);
    ImGui::PopStyleColor();
    time_t now_time = time(nullptr); char time_buf[100]; struct tm timeinfo;
#ifdef _WIN32
    localtime_s(&timeinfo, &now_time);
#else
    localtime_r(&now_time, &timeinfo);
#endif
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S  %d/%m/%Y", &timeinfo);
    const char* haptic_status_str = (DISABLE_HAPTICS == 1) ? "Simulação" : "Real";
    ImGui::Text("Dispositivo: %s | Jogos carregados: %zu | %s", haptic_status_str, games.size(), time_buf);
    ImGui::End();
}

void mostrarCardJogo(const GameInfo& game) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushID(game.path.string().c_str());

    // --- Dimensões Fixas para o Card ---
    float cardWidth = 290.0f;
    float cardHeight = 260.0f; // ALTURA FIXA PARA O CARD. Ajuste este valor!
        // Aumentei um pouco para dar mais espaço.

    // Usar o ImGuiCol_ChildBg definido globalmente em criarInterface
    ImGui::BeginChild("CardFrame", ImVec2(cardWidth, cardHeight), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

    // --- Header (Subject) ---
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    // Permitir que o título ocupe no máximo 2 linhas de altura
    float titleMaxHeight = ImGui::GetTextLineHeightWithSpacing() * 2.1f;
    ImGui::BeginChild("TitleRegion", ImVec2(0, titleMaxHeight), false, ImGuiWindowFlags_NoScrollbar); // false para sem borda, sem scrollbar
    ImGui::TextWrapped("%s", game.cfg.subject.c_str());
    ImGui::EndChild(); // TitleRegion
    ImGui::PopStyleColor();
    ImGui::Separator();

    // --- Corpo (Description) ---
    // Calcular a altura disponível para a descrição e tags, deixando espaço para o botão
    float currentY = ImGui::GetCursorPosY();
    float buttonAreaHeight = 30.0f + style.ItemSpacing.y * 2; // Altura do botão + espaçamento acima/abaixo dele
    float availableHeightForDescAndTags = cardHeight - currentY - buttonAreaHeight - style.WindowPadding.y; // Subtrai padding inferior do CardFrame

    float tagsFixedRenderHeight = ImGui::GetTextLineHeightWithSpacing() + style.FramePadding.y * 2 + style.ItemSpacing.y; // Altura da área de tags + espaçamento abaixo

    float descMaxHeight = availableHeightForDescAndTags - tagsFixedRenderHeight;
    if (descMaxHeight < ImGui::GetTextLineHeightWithSpacing() * 2) { // Mínimo de 2 linhas para descrição
        descMaxHeight = ImGui::GetTextLineHeightWithSpacing() * 2;
    }

    // Child para a descrição. Se o texto for maior, ele será cortado ou terá scroll.
    // Adicione ImGuiWindowFlags_AlwaysVerticalScrollbar se quiser scroll explícito.
    ImGui::BeginChild("DescriptionRegion", ImVec2(0, descMaxHeight), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextWrapped("%s", game.cfg.description.c_str());
    ImGui::EndChild(); // DescriptionRegion

    // --- Habilidades (Skills) com Scroll Horizontal ---
    if (!game.cfg.skills.empty()) {
        // A altura da área de tags é fixa
        float actualTagsAreaHeight = ImGui::GetTextLineHeightWithSpacing() + style.FramePadding.y * 2;
        std::string skillsChildID = "SkillsChild##"; skillsChildID += game.path.string();
        ImGui::BeginChild(skillsChildID.c_str(), ImVec2(0, actualTagsAreaHeight), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (size_t i = 0; i < game.cfg.skills.size(); ++i) {
            if (i > 0) ImGui::SameLine(0.0f, style.ItemSpacing.x); // Adiciona espaçamento entre tags
            ImGui::Tag(game.cfg.skills[i].c_str(), ImVec4(0.26f, 0.59f, 0.98f, 0.7f));
        }
        ImGui::EndChild(); // SkillsChild
    } else {
        // Se não houver skills, adicione um Dummy para ocupar o espaço e manter o alinhamento
        ImGui::Dummy(ImVec2(0, ImGui::GetTextLineHeightWithSpacing() + style.FramePadding.y * 2));
    }

    // --- Botão de Ação (Alinhar ao Fundo) ---
    // Empurrar o botão para baixo se houver espaço
    float finalContentY = ImGui::GetCursorPosY();
    float spaceToFill = cardHeight - finalContentY - 30.0f - style.WindowPadding.y - style.ItemSpacing.y;
    if (spaceToFill > style.ItemSpacing.y) { // Só adiciona Dummy se houver espaço significativo
        ImGui::Dummy(ImVec2(0.0f, spaceToFill - style.ItemSpacing.y)); // Deixa um ItemSpacing acima do botão
    } else if (spaceToFill < 0){ // Se o conteúdo já estourou, adiciona um pequeno espaço negativo para tentar puxar o botão para cima
        ImGui::Dummy(ImVec2(0.0f, spaceToFill));
    }
    ImGui::Spacing(); // Garante um pequeno espaço antes do botão

    if (ImGui::ButtonCustom("INICIAR", ImVec2(-1.0f, 30.0f))) {
        std::cout << "Iniciando jogo: " << game.path.string() << std::endl;
        std::thread([game_path_str = game.path.string()]() {
            int result = std::system(game_path_str.c_str());
            if (result != 0) std::cerr << "Erro ao executar o jogo (código: " << result << "): " << game_path_str << std::endl;
        }).detach();
    }

    ImGui::EndChild(); // CardFrame
    ImGui::PopID();
}

void mostrarFiltros(char* filtro_texto, std::string& materiaSelecionada, std::string& habilidadeSelecionada,
                    const std::set<std::string>& todasMaterias, const std::set<std::string>& todasHabilidades) {
    // Este BeginChild (Filtros e Pesquisa) usará o ImGuiCol_ChildBg definido no tema
    ImGui::Begin("Filtros e Pesquisa", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextUnformatted("Pesquisar por termo:");
    ImGui::InputTextWithHint("##SearchTerm", "Digite descrição, matéria, etc.", filtro_texto, 128); ImGui::Separator();
    if (ImGui::BeginTabBar("FiltrosTabBar", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Matérias")) {
            if (ImGui::BeginListBox("##MateriasListBox", ImVec2(-FLT_MIN, 150))) {
                if (ImGui::Selectable("Todas Matérias", materiaSelecionada == "Todas")) materiaSelecionada = "Todas";
                for (const auto& m : todasMaterias) if (ImGui::Selectable(m.c_str(), m == materiaSelecionada)) materiaSelecionada = m;
                ImGui::EndListBox();
            } ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Habilidades")) {
            if (ImGui::BeginListBox("##HabilidadesListBox", ImVec2(-FLT_MIN, 150))) {
                if (ImGui::Selectable("Todas Habilidades", habilidadeSelecionada == "Todas")) habilidadeSelecionada = "Todas";
                for (const auto& h : todasHabilidades) if (ImGui::Selectable(h.c_str(), h == habilidadeSelecionada)) habilidadeSelecionada = h;
                ImGui::EndListBox();
            } ImGui::EndTabItem();
        } ImGui::EndTabBar();
    } ImGui::End();
}

// --- Loop Principal ---
void executarLoop(GLFWwindow* window) {
    char filtro_texto[128] = {0};
    std::string materiaSelecionada = "Todas";
    std::string habilidadeSelecionada = "Todas";
    ImVec4 clear_color_fallback = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
    float overall_margin = 20.0f; // Margem geral para os painéis

    const char* projectTitle = "Projeto Jardim";

    while (!glfwWindowShouldClose(window) && !emergency_stop) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Desenhar a imagem de fundo primeiro
        if (background_texture_id != 0) {
            ImGui::GetBackgroundDrawList()->AddImage(
                (ImTextureID)(uintptr_t)background_texture_id,
                ImVec2(0, 0), ImVec2((float)display_w, (float)display_h),
                ImVec2(0, 0), ImVec2(1, 1));
        }

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        // Configurar a janela principal para ser transparente e ocupar toda a tela
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); // Padding da JanelaPrincipal é zero
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Fundo totalmente transparente

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                        ImGuiWindowFlags_NoNavFocus;
        ImGui::Begin("JanelaPrincipal", nullptr, window_flags);
        ImGui::PopStyleColor(); // Restaura ImGuiCol_WindowBg
        ImGui::PopStyleVar(3);  // Restaura rounding, border, padding

        // --- Barra de Menu ---
        float menuBarHeight = 0.0f;
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Arquivo")) {
                if (ImGui::MenuItem("Sair", "Alt+F4")) { emergency_stop = 1; }
                ImGui::EndMenu();
            }
            menuBarHeight = ImGui::GetFrameHeight(); // Altura da barra de menu
            ImGui::EndMenuBar();
        }

        // --- ÁREA DO TÍTULO COM SOMBRA E COR ---
        float titleAreaEffectiveHeight = 0.0f;
        ImFont* fontToUseForTitle = g_TitleFont ? g_TitleFont : ImGui::GetFont();

        ImGui::PushFont(fontToUseForTitle);
        ImVec2 titleTextSize = ImGui::CalcTextSize(projectTitle);
        // Calcular altura da área do título: altura do texto + padding vertical (ex: 10px acima, 10px abaixo)
        float titleVerticalPadding = 10.0f;
        titleAreaEffectiveHeight = titleTextSize.y + (titleVerticalPadding * 2);

        float titleActualPosX = (viewport->WorkSize.x - titleTextSize.x) * 0.5f;
        float titleActualPosY = menuBarHeight + titleVerticalPadding;

        ImU32 shadowColor = IM_COL32(0, 0, 0, 180);
        ImU32 titleMainColor = IM_COL32(255, 255, 240, 255);
        float shadowOffset = 2.0f;

        // Desenhar na draw list da janela atual ("JanelaPrincipal")
        // As coordenadas são relativas ao canto superior esquerdo da JanelaPrincipal (que é viewport->WorkPos)
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddText(
            fontToUseForTitle,
            fontToUseForTitle->FontSize, // O tamanho da fonte já está embutido no ImFont*
            ImVec2(titleActualPosX + shadowOffset, titleActualPosY + shadowOffset),
            shadowColor,
            projectTitle
            );
        drawList->AddText(
            fontToUseForTitle,
            fontToUseForTitle->FontSize,
            ImVec2(titleActualPosX, titleActualPosY),
            titleMainColor,
            projectTitle
            );
        ImGui::PopFont();
        // Fim da renderização do título

        // --- PAINÉIS LATERAIS (Filtros e Jogos) ---
        ImGuiStyle& style_loop = ImGui::GetStyle();
        // Posição Y inicial dos painéis: abaixo da menubar E abaixo da área calculada do título, mais uma margem
        float panelsStartY = menuBarHeight + titleAreaEffectiveHeight + overall_margin * 0.5f;

        // Definir a posição inicial do cursor para o primeiro painel (FiltrosPane)
        // Esta posição é relativa à JanelaPrincipal
        ImGui::SetCursorPos(ImVec2(overall_margin, panelsStartY));

        // Altura disponível para os painéis, considerando a margem inferior e a barra de status
        float availablePaneHeight = viewport->WorkSize.y - panelsStartY - overall_margin - ImGui::GetFrameHeightWithSpacing();
        if(availablePaneHeight < 150) availablePaneHeight = 150; // Altura mínima para os painéis

        float filterPaneWidth = 300.0f;
        ImGui::BeginChild("FiltrosPane", ImVec2(filterPaneWidth, availablePaneHeight), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
        mostrarFiltros(filtro_texto, materiaSelecionada, habilidadeSelecionada, g_availableSubjects, g_availableSkills);
        ImGui::EndChild();

        ImGui::SameLine(0.0f, style_loop.ItemSpacing.x);

        // Para alinhar o JogosPane com o FiltrosPane na mesma altura Y:
        ImGui::SetCursorPosY(panelsStartY);

        ImGui::BeginChild("JogosPane", ImVec2(ImGui::GetContentRegionAvail().x - overall_margin, availablePaneHeight), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
        ImGui::Text("Jogos Disponíveis"); ImGui::Separator();

        float card_fixed_width_for_layout = 290.0f;
        int displayed_games_count = 0;
        int cards_in_current_row = 0;
        for (const auto& game : games) {
            bool filtroMateriaOk = (materiaSelecionada == "Todas") || (game.cfg.subject == materiaSelecionada);
            bool filtroHabilidadeOk = (habilidadeSelecionada == "Todas") || (std::find(game.cfg.skills.begin(), game.cfg.skills.end(), habilidadeSelecionada) != game.cfg.skills.end());
            std::string lowerFiltro(filtro_texto); std::transform(lowerFiltro.begin(), lowerFiltro.end(), lowerFiltro.begin(), [](unsigned char c){ return std::tolower(c); });
            std::string lowerDesc(game.cfg.description); std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), [](unsigned char c){ return std::tolower(c); });
            std::string lowerSubj(game.cfg.subject); std::transform(lowerSubj.begin(), lowerSubj.end(), lowerSubj.begin(), [](unsigned char c){ return std::tolower(c); });
            bool filtroTextoOk = (strlen(filtro_texto) == 0) || (lowerDesc.find(lowerFiltro) != std::string::npos) || (lowerSubj.find(lowerFiltro) != std::string::npos);

            if (filtroMateriaOk && filtroHabilidadeOk && filtroTextoOk) {
                displayed_games_count++;
                if (cards_in_current_row > 0) {
                    float lastItemEndX = ImGui::GetItemRectMax().x;
                    float nextItemStartX = lastItemEndX + style_loop.ItemSpacing.x;
                    // O GetWindowPos().x aqui se refere ao JogosPane (janela atual do ImGui::BeginChild)
                    if ((nextItemStartX + card_fixed_width_for_layout) <= (ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x)) {
                        ImGui::SameLine();
                    } else {
                        cards_in_current_row = 0;
                    }
                }
                mostrarCardJogo(game);
                cards_in_current_row++;
            }
        }
        if (displayed_games_count == 0) ImGui::TextWrapped("Nenhum jogo encontrado com os filtros atuais.");
        ImGui::EndChild(); // JogosPane

        ImGui::End(); // JanelaPrincipal

        mostrarBarraStatus();

        glViewport(0, 0, display_w, display_h);
        if (background_texture_id == 0) {
            glClearColor(clear_color_fallback.x * clear_color_fallback.w, clear_color_fallback.y * clear_color_fallback.w, clear_color_fallback.z * clear_color_fallback.w, clear_color_fallback.w);
        }
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

// --- Ponto de Entrada ---
int main(int argc, char** argv) {
    (void)argc; (void)argv;
    if (!glfwInit()) { std::cerr << "ERRO CRÍTICO: Falha ao inicializar GLFW!" << std::endl; return EXIT_FAILURE; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2); glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Projeto Jardim - Interface Educativa CHAI3D", nullptr, nullptr);
    if (!window) { std::cerr << "ERRO CRÍTICO: Falha ao criar janela GLFW!" << std::endl; glfwTerminate(); return EXIT_FAILURE; }
    glfwMakeContextCurrent(window); glfwSwapInterval(1);
    glewExperimental = GL_TRUE; GLenum glewError = glewInit();
    if (glewError != GLEW_OK) { std::cerr << "ERRO CRÍTICO: Falha ao inicializar GLEW! " << glewGetErrorString(glewError) << std::endl; glfwDestroyWindow(window); glfwTerminate(); return EXIT_FAILURE; }
    glGetError();
    if (!inicializarSistema()) { std::cerr << "ERRO CRÍTICO: Falha na inicialização do sistema." << std::endl; glfwDestroyWindow(window); glfwTerminate(); return EXIT_FAILURE; }
    criarInterface(window);
    executarLoop(window);
    if (background_texture_id != 0) glDeleteTextures(1, &background_texture_id);
    ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext();
    glfwDestroyWindow(window); glfwTerminate();
#if DISABLE_HAPTICS == 0
    // Haptics::shutdown();
#else
    // HapticSimulator::shutdown();
#endif
    std::cout << "Aplicação finalizada." << std::endl;
    return EXIT_SUCCESS;
}
