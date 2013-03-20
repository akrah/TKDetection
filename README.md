TKDetection
===========

TKDetection is a software to detect and segment wood knots.

1. [Dependencies](#dpendances)
2. [Installation of dependencies on Ubuntu](#installation-des-dpendances-sur-ubuntu)
3. [Installation of TKDetection](#installation-de-tkdetection)


Dependencies
-----------

|   |            Library                  |  Tested Version  |   |      Library            |  Tested Version  |   |        Library              |  Tested Version  |
|:-:|:-----------------------------------:|:----------------:|---|:-----------------------:|:----------------:|---|:---------------------------:|:----------------:|
| 1 | [Qt](#1-qt)                         |       4.8        | 4 | [Qwt](#4-qwt)           |       6.0.0      | 7 | [DGtalTools](#7-dgtaltools) |   repository     |
| 2 | [Armadillo](#2-armadillo)           |       3.6.1      | 5 | [Qxt](#5-qxt)           |       0.6.2      | 8 | [DGLib](#9-dglib)           |     online       |
| 3 | [InsightToolkit](#3-insighttoolkit) |       4.1        | 6 | [QwtPolar](#6-qwtpolar) |       1.0.1      | 9 | [ImaGene](#8-imagene)       |   repository     |


Installation of the dependencies on Ubuntu
---------------------------------------

### Preliminary 

Install the package "build-essential" and "cmake".


~~~
  sudo apt-get install build-essential cmake
~~~


### 1. QT
[Top](#tkdetection)

Install the package  *qtcreator* available on the repository *Universe*.
It is a meta-packet which install  the set of QT dependencies.

~~~
  sudo apt-get install qtcreator
~~~


### 2. Armadillo
[Top](#tkdetection)

Download the last version available on the site: http://arma.sourceforge.net.

It is recommended to install the libraries LAPACK, BLAS, ATLAS, and Boost to improve the performances in particular for the addition and multiplications of matrices. 


~~~
  sudo apt-get install liblapack-dev libblas-dev libatlas-dev libboost-dev
~~~

Then install Armadillo by replacing *x-x-x* with  the number of the downloaded version:

~~~
  tar xvf armadillo-x.x.x.tar.gz
  cd armadillo-x.x.x/
  ./configure
  make
  sudo make install
~~~


### 3. InsightToolkit
[Top](#tkdetection)

Download the version available on the site http://www.itk.org/ITK/resources/software.html.
Install it by replacing  *x-x-x* with the number of the downloaded version :

~~~
  tar xvf InsightToolkit-x.x.x.tar.gz
  cd InsightToolkit-x.x.x/
  mkdir binary
  cd binary
  cmake ..
  make
  sudo make install
~~~


### 4. Qwt
[Top](#tkdetection)

Use the version available on the main repository. 

~~~
  sudo apt-get install libqwt-dev
~~~


### 5. Qxt
[Top](#tkdetection)

Download the version available on the site  http://www.libqxt.org.
The version available on the directories does not install the configuration file  *qxt.prf* and *qxtvars.prf* needed to TKDetection.pro 


Replace  *xxxxxxxxx* with the number attributed to the download.

~~~
  tar xvf libqxt-libqxt-xxxxxxxxx.tar.bz2
  cd libqxt-libqxt-xxxxxxxxx/
  ./configure
  make
  sudo make install
~~~

The the list of the Qxt widgets does not appears in the list of the widgets of QtDesigner:

~~~
  sudo ln -s /usr/local/Qxt/lib/libQxtGui.so.0 /usr/lib/
  sudo ln -s /usr/local/Qxt/lib/libQxtCore.so.0 /usr/lib/
~~~
  
Reload  QtDesigner.


### 6. QwtPolar
[Top](#tkdetection)

Use the version available on the site:  http://sourceforge.net/projects/qwtpolar.
Install it by replacing *x-x-x* with the number of the downloaded version:


~~~
  unzip qwtpolar-x-x-x.zip
  cd qwtpolar-x-x-x/
  mkdir build && cd build
  qmake ..
  make
  sudo make install
~~~

##### If you have problem during the compilation of TKDetection:

1.  Check that the following files exist:
    - /usr/share/qt4/mkspecs/features/qwtpolar.prf
    - /usr/share/qt4/mkspecs/features/qwtpolarconfig.pri

  If they does not exist:
  ~~~
      sudo ln -s /usr/local/qwtpolar-x.x.x/features/qwtpolar.prf /usr/share/qt4/mkspecs/features/
      sudo ln -s /usr/local/qwtpolar-x.x.x/features/qwtpolarconfig.pri /usr/share/qt4/mkspecs/features/
  ~~~

2.  if 1. does not resolve the problem apple: 

  ~~~
      sudo ln -s /usr/local/qwtpolar-x.x.x/lib/libqwtpolar.so       /usr/lib/
      sudo ln -s /usr/local/qwtpolar-x.x.x/lib/libqwtpolar.so.1     /usr/lib/
      sudo ln -s /usr/local/qwtpolar-x.x.x/lib/libqwtpolar.so.1.0   /usr/lib/
      sudo ln -s /usr/local/qwtpolar-x.x.x/lib/libqwtpolar.so.1.0.0 /usr/lib/
  ~~~

##### If the widget  QwtPolarPlot does not appear in the list of QtDesigner widgets

Don't forget to replace  x.x.x with the number of the downloaded version:

~~~
  sudo cp /usr/local/qwtpolar-x.x.x/plugins/designer/libqwt_polar_designer_plugin.so /usr/lib/x86_64-linux-gnu/qt4/plugins/designer/
~~~


### 7. DGtalTools
[Top](#tkdetection)

Need the installation of the DGtal library.
You can do it by cloning it directly from the main repository :

~~~
  git clone git://github.com/DGtal-team/DGtal.git DGtal
  cd DGtal
  mkdir build && cd build
  cmake .. -DWITH_GMP=true -DWITH_ITK=true -DWITH_QGLVIEWER=true
  make
  sudo make install
~~~

Then Install DGtalTools :

~~~
  git clone git://github.com/DGtal-team/DGtalTools.git DGtalTools
  cd DGtalTools
  mkdir build && cd build
  cmake .. -DWITH_VISU3D_QGLVIEWER=true
  make
  sudo make install
~~~


### 8. ImaGene
[Top](#tkdetection)

DGLib uses feature of the ImaGene library.
Install ImaGene, by using for example a version without dependencies :

~~~
  git clone git://github.com/kerautret/ImaGeneNoDep.git
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

Installation of TKDetection
---------------------------
[Top](#tkdetection)

~~~
  git clone https://github.com/adrien057/TKDetection.git
  cd TKDetection
  mkdir build && cd build
  qmake ..
  make
~~~

The executable binary file   **TKDetection** is then located in the directory TKDetection.
