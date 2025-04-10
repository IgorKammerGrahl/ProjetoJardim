// 1. Instruir GLFW a NÃO incluir headers GL padrão
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// 2. Incluir GLEW DEPOIS de GLFW (e depois de definir GLFW_INCLUDE_NONE)
#include <GL/glew.h>

#include <iostream>
#include "chai3d.h" // Header principal do CHAI3D

// 3. Incluir header específico da LUZ POSICIONAL
#include "lighting/CPositionalLight.h" // <--- CORRIGIDO

// Use namespaces se preferir, ou use chai3d:: explicitamente
using namespace chai3d;
// using namespace std; // Opcional

int main(int argc, char* argv[]) {
    // ... (código de inicialização GLFW, GLEW igual a antes) ...
    // Inicializar GLFW
    if (!glfwInit()) {
        std::cerr << "Erro ao inicializar GLFW" << std::endl;
        return -1;
    }

    // Configurar GLFW para OpenGL 2.1 (Compatível com CHAI3D 3.2.0) - CORRETO!
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE); // Use ANY ou COMPATIBILITY

    // Criar janela
    GLFWwindow* window = glfwCreateWindow(800, 600, "Meu Projeto CHAI3D", NULL, NULL);
    if (!window) {
        std::cerr << "Erro ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Contexto OpenGL - PRECISA VIR ANTES DE glewInit()!
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Opcional: Habilita V-Sync

    // Inicializar GLEW - PRECISA VIR DEPOIS de MakeContextCurrent!
    // glewExperimental = GL_TRUE; // Pode ser necessário, mas tente sem primeiro
    if (glewInit() != GLEW_OK) {
        std::cerr << "Erro ao inicializar GLEW" << std::endl;
        glfwDestroyWindow(window); // Limpeza parcial em caso de erro
        glfwTerminate();
        return -1;
    }

    // Imprimir versão OpenGL para depuração (opcional)
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;


    // --- Configuração CHAI3D ---

    // Criar mundo e câmera
    cWorld* world = new cWorld();
    cCamera* camera = new cCamera(world);
    world->addChild(camera); // Adiciona a câmera ao mundo

    // Posicionar câmera (API CORRETA para 3.2.0)
    camera->set( cVector3d(3.0, 0.0, 0.0),    // Posição da câmera (olhando para a origem)
                cVector3d(0.0, 0.0, 0.0),    // Ponto para onde a câmera olha (lookAt)
                cVector3d(0.0, 0.0, 1.0) );  // Vetor 'up' (Z aponta para cima)

    // Configurar projeção (MÉTODOS CORRETOS para 3.2.0)
    camera->setFieldViewAngleDeg(45.0);       // Campo de visão vertical (implica perspectiva)
    camera->setClippingPlanes(0.01, 100.0);   // Planos near/far
    // camera->setPerspectiveMode(); // <-- Certifique-se que esta linha FOI REMOVIDA!

    // Criar e adicionar esfera vermelha
    cShapeSphere* sphere = new cShapeSphere(0.5); // Raio 0.5
    sphere->m_material->setRedFireBrick();
    world->addChild(sphere); // Adiciona a esfera ao mundo

    // Adicionar uma luz POSICIONAL (importante para visualização)
    cPositionalLight* light = new cPositionalLight(world); // <--- CORRIGIDO
    world->addChild(light);
    light->setEnabled(true);                   // Habilita a luz
    light->setLocalPos(2.0, 3.0, 4.0);      // Define a posição da luz
    // light->setDir(cVector3d(-1.0, -1.0, -1.0)); // <--- REMOVIDO/COMENTADO (não essencial para CPositionalLight)

    // Configurar propriedades de renderização OpenGL (CHAI3D pode sobrescrever algumas)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING); // Habilita iluminação (CHAI3D usa iluminação GL legada)
    glEnable(GL_LIGHT0);   // Habilita a luz (CHAI3D mapeia cLight para luzes GL, geralmente começa com LIGHT0)
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f); // Cor de fundo um pouco diferente

    // Loop de renderização
    while (!glfwWindowShouldClose(window)) {
        // ... (código do loop de renderização igual a antes) ...
        // Obter tamanho do framebuffer (pode mudar se a janela for redimensionada)
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // Definir viewport CADA frame para lidar com redimensionamento
        if (width > 0 && height > 0) { // Evita divisão por zero se minimizado
            glViewport(0, 0, width, height);

            // Limpar buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Renderizar a cena vista pela câmera
            // Passar width e height atualizados!
            camera->renderView(width, height);

            // Trocar buffers (mostrar o que foi desenhado)
            glfwSwapBuffers(window);
        }

        // Processar eventos (teclado, mouse, etc.)
        glfwPollEvents();
    }

    // --- Limpeza ---

    // Deletar o mundo CHAI3D (isso deve deletar a câmera, esfera e luz adicionadas como filhas)
    delete world;
    world = nullptr; // Boa prática após delete

    // Limpeza GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
