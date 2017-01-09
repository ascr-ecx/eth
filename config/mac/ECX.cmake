DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT=$DIR/../..

CC=mpicc CXX=mpicxx cmake .. \
	-DVTK_DIR=/Users/gda/vtk-7.0/lib/cmake/vtk-7.0 \
	-DCMAKE_BUILD_TYPE=Release  \
	-DCMAKE_INSTALL_PREFIX=$PROJECT/install 

