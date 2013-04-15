TKDetection
===========

TKDetection is a software to detect and segment wood knots.

1. [Dependencies list](#dependencies-list)
2. [Dependencies installation on Ubuntu](#dependencies-installation-on-ubuntu)
3. [TKDetection installation](#tkdetection-installation)


Dependencies list
-----------------

|   |            Library                  |  Tested version  |   |      Library            |  Tested version  |   |        Library              |  Tested version  |
|:-:|:-----------------------------------:|:----------------:|---|:-----------------------:|:----------------:|---|:---------------------------:|:----------------:|
| 1 | [Qt](#1-qt)                         |       4.8        | 4 | [Qwt](#4-qwt)           |       6.0.2      | 7 | [DGtalTools](#7-dgtaltools) |       0.6        |
| 2 | [Armadillo](#2-armadillo)           |     3.800.2      | 5 | [Qxt](#5-qxt)           |       0.6.2      | 8 | [ImaGene](#8-imagene)       |    repository    |
| 3 | [InsightToolkit](#3-insighttoolkit) |      4.3.1       | 6 | [QwtPolar](#6-qwtpolar) |       1.0.1      | 9 | [DGLib](#9-dglib)           |      online      |


Dependencies installation on Ubuntu
-----------------------------------

### Preliminaries

Install "build-essential" and "cmake" packages.


~~~
  sudo apt-get install build-essential cmake
~~~


### 1. QT
[Top](#tkdetection)

Install the *qtcreator* package available on the *Universe* repository.
This meta-package install the set of Qt dependencies.

~~~
  sudo apt-get install qtcreator
~~~


### 2. Armadillo
[Top](#tkdetection)

Download the last version available on the website http://arma.sourceforge.net.

It is recommended to install the libraries LAPACK, BLAS, ATLAS, and Boost to improve the performances, in particular for the matrix sum and product. 


~~~
  sudo apt-get install liblapack-dev libblas-dev libatlas-dev libboost-dev
~~~

Install then Armadillo by replacing *x-x-x* by the downloaded version number:

~~~
  tar xvf armadillo-x.x.x.tar.gz
  cd armadillo-x.x.x/
  ./configure
  make
  sudo make install
~~~

If problems appear concerning boost, you can compile boost from the version available on the website http://www.boost.org/users/history/version_1_53_0.html:

~~~
  tar xvf boost_1_53_0.tar.gz
  cd boost_1_53_0/
  ./bootstrap.sh -prefix=/usr/
  sudo ./b2 --build-type=complete --layout=tagged install
~~~


### 3. InsightToolkit
[Top](#tkdetection)

Download the version available on the website http://www.itk.org/ITK/resources/software.html.

Replace *x-x-x* by the downloaded version number:

~~~
  tar xvf InsightToolkit-x.x.x.tar.gz
  cd InsightToolkit-x.x.x/
  mkdir binary
  cd binary
  cmake .. -DBUILD_EXAMPLES:string=false -DBUILD_TESTING:string=false -DITK_BUILD_ALL_MODULES:string=false -DITKGroup_Core:string=true -DITKGroup_IO:string=true
  make
  sudo make install
~~~

If a problem appear with *tif_config.h* and/or *tif_dir.h*, copy the corresponding files to the libraries repository:

~~~
  sudo cp  ./Modules/ThirdParty/TIFF/src/itktiff/tif_config.h /usr/local/include/ITK-4.3/itktiff/
  sudo cp ../Modules/ThirdParty/TIFF/src/itktiff/tif_dir.h    /usr/local/include/ITK-4.3/itktiff/
~~~

**Think to change the ITK_NUMBER variable by x.x in the TKDetection.pro file (line 13) !**

### 4. Qwt
[Top](#tkdetection)

Use the version available on the *Main* repository. 

~~~
  sudo apt-get install libqwt-dev
~~~

If problems appear, you can install the version available on the website http://sourceforge.net/projects/qwt/files/qwt/6.0.2/:

~~~
  tar xvf qwt-6.0.2.tar.bz2
  cd qwt-6.0.2/
  mkdir build && cd build
  qmake ../qwt.pro
  make
  sudo make install
~~~

### 5. Qxt
[Top](#tkdetection)

Download the version available on the website http://www.libqxt.org.
The repository version does not install the configuration files *qxt.prf* and *qxtvars.prf*, required by TKDetection.pro.


Replace *xxxxxxxxx* by the downloaded file number.

~~~
  tar xvf libqxt-libqxt-xxxxxxxxx.tar.bz2
  cd libqxt-libqxt-xxxxxxxxx/
  ./configure
  make
  sudo make install
~~~

If Qxt widgets does not appear on QtDesigner:

~~~
  sudo ln -s /usr/local/Qxt/lib/libQxtGui.so.0 /usr/lib/
  sudo ln -s /usr/local/Qxt/lib/libQxtCore.so.0 /usr/lib/
~~~
  
Restart QtDesigner.


### 6. QwtPolar
[Top](#tkdetection)

Use the version available on the website http://sourceforge.net/projects/qwtpolar.

Replace *x-x-x* by the downloaded version number:


~~~
  unzip qwtpolar-x-x-x.zip
  cd qwtpolar-x-x-x/
  mkdir build && cd build
  qmake ..
  make
  sudo make install
~~~

##### If you have a problem during the TKDetection compilation step:

1.  Check that the following files exist:
    - /usr/share/qt4/mkspecs/features/qwtpolar.prf
    - /usr/share/qt4/mkspecs/features/qwtpolarconfig.pri

  If they do not exist:
  ~~~
      sudo ln -s /usr/local/qwtpolar-x.x.x/features/qwtpolar.prf /usr/share/qt4/mkspecs/features/
      sudo ln -s /usr/local/qwtpolar-x.x.x/features/qwtpolarconfig.pri /usr/share/qt4/mkspecs/features/
  ~~~

2.  if 1. does not resolve the problem:

  ~~~
      sudo ln -s /usr/local/qwtpolar-x.x.x/lib/libqwtpolar.so       /usr/lib/
      sudo ln -s /usr/local/qwtpolar-x.x.x/lib/libqwtpolar.so.1     /usr/lib/
      sudo ln -s /usr/local/qwtpolar-x.x.x/lib/libqwtpolar.so.1.0   /usr/lib/
      sudo ln -s /usr/local/qwtpolar-x.x.x/lib/libqwtpolar.so.1.0.0 /usr/lib/
  ~~~

##### If the QwtPolarPlot widget does not appear in QtDesigner

Don't forget to replace *x.x.x* by the downloaded version number:

~~~
  sudo cp /usr/local/qwtpolar-x.x.x/plugins/designer/libqwt_polar_designer_plugin.so /usr/lib/x86_64-linux-gnu/qt4/plugins/designer/
~~~


### 7. DGtalTools
[Top](#tkdetection)

DGtalTools required the DGtal library.
You can clone DGtalTools the main repository:

~~~
  git clone git://github.com/DGtal-team/DGtal.git DGtal
  cd DGtal
  mkdir build && cd build
  cmake .. -DWITH_GMP=true -DWITH_ITK=true -DWITH_QGLVIEWER=true
  make
  sudo make install
~~~

If a problem appear during the cmake step, add an ITK parameter to the command with the version number of ITK (x.x below):

~~~
  cmake .. -DWITH_GMP=true -DWITH_ITK=true -DWITH_QGLVIEWER=true -DITK_DIR=/usr/local/lib/cmake/ITK-x.x/
~~~

Install then DGtalTools:

~~~
  git clone git://github.com/DGtal-team/DGtalTools.git DGtalTools
  cd DGtalTools
  mkdir build && cd build
  cmake .. -DWITH_VISU3D_QGLVIEWER=true
  make
  sudo make install
~~~

If a problem appear, use the same tips than the DGtal installation:

~~~
  cmake .. -DWITH_VISU3D_QGLVIEWER=true -DWITH_QGLVIEWER=true -DITK_DIR=/usr/local/lib/cmake/ITK-x.x/
~~~

### 8. ImaGene
[Top](#tkdetection)

DGLib uses features of ImaGene library.

Install ImaGene by using the version without dependencies:

~~~
  git clone git://github.com/kerautret/ImaGene-forIPOL.git
  cd ImaGeneNoDep
  mkdir build && cd build
  cmake ..
  make
  sudo make install
~~~


### 9. DGLib
[Top](#tkdetection)

It should be downloaded here: http://www.loria.fr/~krahenbu/dgci2013/DGLib_for_TKDetection.zip

~~~
  unzip DGLib_for_TKDetection
  cd DGLib_for_TKDetection
  mkdir build && cd build
  cmake ..
  make
  sudo make install
~~~

TKDetection installation
-------------------------
[Top](#tkdetection)


Begin by check the ITK_NUMBER variable in the TKDetection.pro file (line 13).

Clone the project from the Github repository of this webpage and compile:

~~~
  git clone https://github.com/adrien057/TKDetection.git
  cd TKDetection
  mkdir build && cd build
  qmake ..
  make
~~~

The binary file **TKDetection** is then located in the build/bin directory of TKDetection.
