DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT=$DIR/../..

module load intel/15.0.3 cxx11
CC=mpicc CXX=mpicxx cmake .. \
	-DCMAKE_BUILD_TYPE=Release  \
	-DCMAKE_INSTALL_PREFIX=$PROJECT/install \
  -DCMAKE_CXX_FLAGS=-std=c++11 \
  -DOSPRAY_OBJDIR=$PROJECT/install/lib64 \
  -DOSPRAY_SRCDIR=$PROJECT/pvol-ospray \
  -DICET_INSTALL=$PROJECT/install

