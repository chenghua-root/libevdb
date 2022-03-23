BASE_DIR=$(cd "$(dirname "$0")"; pwd)

cd $BASE_DIR

mkdir -p bin
mkdir -p bin-unittest

# binnanry
echo -e "\033[33m begin compile \033[0m"
mkdir -p build
pushd build
cmake ../
make -j2
popd
echo -e "\033[33m end compile \033[0m"

# unittest
echo -e "\033[33m begin compile unittest \033[0m"
mkdir -p build-unittest
pushd build-unittest
cmake ../
make -j2
popd
echo -e "\033[33m end compile unittest \033[0m"
