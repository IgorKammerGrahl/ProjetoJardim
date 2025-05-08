cmake_minimum_required(VERSION 3.5)
project(MeuProjetoChai3D)

option(DISABLE_HAPTICS "Disable haptic device support" OFF)

# Configurações básicas do projeto
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wno-ignored-attributes -Wno-deprecated-declarations)

# Política para tratamento de OpenGL
cmake_policy(SET CMP0072 NEW)

# Configuração do ImGui
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/extern/imgui")
if(NOT EXISTS ${IMGUI_DIR})
    message(FATAL_ERROR "ImGui não encontrado. Execute:\ngit clone https://github.com/ocornut/imgui.git extern/imgui")
endif()

# Configuração de bibliotecas base
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

# Configuração do JSONCPP
find_path(JSONCPP_INCLUDE_DIR
    NAMES json/json.h
    PATHS
    /usr/include
    /usr/include/jsoncpp
    /usr/local/include
)
find_library(JSONCPP_LIBRARY
    NAMES jsoncpp
    PATHS
    /usr/lib
    /usr/lib/x86_64-linux-gnu
    /usr/local/lib
)

if(JSONCPP_INCLUDE_DIR AND JSONCPP_LIBRARY)
    message(STATUS "JSONCPP encontrado:")
    message(STATUS "  Includes: ${JSONCPP_INCLUDE_DIR}")
    message(STATUS "  Biblioteca: ${JSONCPP_LIBRARY}")
    include_directories(${JSONCPP_INCLUDE_DIR})
else()
    message(FATAL_ERROR "JSONCPP não encontrado! Instale com:\n  sudo apt-get install libjsoncpp-dev")
endif()

# Configuração condicional do Haptic
if(NOT DISABLE_HAPTICS)
    message(STATUS "Haptic support enabled")

    # Configurações do CHAI3D
    set(CHAI3D_ROOT "/home/igor/chai3d-3.2.0-Makefiles/chai3d-3.2.0")
    if(NOT EXISTS ${CHAI3D_ROOT})
        message(FATAL_ERROR "CHAI3D_ROOT não encontrado em: ${CHAI3D_ROOT}")
    endif()

    # Configurações do Force Dimension SDK
    set(FD_SDK_ROOT "/home/igor/sdk-3.17.6")
    if(NOT EXISTS ${FD_SDK_ROOT})
        message(FATAL_ERROR "FD_SDK_ROOT não encontrado em: ${FD_SDK_ROOT}")
    endif()

    include_directories(
        ${CHAI3D_ROOT}/src
        ${CHAI3D_ROOT}/external/Eigen
        ${FD_SDK_ROOT}/include
    )

    # Buscar bibliotecas do Force Dimension
    set(FD_SDK_LIB_DIR "${FD_SDK_ROOT}/lib/release/lin-x86_64-gcc")
    find_library(DHD_LIBRARY NAMES dhd PATHS ${FD_SDK_LIB_DIR} REQUIRED)
    find_library(DRD_LIBRARY NAMES drd PATHS ${FD_SDK_LIB_DIR} REQUIRED)

    add_definitions(-DUSE_HAPTICS)
else()
    message(STATUS "Haptic support disabled")
    add_definitions(-DDISABLE_HAPTICS)
endif()

# Configuração do ImGui
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})

add_library(imgui_impl_glfw STATIC ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp)
target_link_libraries(imgui_impl_glfw PRIVATE imgui ${GLFW_LIBRARIES})
target_include_directories(imgui_impl_glfw PUBLIC ${IMGUI_DIR}/backends)

add_library(imgui_impl_opengl3 STATIC ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
target_link_libraries(imgui_impl_opengl3 PRIVATE imgui OpenGL::GL)
target_include_directories(imgui_impl_opengl3 PUBLIC ${IMGUI_DIR}/backends)

# Configuração do executável principal
add_executable(MeuProjetoChai3D
    src/main.cpp
    src/config_parser.cpp
)

# Arquivos condicionais
if(DISABLE_HAPTICS)
    target_sources(MeuProjetoChai3D PRIVATE src/haptic_simulator.cpp)
    target_include_directories(MeuProjetoChai3D PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
else()
    target_sources(MeuProjetoChai3D PRIVATE src/haptics.cpp)
endif()

# Linkagem comum
target_link_libraries(MeuProjetoChai3D
    PRIVATE
    ${GLFW_LIBRARIES}
    GLEW::GLEW
    OpenGL::GL
    X11
    dl
    pthread
    imgui
    imgui_impl_glfw
    imgui_impl_opengl3
    ${JSONCPP_LIBRARY}
    ${LIBUSB_LIBRARIES}
)

include_directories(${JSONCPP_INCLUDE_DIR})  # Adicionado

# Linkagem condicional para haptics
if(NOT DISABLE_HAPTICS)
    target_link_libraries(MeuProjetoChai3D
        PRIVATE
        ${CHAI3D_ROOT}/lib/release/lin-x86_64-cc/libchai3d.a
        ${DHD_LIBRARY}
        ${DRD_LIBRARY}
    )

    target_link_directories(MeuProjetoChai3D PRIVATE ${FD_SDK_LIB_DIR})
endif()

# Configurar arquivo de configuração
configure_file(
    ${CMAKE_SOURCE_DIR}/src/games_config.json
    ${CMAKE_BINARY_DIR}/games_config.json
    COPYONLY
)

# Mensagem final
message(STATUS "\n==============================================")
message(STATUS " Configuração concluída com sucesso!")
message(STATUS " Haptic support: ${DISABLE_HAPTICS}")
message(STATUS "==============================================\n")
