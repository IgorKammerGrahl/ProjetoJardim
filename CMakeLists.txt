cmake_minimum_required(VERSION 3.5)
project(MeuProjetoChai3D)

# --- Opções do Projeto ---
option(DISABLE_HAPTICS "Disable haptic device support" OFF)
option(IMGUI_INCLUDE_DEMO "Include ImGui demo window sources" OFF) # Opção para incluir o demo

# --- Configurações Básicas do Projeto ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wno-ignored-attributes -Wno-deprecated-declarations)

# Política para tratamento de OpenGL
cmake_policy(SET CMP0072 NEW)

# Diretório de saída para o executável
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# --- Configuração do ImGui ---
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/extern/imgui") # Assumindo que ImGui está como submódulo ou clonado aqui
if(NOT EXISTS ${IMGUI_DIR})
    message(FATAL_ERROR "ImGui não encontrado em ${IMGUI_DIR}. Execute:\ngit clone https://github.com/ocornut/imgui.git extern/imgui")
endif()

# --- Busca de Dependências Base ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0) # Necessário para alguns dispositivos hápticos
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

# --- Configuração do JSONCPP ---
find_path(JSONCPP_INCLUDE_DIR NAMES json/json.h PATHS /usr/include /usr/include/jsoncpp /usr/local/include)
find_library(JSONCPP_LIBRARY NAMES jsoncpp PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)

if(NOT JSONCPP_INCLUDE_DIR OR NOT JSONCPP_LIBRARY)
    message(FATAL_ERROR "JSONCPP não encontrado! Instale com (ex: Debian/Ubuntu):\n  sudo apt-get install libjsoncpp-dev")
else()
    message(STATUS "JSONCPP encontrado:")
    message(STATUS "  Includes: ${JSONCPP_INCLUDE_DIR}")
    message(STATUS "  Biblioteca: ${JSONCPP_LIBRARY}")
endif()

# --- Configuração Condicional para Dispositivos Hápticos ---
if(NOT DISABLE_HAPTICS)
    message(STATUS "Haptic support ENABLED")

    # Tenta ler de variáveis de ambiente, depois usa opções CMake, com fallback para defaults
    set(CHAI3D_ROOT_DEFAULT "/home/igor/chai3d-3.2.0-Makefiles/chai3d-3.2.0")
    set(FD_SDK_ROOT_DEFAULT "/home/igor/sdk-3.17.6")

    if($ENV{CHAI3D_ROOT})
        set(CHAI3D_ROOT_DEFAULT $ENV{CHAI3D_ROOT})
    endif()
    if($ENV{FD_SDK_ROOT})
        set(FD_SDK_ROOT_DEFAULT $ENV{FD_SDK_ROOT})
    endif()

    set(CHAI3D_ROOT "${CHAI3D_ROOT_DEFAULT}" CACHE PATH "Root directory of CHAI3D")
    set(FD_SDK_ROOT "${FD_SDK_ROOT_DEFAULT}" CACHE PATH "Root directory of Force Dimension SDK")

    if(NOT EXISTS "${CHAI3D_ROOT}")
        message(FATAL_ERROR "CHAI3D_ROOT não encontrado em: ${CHAI3D_ROOT}. \nConfigure com -DCHAI3D_ROOT=/path/to/chai3d ou defina a variável de ambiente CHAI3D_ROOT.")
    endif()
    if(NOT EXISTS "${FD_SDK_ROOT}")
        message(FATAL_ERROR "FD_SDK_ROOT não encontrado em: ${FD_SDK_ROOT}. \nConfigure com -DFD_SDK_ROOT=/path/to/fdsdk ou defina a variável de ambiente FD_SDK_ROOT.")
    endif()

    set(FD_SDK_LIB_DIR "${FD_SDK_ROOT}/lib/release/lin-x86_64-gcc")
    find_library(DHD_LIBRARY NAMES dhd PATHS "${FD_SDK_LIB_DIR}" REQUIRED)
    find_library(DRD_LIBRARY NAMES drd PATHS "${FD_SDK_LIB_DIR}" REQUIRED)

    if(NOT DHD_LIBRARY OR NOT DRD_LIBRARY)
        message(FATAL_ERROR "Bibliotecas DHD ou DRD não encontradas em ${FD_SDK_LIB_DIR}")
    endif()

else()
    message(STATUS "Haptic support DISABLED")
endif()

# --- Construção das Bibliotecas ImGui ---
list(APPEND IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
)
if(IMGUI_INCLUDE_DEMO)
    list(APPEND IMGUI_SOURCES "${IMGUI_DIR}/imgui_demo.cpp")
endif()

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC "${IMGUI_DIR}")

add_library(imgui_impl_glfw STATIC "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp")
target_include_directories(imgui_impl_glfw PUBLIC "${IMGUI_DIR}/backends")
target_link_libraries(imgui_impl_glfw PRIVATE imgui ${GLFW_LIBRARIES})

add_library(imgui_impl_opengl3 STATIC "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp")
target_include_directories(imgui_impl_opengl3 PUBLIC "${IMGUI_DIR}/backends")
target_link_libraries(imgui_impl_opengl3 PRIVATE imgui OpenGL::GL)


# --- Configuração do Executável Principal ---
add_executable(MeuProjetoChai3D
    src/main.cpp
    src/config_parser.cpp
)

# Definições de compilação e includes específicos do target
target_include_directories(MeuProjetoChai3D PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"    # Para haptic_simulator.h/haptics.h e config_parser.h
    "${JSONCPP_INCLUDE_DIR}"
    "${CMAKE_SOURCE_DIR}/extern/stb"     # <--- ADICIONADO: Para encontrar stb_image.h
)

if(DISABLE_HAPTICS)
    target_sources(MeuProjetoChai3D PRIVATE src/haptic_simulator.cpp)
    target_compile_definitions(MeuProjetoChai3D PRIVATE DISABLE_HAPTICS=1)
else()
    target_sources(MeuProjetoChai3D PRIVATE src/haptics.cpp)
    target_compile_definitions(MeuProjetoChai3D PRIVATE DISABLE_HAPTICS=0 USE_HAPTICS) # USE_HAPTICS é opcional se DISABLE_HAPTICS=0 já é suficiente

    # Includes específicos para quando hápticos estão habilitados
    target_include_directories(MeuProjetoChai3D PRIVATE
        "${CHAI3D_ROOT}/src"
        "${CHAI3D_ROOT}/external/Eigen" # Eigen é frequentemente usado por CHAI3D
        "${FD_SDK_ROOT}/include"
    )
endif()

# --- Linkagem de Bibliotecas ---
target_link_libraries(MeuProjetoChai3D PRIVATE
    ${GLFW_LIBRARIES}
    GLEW::GLEW
    OpenGL::GL
    ${LIBUSB_LIBRARIES}
    ${JSONCPP_LIBRARY}
    imgui
    imgui_impl_glfw
    imgui_impl_opengl3
    # Bibliotecas do sistema comuns no Linux
    X11 # Geralmente necessário para GLFW no X11
    dl
    pthread
)

if(NOT DISABLE_HAPTICS)
    target_link_libraries(MeuProjetoChai3D PRIVATE
        "${CHAI3D_ROOT}/lib/release/lin-x86_64-cc/libchai3d.a" # Verifique este caminho exato
        ${DHD_LIBRARY}
        ${DRD_LIBRARY}
    )
endif()


# --- Comandos Pós-Build ---

# Copiar arquivo de configuração de jogos
configure_file(
    "${CMAKE_SOURCE_DIR}/src/games_config.json"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/games_config.json"
    COPYONLY
)

# Copiar pasta de fontes (assumindo que está em ${CMAKE_SOURCE_DIR}/fonts)
if(EXISTS "${CMAKE_SOURCE_DIR}/fonts")
    add_custom_command(TARGET MeuProjetoChai3D POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/fonts" "$<TARGET_FILE_DIR:MeuProjetoChai3D>/fonts"
        COMMENT "Copiando fontes para o diretório do executável"
        VERBATIM
    )
else()
    message(WARNING "Pasta de fontes não encontrada em ${CMAKE_SOURCE_DIR}/fonts. As fontes podem não carregar.")
endif()

# <--- ADICIONADO: Copiar pasta de assets (para a imagem de fundo) ---
if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    add_custom_command(TARGET MeuProjetoChai3D POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:MeuProjetoChai3D>/assets"
        COMMENT "Copiando assets para o diretório do executável"
        VERBATIM
    )
else()
    message(WARNING "Pasta de assets não encontrada em ${CMAKE_SOURCE_DIR}/assets. A imagem de fundo pode não carregar.")
endif()


# --- Mensagens Finais ---
message(STATUS "\n==============================================")
message(STATUS " Configuração do MeuProjetoChai3D concluída!")
if(DISABLE_HAPTICS)
    message(STATUS " Suporte a Hápticos: DESABILITADO")
else()
    message(STATUS " Suporte a Hápticos: HABILITADO")
    message(STATUS "   CHAI3D Root: ${CHAI3D_ROOT}")
    message(STATUS "   FD SDK Root: ${FD_SDK_ROOT}")
endif()
message(STATUS " Para incluir a demo do ImGui, use: -DIMGUI_INCLUDE_DEMO=ON")
message(STATUS "==============================================\n")
