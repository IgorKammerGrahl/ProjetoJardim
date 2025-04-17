#ifndef HAPTICS_H
#define HAPTICS_H

#include "chai3d.h"
#include <thread>
#include <atomic>
#include <mutex>

// Namespace para organizar o código háptico
namespace Haptics {

// --- Variáveis Compartilhadas ---
// Para controlar a execução da thread háptica
extern std::atomic<bool> simulationRunning;
// Para saber se o dispositivo foi inicializado com sucesso
extern std::atomic<bool> hapticDeviceReady;
// Posição atual do dispositivo (lida pela thread háptica, usada pela thread gráfica)
extern chai3d::cVector3d hapticDevicePosition;
// Mutex para proteger o acesso à hapticDevicePosition
extern std::mutex positionMutex;
// (Opcional) Posição da esfera (definida pela thread gráfica, usada pela háptica para calcular força)
extern chai3d::cVector3d spherePosition;
extern double sphereRadius;
extern std::mutex sphereMutex;


// --- Funções ---
// Função principal da thread háptica
void hapticsThreadLoop();

// Inicia a thread háptica
bool startHaptics();

// Para a thread háptica
void stopHaptics();

// Obtém a posição do dispositivo de forma segura (thread-safe)
chai3d::cVector3d getDevicePosition();

// Define a posição/raio da esfera de forma segura (thread-safe)
void setSphereProperties(const chai3d::cVector3d& pos, double radius);

} // namespace Haptics

#endif // HAPTICS_H
