DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT=$DIR/../..

module load qt/4.8.4 vtk

CC=mpicc \
CXX=mpicxx \
	cmake .. \
		-DVTK_DIR=/Users/gda/vtk-7.0/lib/cmake/vtk-7.0 \
		-DCMAKE_INSTALL_PREFIX=$PROJECT/install \
		-DCMAKE_BUILD_TYPE=Release \
		-DOSPRAY_APPS_MODELVIEWER=OFF \
		-DOSPRAY_MODULE_LOADERS=OFF \
		-DOSPRAY_MODULE_OPENGL_UTIL=OFF \
		-DOSPRAY_APPS_QTVIEWER=OFF \
		-DOSPRAY_APPS_VOLUMEVIEWER=OFF \
		-DCMAKE_CXX_COMPILER=mpicxx \
		-DISPC_EXECUTABLE=`which ispc` \
		-DCMAKE_CXX_FLAGS=-std=c++11 

