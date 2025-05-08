#!/bin/bash

# ======================================================
# SCRIPT DE INICIALIZAÇÃO OTIMIZADO (VERSÃO FINAL)
# ======================================================

set -euo pipefail

# Configurações adaptáveis
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
SRC_DIR="${PROJECT_DIR}/src"
EXECUTABLE="MeuProjetoChai3D"
CONFIG_FILE="games_config.json"
CHAI3D_PATH="/home/igor/chai3d-3.2.0-Makefiles"
SDK_PATH="/home/igor/sdk-3.17.6"

# Função para mostrar cabeçalho
show_header() {
    clear
    printf "\033[1;36m"
    echo "=============================================="
    echo "   Sistema de Jogos Educacionais - APAE"
    echo "=============================================="
    printf "\033[0m"
}

# Função para tratamento de erros
handle_error() {
    printf "\033[1;31mERRO: %s\033[0m\n" "$1" >&2
    exit 1
}

# Verificação inicial do sistema
pre_check() {
    # Verificar se é executado como root
    if [ "$(id -u)" -eq 0 ]; then
        handle_error "Não execute este script como root/sudo!"
    fi
    
    # Verificar distribuição compatível
    if ! grep -qiE 'ubuntu|debian' /etc/os-release; then
        handle_error "Script compatível apenas com Ubuntu/Debian"
    fi
}

# Função para seleção de modo
select_mode() {
    show_header
    echo -e "\nSelecione o modo de operação:"
    PS3=$'\nDigite sua opção (1-3): '
    options=(
        "Modo Simulação (sem hardware háptico)"
        "Modo Completo (com hardware háptico)"
        "Sair"
    )
    select choice in "${options[@]}"; do
        case $REPLY in
            1) prepare_simulation_mode; break ;;
            2) prepare_full_mode; break ;;
            3) exit 0 ;;
            *) echo "Opção inválida!"; sleep 1 ;;
        esac
    done
}

# Configuração para modo simulação
prepare_simulation_mode() {
    show_header
    echo -e "\n🔧 Configurando modo simulação...\n"
    export DISABLE_HAPTICS=ON
    setup_environment
    build_project
    run_safe_mode
}

# Configuração para modo completo
prepare_full_mode() {
    show_header
    echo -e "\n🔧 Configurando modo completo...\n"
    export DISABLE_HAPTICS=OFF
    verify_hardware
    setup_environment
    build_project
    run_full_mode
}

# Verificação de hardware háptico
verify_hardware() {
    echo "✅ Verificando hardware..."
    local missing=()
    
    [ ! -d "$CHAI3D_PATH" ] && missing+=("CHAI3D")
    [ ! -d "$SDK_PATH" ] && missing+=("Force Dimension SDK")
    
    if [ ${#missing[@]} -gt 0 ]; then
        handle_error "Componentes faltando: ${missing[*]}"
    fi
    
    if ! lsusb | grep -q "16d0:0"; then
        handle_error "Nenhum dispositivo háptico detectado!"
    fi
}

# Configuração do ambiente
setup_environment() {
    echo "⚙️  Configurando ambiente..."
    
    # Configuração de grupos
    if ! groups | grep -qw plugdev; then
        echo "Adicionando usuário ao grupo plugdev..."
        sudo usermod -aG plugdev "$USER"
    fi

    # Configurar udev rules
    echo "📝 Aplicando regras udev..."
    sudo tee /etc/udev/rules.d/99-haptic.rules > /dev/null <<EOL
# Force Dimension Devices
SUBSYSTEM=="usb", ATTR{idVendor}=="16d0", MODE="0666", GROUP="plugdev"
EOL

    sudo udevadm control --reload-rules
    sudo udevadm trigger
}

# Instalação de dependências
install_dependencies() {
    echo "📦 Verificando dependências..."
    local deps=(
        build-essential cmake
        libglfw3-dev libglew-dev
        libjsoncpp-dev libusb-1.0-0-dev
        libglu1-mesa-dev libx11-dev
    )

    for dep in "${deps[@]}"; do
        if ! dpkg -s "$dep" &> /dev/null; then
            echo "Instalando $dep..."
            sudo apt-get install -y "$dep" || handle_error "Falha ao instalar $dep"
        fi
    done
}

# Função para limpeza e preparação do build
prepare_build() {
    echo "🧹 Limpando builds anteriores..."
    rm -rf "${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"
}

# Função para verificar e copiar configurações
handle_config() {
    echo "📄 Verificando arquivo de configuração..."
    local src_config="${SRC_DIR}/${CONFIG_FILE}"
    local build_config="${BUILD_DIR}/${CONFIG_FILE}"
    
    if [ ! -f "${src_config}" ]; then
        handle_error "Arquivo de configuração não encontrado em: ${src_config}"
    fi
    
    cp -v "${src_config}" "${build_config}" || handle_error "Falha ao copiar arquivo de configuração"
}

# Função de compilação do projeto
build_project() {
    echo "🏗️  Compilando projeto..."
    cd "${BUILD_DIR}"
    
    local build_type="Release"
    local haptic_flag="-DDISABLE_HAPTICS=$DISABLE_HAPTICS"
    
    cmake -DCMAKE_BUILD_TYPE="${build_type}" "${haptic_flag}" .. || handle_error "Falha na configuração CMake"
    make -j"$(nproc)" || handle_error "Falha na compilação"
    
    handle_config  # Garantir que o arquivo de configuração está no build
    
    cd ..
}

# Funções de execução
run_safe_mode() {
    show_header
    echo -e "\n🚀 Iniciando em modo simulação...\n"
    cd "${BUILD_DIR}"
    "./${EXECUTABLE}"
}

run_full_mode() {
    show_header
    echo -e "\n🔥 Iniciando aplicação completa...\n"
    cd "${BUILD_DIR}"
    taskset -c 0 "./${EXECUTABLE}"
}

# Fluxo principal
main_flow() {
    pre_check
    show_header
    install_dependencies
    prepare_build
    select_mode
}

# Ponto de entrada do script
main_flow
