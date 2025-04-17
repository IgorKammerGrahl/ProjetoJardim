// 1. Instruir GLFW a NÃO incluir headers GL padrão
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// 2. Incluir GLEW DEPOIS de GLFW (e depois de definir GLFW_INCLUDE_NONE)
#include <GL/glew.h>

#include <iostream>
#include <chrono>
#include <thread>
#include "chai3d.h" // Inclui a maioria das classes CHAI3D, como cShapeSphere
#include "lighting/CPositionalLight.h" // Para a luz
#include "haptics.h" // Nosso código háptico

// Use namespaces se preferir
using namespace chai3d;
// using namespace std; // Opcional

int main(int argc, char* argv[]) {

    // ****** Fator de Escala REATIVADO ******
    // Ajuste este valor PEQUENO (ex: 2.0 a 5.0) para equilibrar
    // liberdade visual e sincronia com o háptico.
    const double workspaceScaleFactor = 3.0; // Ou outro valor que você prefira
    // **************************************

    // --- Inicialização GLFW e GLEW ---
    if (!glfwInit()) {
        std::cerr << "Erro ao inicializar GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Jardim Sensorial Virtual (CHAI3D + Falcon)", NULL, NULL);
    if (!window) {
        std::cerr << "Erro ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-Sync
    if (glewInit() != GLEW_OK) {
        std::cerr << "Erro ao inicializar GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;


    // --- Configuração CHAI3D ---
    cWorld* world = new cWorld();
    cCamera* camera = new cCamera(world);
    world->addChild(camera);
    // Posição da câmera
    camera->set(cVector3d(1.5, 0.0, 0.8),    // Mantendo a câmera de antes
                cVector3d(0.0, 0.0, 0.0),    // Olhando para a origem
                cVector3d(0.0, 0.0, 1.0));
    camera->setFieldViewAngleDeg(45.0);
    camera->setClippingPlanes(0.01, 10.0);
    // Luz
    cPositionalLight* light = new cPositionalLight(world);
    world->addChild(light);
    light->setEnabled(true);
    light->setLocalPos(2.0, 3.0, 4.0);
    // Configs GL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);


    // --- Objetos da Cena ---
    // Esfera de interação (em metros)
    cShapeSphere* interactionSphere = new cShapeSphere(0.1); // Raio 10cm
    interactionSphere->m_material->setRedFireBrick();
    world->addChild(interactionSphere);
    // Posição da esfera (ex: 0.5 no eixo X)
    interactionSphere->setLocalPos(0.5, 0.0, 0.0);

    // Cursor visual (esfera branca)
    cShapeSphere* hapticCursor = new cShapeSphere(0.015); // Raio 1.5cm
    world->addChild(hapticCursor);
    hapticCursor->m_material->setWhite();
    hapticCursor->setUseMaterial(true);
    hapticCursor->setUseTransparency(false);


    // --- Inicialização Háptica ---
    if (!Haptics::startHaptics()) {
        std::cerr << "MAIN: Failed to start haptics thread. Exiting." << std::endl;
        delete world; glfwDestroyWindow(window); glfwTerminate(); return -1;
    }
    // Espera Inteligente
    std::cout << "MAIN: Waiting for haptic device to become ready..." << std::endl;
    bool deviceReady = false;
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    std::chrono::seconds timeout(5);
    while (std::chrono::steady_clock::now() - startTime < timeout) {
        if (Haptics::hapticDeviceReady.load()) { deviceReady = true; break; }
        if (!Haptics::simulationRunning.load()) { std::cerr << "MAIN: Haptics thread indicated an early stop during initialization." << std::endl; break; }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (!deviceReady) {
        if (Haptics::simulationRunning.load()) { std::cerr << "MAIN: Timeout waiting for haptic device. Exiting." << std::endl; }
        else { std::cerr << "MAIN: Haptic device failed to initialize (check haptics thread logs). Exiting." << std::endl; }
        Haptics::stopHaptics();
        delete world; glfwDestroyWindow(window); glfwTerminate(); return -1;
    }
    std::cout << "MAIN: Haptic device is ready!" << std::endl;


    // --- Loop Principal de Renderização e Lógica ---
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // --- Atualizações Lógicas ---

        // 1. Obter a posição REAL do dispositivo (em metros)
        cVector3d devicePos = Haptics::getDevicePosition();

        // 2. ****** REINTRODUZIDO: Aplicar escala APENAS para o cursor VISUAL ******
        //    Multiplica a posição real pelo fator de escala para ampliar o movimento visual.
        cVector3d visualCursorPos = devicePos * workspaceScaleFactor; // Usa o fator de escala (ex: 3.0)
        hapticCursor->setLocalPos(visualCursorPos); // Define a posição do cursor visual com a escala
        // **************************************************************************

        // 3. Informar à thread háptica a posição/raio da esfera (em metros, SEM escala)
        //    A física háptica SEMPRE usa as coordenadas reais do mundo.
        Haptics::setSphereProperties(interactionSphere->getGlobalPos(), interactionSphere->getRadius());


        // --- Renderização ---
        if (width > 0 && height > 0) {
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Renderiza a cena
            camera->renderView(width, height);
            glfwSwapBuffers(window);
        }
        glfwPollEvents();
    }

    // --- Limpeza Final ---
    std::cout << "MAIN: Exiting application..." << std::endl;
    Haptics::stopHaptics();
    delete world;
    world = nullptr;
    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "MAIN: Cleanup complete." << std::endl;

    return 0;
}
