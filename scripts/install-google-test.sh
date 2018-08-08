set -e
VERSION=1.8.0
WORKDIR=`mktemp -d`

# Get
cd ${WORKDIR}
wget https://github.com/google/googletest/archive/release-${VERSION}.tar.gz
tar -xzvf release-${VERSION}.tar.gz

# Build
cd googletest-*
mkdir mybuild
cd mybuild
cmake -G"Unix Makefiles" ..
make
cd ../..

# Install
cd googletest-*
sudo cp -r googletest/include/gtest /usr/local/include
sudo cp `find -iname "lib*.a"` /usr/local/lib
cd ..

# Clean
cd ..
#rm -rf ${WORKDIR}
