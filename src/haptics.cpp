#include "haptics.h"
#include "chai3d.h"
#include <iostream>
#include <memory> // <--- NECESSÁRIO PARA shared_ptr (cGenericHapticDevicePtr)

// Usa o namespace chai3d para simplificar
using namespace chai3d;

namespace Haptics {

// --- Definição das Variáveis Compartilhadas ---
std::atomic<bool> simulationRunning(false);
std::atomic<bool> hapticDeviceReady(false);
cVector3d hapticDevicePosition(0, 0, 0);
std::mutex positionMutex;
cVector3d spherePosition(0, 0, 0);
double sphereRadius = 0.0;
std::mutex sphereMutex;

// --- Variáveis Internas da Thread Háptica ---
// CORRIGIDO: Usar o tipo correto (shared_ptr)
cGenericHapticDevicePtr hapticDevice = nullptr;
cHapticDeviceHandler* handler = nullptr;
std::thread hapticsThread;

// --- Implementação das Funções ---

void hapticsThreadLoop() {
    std::cout << "Haptics thread started." << std::endl;

    // Criar e obter o dispositivo
    handler = new cHapticDeviceHandler();
    // CORRIGIDO: Passar hapticDevice (que agora é cGenericHapticDevicePtr)
    if (!handler->getDevice(hapticDevice, 0)) {
        std::cerr << "HAPTICS ERROR: Failed to get haptic device." << std::endl;
        hapticDeviceReady = false;
        simulationRunning = false;
        delete handler;
        handler = nullptr;
        return;
    }

    // Abrir conexão
    // CORRIGIDO: Checar retorno booleano
    if (!hapticDevice->open()) {
        std::cerr << "HAPTICS ERROR: Failed to open connection to haptic device." << std::endl;
        hapticDeviceReady = false;
        simulationRunning = false;
        delete handler;
        handler = nullptr;
        hapticDevice = nullptr; // Libera o shared_ptr
        return;
    }

    // Calibrar (opcional)
    // hapticDevice->calibrate();

    hapticDeviceReady = true;
    std::cout << "Haptic device ready: " << hapticDevice->getSpecifications().m_modelName << std::endl;

    double Kp_wall = 20.0;
    double Bp_wall = 0.5;
    cVector3d currentPosition(0, 0, 0);
    cVector3d currentVelocity(0, 0, 0);
    cVector3d force(0, 0, 0);
    cVector3d localSpherePos(0,0,0);
    double localSphereRadius = 0.0;

    while (simulationRunning) {
        // CORRIGIDO: Usar if(hapticDevice) para checar se o ponteiro é válido
        if (!hapticDeviceReady || !hapticDevice) break;

        // 1. Ler estado do dispositivo
        // CORRIGIDO: Checar retorno booleano
        if (!hapticDevice->getPosition(currentPosition) ||
            !hapticDevice->getLinearVelocity(currentVelocity)) {
            std::cerr << "HAPTICS WARNING: Failed to read device state." << std::endl;
            // Não necessariamente fatal, apenas zera a força por segurança
            force.zero();
            // Tenta enviar força zero mesmo em caso de erro de leitura parcial
            hapticDevice->setForce(force);
            continue;
        }

        // 2. Atualizar posição compartilhada
        {
            std::lock_guard<std::mutex> lock(positionMutex);
            hapticDevicePosition = currentPosition;
        }

        // 3. Obter propriedades da esfera
        {
            std::lock_guard<std::mutex> lock(sphereMutex);
            localSpherePos = spherePosition;
            localSphereRadius = sphereRadius;
        }

        // 4. Calcular Força
        force.zero();
        if (localSphereRadius > 0) {
            cVector3d vecSphereToDevice = currentPosition - localSpherePos;
            double distance = vecSphereToDevice.length();

            if (distance < localSphereRadius) {
                double penetrationDistance = localSphereRadius - distance;
                cVector3d forceDirection = vecSphereToDevice;
                if (distance > C_SMALL) {
                    forceDirection.normalize();
                } else {
                    forceDirection.set(0, 0, 1);
                }
                cVector3d springForce = Kp_wall * penetrationDistance * forceDirection;
                double dampingFactor = cDot(currentVelocity, forceDirection);
                cVector3d dampingForce = -Bp_wall * dampingFactor * forceDirection;
                force = springForce + dampingForce;
            }
        }

        // 5. Enviar Força
        // CORRIGIDO: Checar retorno booleano
        if (!hapticDevice->setForce(force)) {
            std::cerr << "HAPTICS WARNING: Failed to send force to device." << std::endl;
        }

        // cSleepMs(1); // Geralmente não necessário, o driver gerencia o tempo
    }

    // --- Limpeza ---
    std::cout << "Stopping haptics thread..." << std::endl;
    hapticDeviceReady = false;
    // CORRIGIDO: Checar ponteiro antes de usar
    if (hapticDevice) {
        cVector3d zeroForce(0,0,0);
        hapticDevice->setForce(zeroForce); // Tenta zerar a força
        hapticDevice->close();
        std::cout << "Haptic device connection closed." << std::endl;
        hapticDevice = nullptr; // Libera o shared_ptr
    }
    if (handler != nullptr) {
        delete handler;
        handler = nullptr;
        std::cout << "Haptic device handler deleted." << std::endl;
    }
}

bool startHaptics() {
    if (simulationRunning) {
        std::cout << "Haptics already running." << std::endl;
        return true;
    }
    simulationRunning = true;
    hapticsThread = std::thread(hapticsThreadLoop);
    return true;
}

void stopHaptics() {
    if (!simulationRunning) return;
    simulationRunning = false;
    if (hapticsThread.joinable()) {
        hapticsThread.join();
        std::cout << "Haptics thread joined." << std::endl;
    }
}

cVector3d getDevicePosition() {
    std::lock_guard<std::mutex> lock(positionMutex);
    return hapticDevicePosition;
}

void setSphereProperties(const cVector3d& pos, double radius) {
    std::lock_guard<std::mutex> lock(sphereMutex);
    spherePosition = pos;
    sphereRadius = radius;
}

} // namespace Haptics
