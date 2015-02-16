TKDetection
===========

TKDetection is a software to detect and segment wood knots.

1. [Dependencies list](#dependencies-list)
2. [Dependencies installation on Ubuntu](#dependencies-installation-on-ubuntu)
3. [TKDetection installation](#tkdetection-installation)


Dependencies list
-----------------

|   |            Library                  |  Tested version  |   |          Library            |  Tested version  |   |        Library              |  Tested version  |
|:-:|:-----------------------------------:|:----------------:|---|:---------------------------:|:----------------:|---|:---------------------------:|:----------------:|
| 1 | [Qt](#1-qt)                         |       4.8        | 4 | [Qwt](#4-qwt)               |      6.0.2       | 7 | [GSL](#8-gsl)               |       1.15       |
| 2 | [Armadillo](#2-armadillo)           |     4.600.4      | 5 | [Qxt](#5-qxt)               |      0.6.2       |   |                             |                  |
| 3 | [InsightToolkit](#3-insighttoolkit) |      4.5.2       | 6 | [QwtPolar](#6-qwtpolar)     |      1.0.1       |   |                             |                  |



Dependencies installation on Ubuntu
-----------------------------------

To install all dependencies on Ubuntu 12.10 64 bits, you can tu use the shell script script_install_dependencies_ubuntu12.10_x86_64.sh available on the TKDetection directory.
We recomand to copy the script on a dedicated directory before to execute the following commands :

~~~
	chmod u+x script_install_dependencies_ubuntu12.10_x86_64.sh
	./script_install_dependencies_ubuntu12.10_x86_64.sh
~~~

When installation finished, you can remove the dedicated directory.


On another plateform, you must apply the following steps.

### Preliminaries

Install "build-essential", "cmake" and "git" packages.


~~~
  sudo apt-get install build-essential cmake git
~~~

It is recommended to install the libraries LAPACK, BLAS, ATLAS, and Boost to improve the performances, in particular for the matrix sum and product of Armadillo.

~~~
  sudo apt-get install liblapack-dev libblas-dev libatlas-dev libatlas-base-dev libboost-dev libboost-all-dev
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
Install then Armadillo by replacing *x.y.z* by the downloaded version number:

~~~
  tar xvf armadillo-x.y.z.tar.gz
  cd armadillo-x.y.z/
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

Replace *x.y.z* by the downloaded version number:

~~~
  tar xvf InsightToolkit-x.y.z.tar.gz
  mkdir InsightToolkit-x.y.z/build
  cd InsightToolkit-x.y.z/build
  cmake .. -DCMAKE_CXX_FLAGS="-std=c++0x" -DCMAKE_BUILD_TYPE="Release" -DBUILD_EXAMPLES=false -DBUILD_TESTING=false -DITK_BUILD_ALL_MODULES=false -DITK_BUILD_DEFAULT_MODULES=false -DITKGroup_Core=true -DITKGroup_IO=true
  make
  sudo make install
~~~

If a problem appear with *tif_config.h* and/or *tif_dir.h* (typically with ITK 4.3), copy the missing files to the libraries repository:

~~~
  sudo cp  ./Modules/ThirdParty/TIFF/src/itktiff/tif_config.h /usr/local/include/ITK-x.y/itktiff/
  sudo cp ../Modules/ThirdParty/TIFF/src/itktiff/tif_dir.h    /usr/local/include/ITK-x.y/itktiff/
~~~

**Think to change the ITK_NUMBER variable by x.y in the TKDetection.pro file (line 13) !**

### 4. Qwt
[Top](#tkdetection)

Use the version available on the *Main* repository. 

~~~
  sudo apt-get install libqwt-dev
~~~

If problems appear, you can install the version available on the website http://sourceforge.net/projects/qwt/files/qwt/x.y.z/ :

~~~
  tar xvf qwt-x.y.z.tar.bz2
  mkdir qwt-x.y.z/build
  cd qwt-x.y.z/build
  qmake ../qwt.pro
  make
  sudo make install
~~~

##### If you have a problem during the TKDetection compilation step:

1.  Check that the following files exist:
    - /usr/share/qt4/mkspecs/features/qwt.prf
    - /usr/share/qt4/mkspecs/features/qwtconfig.pri
    - /usr/share/qt4/mkspecs/features/qwtfunctions.pri
    - /usr/share/qt4/mkspecs/features/qwtmathml.prf

  If they do not exist:
  ~~~
      sudo ln -s /usr/local/qwt-x.y.z/features/qwt.prf /usr/share/qt4/mkspecs/features/
      sudo ln -s /usr/local/qwtpolar-x.y.z/features/qwtconfig.pri /usr/share/qt4/mkspecs/features/
      sudo ln -s /usr/local/qwtpolar-x.y.z/features/qwtfunctions.pri /usr/share/qt4/mkspecs/features/
      sudo ln -s /usr/local/qwtpolar-x.y.z/features/qwtmathml.pri /usr/share/qt4/mkspecs/features/
  ~~~

2.  if 1. does not resolve the problem:

  ~~~
      sudo ln -s /usr/local/qwt-x.y.z/lib/libqwt.so       /usr/lib/
      sudo ln -s /usr/local/qwt-x.y.z/lib/libqwt.so.x     /usr/lib/
      sudo ln -s /usr/local/qwt-x.y.z/lib/libqwt.so.x.y   /usr/lib/
      sudo ln -s /usr/local/qwt-x.y.z/lib/libqwt.so.x.y.z /usr/lib/
      sudo ln -s /usr/local/qwt-x.y.z/lib/libqwtmathml.so       /usr/lib/
      sudo ln -s /usr/local/qwt-x.y.z/lib/libqwtmathml.so.x     /usr/lib/
      sudo ln -s /usr/local/qwt-x.y.z/lib/libqwtmathml.so.x.y   /usr/lib/
      sudo ln -s /usr/local/qwt-x.y.z/lib/libqwtmathml.so.x.y.z /usr/lib/
  ~~~

### 5. Qxt
[Top](#tkdetection)

Use the version available on the *Main* repository. 

~~~
  sudo apt-get install libqxt-dev
~~~

If the repository version does not install the configuration files *qxt.prf* and *qxtvars.prf* required by TKDetection.pro,
you can install the version available on the website http://www.libqxt.org.


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

Replace *x.y.z* by the downloaded version number:

~~~
  tar xvf qwtpolar-x.y.z.tar.bz2
  mkdir qwtpolar-x.y.z/build
  cd qwtpolar-x.y.z/build
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
      sudo ln -s /usr/local/qwtpolar-x.y.z/features/qwtpolar.prf /usr/share/qt4/mkspecs/features/
      sudo ln -s /usr/local/qwtpolar-x.y.z/features/qwtpolarconfig.pri /usr/share/qt4/mkspecs/features/
  ~~~

2.  if 1. does not resolve the problem:

  ~~~
      sudo ln -s /usr/local/qwtpolar-x.y.z/lib/libqwtpolar.so       /usr/lib/
      sudo ln -s /usr/local/qwtpolar-x.y.z/lib/libqwtpolar.so.x     /usr/lib/
      sudo ln -s /usr/local/qwtpolar-x.y.z/lib/libqwtpolar.so.x.y   /usr/lib/
      sudo ln -s /usr/local/qwtpolar-x.y.z/lib/libqwtpolar.so.x.y.z /usr/lib/
  ~~~

##### If the QwtPolarPlot widget does not appear in QtDesigner

Don't forget to replace *x.y.z* by the downloaded version number:

~~~
  sudo cp /usr/local/qwtpolar-x.y.z/plugins/designer/libqwt_polar_designer_plugin.so /usr/lib/x86_64-linux-gnu/qt4/plugins/designer/
~~~


### 8. GSL
[Top](#tkdetection)

The Gnu Standard Library (GSL) is present in the Ubuntu Main repository:

~~~
  sudo apt-get install libgsl0-dev
~~~

TKDetection installation
-------------------------
[Top](#tkdetection)


Clone the project from the Github repository of this webpage.

~~~
  git clone https://github.com/akrah/TKDetection.git
~~~

Check the ITK_NUMBER variable in the TKDetection.pro file (line 13) and compile:

~~~
  cd TKDetection
  mkdir build && cd build
  qmake ..
  make
~~~

The binary file **TKDetection** is then located in the build/bin directory of TKDetection.

### Translation

TKDetection is originally wrote in french language.
If you want to translate interface in english, just copy the **TKDetection_en.qm** file in the same folder than the binary file **TKDetection**.


[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/akrah/tkdetection/trend.png)](https://bitdeli.com/free "Bitdeli Badge")
