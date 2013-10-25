#! /bin/bash

### Ensemble des paquets à installer pour l'ensemble des dépendances de TKDetection
sudo apt-get install build-essential cmake qtcreator liblapack-dev libblas-dev libatlas-dev libboost-dev libqwt-dev libqxt-dev libqglviewer-dev-common libboost-program-options-dev libgmp-dev git

### Armadillo v. 3.920.2
wget "http://downloads.sourceforge.net/project/arma/armadillo-3.920.2.tar.gz"
tar xvf armadillo-3.920.2.tar.gz
cd armadillo-3.920.2/
./configure
make
sudo make install
cd -

### InsightToolkit v. 4.4.2
wget "http://sourceforge.net/projects/itk/files/itk/4.3/InsightToolkit-4.4.2.tar.gz"
tar xvf InsightToolkit-4.4.2.tar.gz
mkdir InsightToolkit-4.4.2/build
cd InsightToolkit-4.4.2/build
cmake .. -DCMAKE_CXX_FLAGS="-std=c++0x" -DCMAKE_BUILD_TYPE="Release" -DBUILD_EXAMPLES=false -DBUILD_TESTING=false -DITK_BUILD_ALL_MODULES=false -DITKGroup_Core=true -DITKGroup_IO=true
make
sudo make install
cd -

### QwtPolar v. 1.0.1
wget "http://downloads.sourceforge.net/project/qwtpolar/qwtpolar/1.0.1/qwtpolar-1.0.1.zip"
unzip qwtpolar-1.0.1.zip
mkdir qwtpolar-1.0.1/build
cd qwtpolar-1.0.1/build
qmake ..
make
sudo make install
cd -

sudo ln -s /usr/local/qwtpolar-1.0.0/features/qwtpolar.prf /usr/share/qt4/mkspecs/features/
sudo ln -s /usr/local/qwtpolar-1.0.0/features/qwtpolarconfig.pri /usr/share/qt4/mkspecs/features/
sudo ln -s /usr/local/qwtpolar-1.0.0/lib/libqwtpolar.so       /usr/lib/
sudo ln -s /usr/local/qwtpolar-1.0.0/lib/libqwtpolar.so.1     /usr/lib/
sudo ln -s /usr/local/qwtpolar-1.0.0/lib/libqwtpolar.so.1.0   /usr/lib/
sudo ln -s /usr/local/qwtpolar-1.0.0/lib/libqwtpolar.so.1.0.0 /usr/lib/
sudo cp /usr/local/qwtpolar-1.0.0/plugins/designer/libqwt_polar_designer_plugin.so /usr/lib/x86_64-linux-gnu/qt4/plugins/designer/ # Pour le designer

### DGtal
git clone git://github.com/DGtal-team/DGtal.git DGtal
mkdir DGtal/build
cd DGtal/build
cmake .. -DWITH_GMP=true -DWITH_ITK=true -DWITH_C11=true -DWITH_QGLVIEWER=true -DBUILD_EXAMPLES=false -DBUILD_TESTING=false -DCMAKE_BUILD_TYPE="Release"
make
sudo make install
cd -

### DGtalTools
git clone git://github.com/DGtal-team/DGtalTools.git DGtalTools
mkdir DGtalTools/build
cd DGtalTools/build
cmake .. -DWITH_VISU3D_QGLVIEWER=true -DCMAKE_BUILD_TYPE="Release"
make
sudo make install
cd -

### ImaGene
git clone git://github.com/kerautret/ImaGene-forIPOL.git
mkdir ImaGene-forIPOL/build
cd ImaGene-forIPOL/build
cmake .. -DCMAKE_BUILD_TYPE="Release" -BUILD_TESTING=false
make
sudo make install
cd -

### KerUtils
wget "http://www.loria.fr/~krahenbu/TKDetection/KerUtils.zip"
unzip KerUtils.zip
mkdir KerUtils/build
cd KerUtils/build
cmake .. -DCMAKE_BUILD_TYPE="Release"
make
sudo make install
cd -

