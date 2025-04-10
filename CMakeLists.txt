cmake_minimum_required(VERSION 3.5)
project(MeuProjetoChai3D)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Silencia avisos do Eigen (mantido, útil)
add_compile_options(-Wno-ignored-attributes -Wno-deprecated-declarations)

# --- VERIFIQUE ESTE CAMINHO CUIDADOSAMENTE ---
set(CHAI3D_ROOT "/home/igor/chai3d-3.2.0-Makefiles/chai3d-3.2.0") # CONFIRME!

# Verificar se o diretório raiz existe
if(NOT EXISTS ${CHAI3D_ROOT})
    message(FATAL_ERROR "CHAI3D_ROOT não encontrado em: ${CHAI3D_ROOT}")
endif()

# Includes do CHAI3D e Eigen
include_directories(
    ${CHAI3D_ROOT}/src
    ${CHAI3D_ROOT}/external/Eigen
    # ${CHAI3D_ROOT}/external/OpenGL # Provavelmente desnecessário, tente remover
)

# GLFW via pkg-config (mantido)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

# GLEW (mantido)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

# --- VERIFIQUE SE main.cpp ESTÁ EM 'src/' OU NÃO ---
add_executable(MeuProjetoChai3D src/main.cpp) # ou main.cpp se estiver na raiz

# --- VERIFIQUE ESTE CAMINHO DA BIBLIOTECA CUIDADOSAMENTE ---
set(CHAI3D_LIB_PATH "${CHAI3D_ROOT}/lib/release/lin-x86_64-cc/libchai3d.a") # CONFIRME!

# Verificar se a biblioteca existe
if(NOT EXISTS ${CHAI3D_LIB_PATH})
    message(FATAL_ERROR "Biblioteca CHAI3D não encontrada em: ${CHAI3D_LIB_PATH}")
endif()

# Linkagem (ordem crítica!)
target_link_libraries(MeuProjetoChai3D
    ${CHAI3D_LIB_PATH}  # Usar a variável definida
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    GL # Equivalente a -lGL
    GLU # Equivalente a -lGLU
    # glut # TENTE REMOVER ESTA LINHA PRIMEIRO! Equivalente a -lglut
    X11 # Equivalente a -lX11
    pthread # Equivalente a -lpthread
    dl # Equivalente a -ldl
)

# Opcional: Adicionar RPATH para encontrar libs compartilhadas (GLFW, GLEW) se necessário
# target_link_options(MeuProjetoChai3D PRIVATE "-Wl,-rpath,'$$ORIGIN'") # Exemplo
