cmake_minimum_required(VERSION 3.5)
project(MeuProjetoChai3D)

# Usar C++17 para <filesystem> e features modernas
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wno-ignored-attributes -Wno-deprecated-declarations)

# --- CAMINHO CHAI3D ---
set(CHAI3D_ROOT "/home/igor/chai3d-3.2.0-Makefiles/chai3d-3.2.0")
if(NOT EXISTS ${CHAI3D_ROOT})
    message(FATAL_ERROR "CHAI3D_ROOT não encontrado em: ${CHAI3D_ROOT}")
endif()

# --- CAMINHO DO SDK DA FORCE DIMENSION (DHD) ---
set(FD_SDK_ROOT "/home/igor/sdk-3.17.6")
if(NOT EXISTS ${FD_SDK_ROOT})
    message(FATAL_ERROR "FD_SDK_ROOT não encontrado em: ${FD_SDK_ROOT}")
endif()

set(FD_SDK_LIB_DIR "${FD_SDK_ROOT}/lib/release/lin-x86_64-gcc")
if(NOT EXISTS ${FD_SDK_LIB_DIR})
    message(FATAL_ERROR "DHD lib não encontrada em ${FD_SDK_LIB_DIR}")
endif()

set(FD_SDK_INCLUDE_DIR "${FD_SDK_ROOT}/include")
if(NOT EXISTS ${FD_SDK_INCLUDE_DIR})
    message(FATAL_ERROR "DHD include não encontrado em ${FD_SDK_INCLUDE_DIR}")
endif()

# --- Incluir fontes do projeto ---
include_directories(
    ${CHAI3D_ROOT}/src
    ${CHAI3D_ROOT}/external/Eigen
    ${FD_SDK_INCLUDE_DIR}
    src
)

# --- Dependências externas ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

pkg_check_modules(LIBUSB libusb-1.0 QUIET)
if(LIBUSB_FOUND)
    include_directories(${LIBUSB_INCLUDE_DIRS})
    set(USB_LIBRARIES ${LIBUSB_LIBRARIES})
else()
    set(USB_LIBRARIES "")
endif()

find_library(DHD_LIBRARY NAMES dhd PATHS ${FD_SDK_LIB_DIR} /usr/lib /usr/local/lib)
if(NOT DHD_LIBRARY)
    message(FATAL_ERROR "Biblioteca DHD não encontrada.")
endif()

find_library(DRD_LIBRARY NAMES drd PATHS ${FD_SDK_LIB_DIR})
if(NOT DRD_LIBRARY)
    message(FATAL_ERROR "Biblioteca DRD não encontrada.")
endif()

# --- OpenGL ---
find_package(OpenGL REQUIRED)

# --- ImGui via submódulo local ---
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/extern/imgui")
if(NOT EXISTS ${IMGUI_DIR})
    message(FATAL_ERROR "ImGui não encontrado em ${IMGUI_DIR}. Execute: git clone https://github.com/ocornut/imgui.git extern/imgui")
endif()

# Biblioteca ImGui Core
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})

# Backends do ImGui (GLFW + OpenGL3)
add_library(imgui_impl_glfw STATIC
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.h
)
target_link_libraries(imgui_impl_glfw PUBLIC imgui glfw)
target_include_directories(imgui_impl_glfw PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

add_library(imgui_impl_opengl3 STATIC
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.h
)
target_link_libraries(imgui_impl_opengl3 PUBLIC imgui OpenGL::GL)
target_include_directories(imgui_impl_opengl3 PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# ------------- Executável -------------
add_executable(MeuProjetoChai3D
    src/main.cpp
    src/haptics.cpp
)

# --- Linkagem de bibliotecas ---
target_link_libraries(MeuProjetoChai3D
    ${CHAI3D_ROOT}/lib/release/lin-x86_64-cc/libchai3d.a
    ${DHD_LIBRARY}
    ${DRD_LIBRARY}
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    OpenGL::GL GLU X11 dl pthread
    ${USB_LIBRARIES}
    imgui
    imgui_impl_glfw
    imgui_impl_opengl3
)

# RPATH para DHD
if(EXISTS ${FD_SDK_LIB_DIR})
    target_link_options(MeuProjetoChai3D PRIVATE "-Wl,-rpath,${FD_SDK_LIB_DIR}")
endif()

message(STATUS "Configuração CMake concluída. Execute 'cmake .. && make' dentro de build/.")
