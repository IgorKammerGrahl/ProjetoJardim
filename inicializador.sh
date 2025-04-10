sudo chmod 777 inicializador.sh

sudo apt update
sudo apt install build-essential cmake pkg-config libglfw3-dev libglew-dev libglu1-mesa-dev libx11-dev

echo "Inicializando o Jardim"

rm -rf build/

mkdir build && cd build

cmake -DCMAKE_BUILD_TYPE=Release ..

make

./MeuProjetoChai3D
