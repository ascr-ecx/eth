DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT=$DIR/../..

module load qt/4.8.4 vtk

CC=mpicc CXX=mpicxx cmake .. \
	-DCMAKE_BUILD_TYPE=Release  \
	-DCMAKE_INSTALL_PREFIX=$PROJECT/install 

