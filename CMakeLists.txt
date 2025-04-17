cmake_minimum_required(VERSION 3.5)
project(MeuProjetoChai3D)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wno-ignored-attributes -Wno-deprecated-declarations)

# --- CAMINHO CHAI3D ---
set(CHAI3D_ROOT "/home/igor/chai3d-3.2.0-Makefiles/chai3d-3.2.0") # CONFIRME ESTE CAMINHO!
if(NOT EXISTS ${CHAI3D_ROOT})
    message(FATAL_ERROR "CHAI3D_ROOT não encontrado em: ${CHAI3D_ROOT}")
endif()

# --- CAMINHO DO SDK DA FORCE DIMENSION (DHD) ---
# Defina o caminho raiz do SDK que você baixou
set(FD_SDK_ROOT "/home/igor/sdk-3.17.6") # <--- AJUSTE SE O CAMINHO FOR DIFERENTE
if(NOT EXISTS ${FD_SDK_ROOT})
    message(FATAL_ERROR "Diretório raiz do SDK da Force Dimension (FD_SDK_ROOT) não encontrado em: ${FD_SDK_ROOT}")
endif()
# Construa os caminhos para as bibliotecas e includes dentro do SDK
set(FD_SDK_LIB_DIR "${FD_SDK_ROOT}/lib/release/lin-x86_64-gcc") # <--- Caminho exato onde libdhd está
set(FD_SDK_INCLUDE_DIR "${FD_SDK_ROOT}/include") # <--- Caminho provável para os headers
if(NOT EXISTS ${FD_SDK_LIB_DIR})
    message(FATAL_ERROR "Diretório de bibliotecas do SDK DHD não encontrado em: ${FD_SDK_LIB_DIR}")
endif()
if(NOT EXISTS ${FD_SDK_INCLUDE_DIR})
    message(FATAL_ERROR "Diretório de includes do SDK DHD não encontrado em: ${FD_SDK_INCLUDE_DIR}")
endif()
# -------------------------------------------------

# Includes
include_directories(
    ${FD_SDK_INCLUDE_DIR}   # <--- ADICIONADO: Headers do SDK DHD
    ${CHAI3D_ROOT}/src
    ${CHAI3D_ROOT}/external/Eigen
    src                     # Diretório dos seus fontes (main.cpp, haptics.cpp, haptics.h)
)

# GLFW (via pkg-config)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS}) # Já adiciona os headers do GLFW

# GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS}) # Já adiciona os headers do GLEW

# LibUSB (Dependência comum para drivers hápticos)
pkg_check_modules(LIBUSB libusb-1.0 QUIET)
if(LIBUSB_FOUND)
    include_directories(${LIBUSB_INCLUDE_DIRS})
    set(USB_LIBRARIES ${LIBUSB_LIBRARIES})
    message(STATUS "Found LibUSB 1.0 via pkg-config")
else()
    find_library(LIBUSB_LIBRARY NAMES usb-1.0 PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
    if(LIBUSB_LIBRARY)
        set(USB_LIBRARIES ${LIBUSB_LIBRARY})
        include_directories(/usr/include/libusb-1.0) # Palpite comum para headers se find_library for usado
        message(STATUS "Found LibUSB 1.0 library via find_library: ${LIBUSB_LIBRARY}")
    else()
        message(FATAL_ERROR "LibUSB 1.0 não encontrada. Instale o pacote 'libusb-1.0-0-dev'.")
    endif()
endif()

# --- ENCONTRAR A BIBLIOTECA DO DRIVER (dhd) ---
# Agora procuramos PRIMEIRO no diretório específico do SDK que encontramos
find_library(DHD_LIBRARY NAMES dhd # Procura por libdhd.so ou libdhd.a
             PATHS ${FD_SDK_LIB_DIR} # <--- CAMINHO ESPECÍFICO DO SDK
                   # Mantemos os caminhos padrão como fallback, embora não devam ser necessários agora
                   /usr/local/lib
                   /usr/lib
                   /usr/lib/x86_64-linux-gnu
             DOC "Force Dimension SDK Library (libdhd)")

# Verifica se a biblioteca foi encontrada (agora deve funcionar!)
if(NOT DHD_LIBRARY)
    message(FATAL_ERROR "Biblioteca DHD (libdhd) NÃO ENCONTRADA mesmo no caminho do SDK especificado: ${FD_SDK_LIB_DIR}.\n"
                        "Verifique se o caminho FD_SDK_LIB_DIR está correto e se os arquivos .so ou .a existem lá.\n"
                        "A compilação não pode prosseguir.")
else()
    message(STATUS "Found DHD library: ${DHD_LIBRARY}")
    # Também precisamos da libdrd, que geralmente está no mesmo diretório
    find_library(DRD_LIBRARY NAMES drd PATHS ${FD_SDK_LIB_DIR} DOC "Force Dimension DRD Library (libdrd)")
    if (NOT DRD_LIBRARY)
         message(WARNING "Biblioteca DRD (libdrd) não encontrada em ${FD_SDK_LIB_DIR}. Pode ser necessária pela libdhd.")
         # Defina como vazio para não quebrar a linkagem se não for encontrada
         set(DRD_LIBRARY "")
    else()
         message(STATUS "Found DRD library: ${DRD_LIBRARY}")
    endif()
endif()
# -------------------------------------------

# Adicionar os arquivos fonte do seu projeto
add_executable(MeuProjetoChai3D
    src/main.cpp
    src/haptics.cpp
)

# --- CAMINHO LIB CHAI3D ---
set(CHAI3D_LIB_PATH "${CHAI3D_ROOT}/lib/release/lin-x86_64-cc/libchai3d.a") # CONFIRME ESTE CAMINHO!
if(NOT EXISTS ${CHAI3D_LIB_PATH})
    message(FATAL_ERROR "Biblioteca CHAI3D (`libchai3d.a`) não encontrada em: ${CHAI3D_LIB_PATH}")
endif()

# --- LINKAGEM ---
target_link_libraries(MeuProjetoChai3D
    # 1. Biblioteca principal do CHAI3D
    ${CHAI3D_LIB_PATH}

    # 2. Bibliotecas do Driver DHD e DRD (encontradas no SDK)
    ${DHD_LIBRARY}  # Caminho completo para libdhd.so ou libdhd.a
    ${DRD_LIBRARY}  # Caminho completo para libdrd.so ou libdrd.a (se encontrada)

    # 3. Bibliotecas Gráficas e do Sistema
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    GL
    GLU
    X11

    # 4. Biblioteca de Threads
    pthread

    # 5. Biblioteca de carregamento dinâmico
    dl

    # 6. Biblioteca LibUSB
    ${USB_LIBRARIES}
)

# Adicionar diretório da biblioteca DHD ao RPATH
# Essencial para que o executável encontre libdhd.so e libdrd.so em tempo de execução
# Usa o diretório que definimos anteriormente (FD_SDK_LIB_DIR)
if(EXISTS ${FD_SDK_LIB_DIR})
    target_link_options(MeuProjetoChai3D PRIVATE "-Wl,-rpath,${FD_SDK_LIB_DIR}")
    message(STATUS "Adding RPATH for DHD/DRD libraries: ${FD_SDK_LIB_DIR}")
endif()

message(STATUS "CMake configuration finished. Run 'make' to build.")
