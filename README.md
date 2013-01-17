TKDetection
===========
A software to detect and segment wood knot

Dépendances
-----------

|            Bibliothèques            |  Version  |
|:-----------------------------------:|:----------|
| [Qt](#1-qt)                         |   4.7     |
| [Armadillo](#2-armadillo)           |   2.4.2   |
| [InsightToolkit](#3-insighttoolkit) |   4.0.0   |
| [Qwt](#4-qwt)                       |   6.0.0   |
| [Qxt](#5-qxt)                       |   0.6.2   |
| [QwtPolar](#6-qwtpolar)             |   1.0.0   |
| [DGtalTools](#7-dgtaltools)         |  current  |
| [ImaGene](#8-imagene)               |  perso    |
| [DGLib](#9-dglib)                   |  perso    |

Pré-requis
----------

Installez les paquets "build-essential" et "cmake".

~~~
  sudo apt-get install build-essential cmake
~~~


### 1. QT
[Top](#dépendances)

Installation du packet qtcreator dans le dépôt UNIVERSE qui installe les librairies de Qt par dépendance.

~~~
  sudo apt-get install qtcreator
~~~


### 2. Armadillo
[Top](#dépendances)

Utilisation de la dernière version disponible sur le site http://arma.sourceforge.net.

Il est recommandé d'installer LAPACK, BLAS, ATLAS et Boost pour améliorer les performances, particulièrement des additions et multiplication de matrices.

~~~
  sudo apt-get install liblapack-dev libblas-dev libatlas-dev libboost-dev
~~~
~~~
  tar xvf armadillo-2.4.2.tar.gz
  cd armadillo-2.4.2/
  cmake .
  make
  sudo make install
~~~


### 3. InsightToolkit
[Top](#dépendances)

Utilisation de la version disponible sur le site http://www.itk.org/ITK/resources/software.html.

~~~
  tar xvf InsightToolkit-4.0.0.tar.gz
  cd InsightToolkit-4.0.0/
  mkdir binary
  cd binary
  cmake ..
  make
  sudo make install
~~~


### 4. Qwt
[Top](#dépendances)

Utilisation de la version disponible dans le dépôt main.

~~~
  sudo apt-get install libqwt-dev
~~~


### 5. Qxt
[Top](#dépendances)

Utilisation de la version disponible sur le site http://www.libqxt.org.
La version des dépôts est 0.6.1 mais n'installe pas les fichiers de configuration (qxt.prf et qxtvars.prf) nécaissaires à TKDetection.pro.

~~~
  tar xvf libqxt-libqxt-af08f520f71c.tar.bz2
  cd libqxt-libqxt-af08f520f71c/
  ./configure
  make
  sudo make install
~~~

Si la liste des widgets de Qxt n'apparait pas dans la liste des widgets de QtDesigner :

~~~
  sudo ln -s /usr/local/Qxt/lib/libQxtGui.so.0 /usr/lib/
  sudo ln -s /usr/local/Qxt/lib/libQxtCore.so.0 /usr/lib/
~~~
  
Relancez QtDesigner.


### 6. QwtPolar
[Top](#dépendances)

Utilisation de la version disponible sur le site http://sourceforge.net/projects/qwtpolar.

~~~
  unzip qwtpolar-1.0.0.zip
  cd qwtpolar-1.0.0/
  qmake
  make
  sudo make install
~~~

##### Si vous rencontrez des problèmes lors de la compilation de TKDetection :

1.  Vérifiez que les deux fichiers suivants existent :
    - /usr/share/qt4/mkspecs/features/qwtpolar.prf
    - /usr/share/qt4/mkspecs/features/qwtpolarconfig.pri

  S'ils n'existent pas :
  ~~~
      sudo ln -s /usr/local/qwtpolar-1.0.0/features/qwtpolar.prf /usr/share/qt4/mkspecs/features/
      sudo ln -s /usr/local/qwtpolar-1.0.0/features/qwtpolarconfig.pri /usr/share/qt4/mkspecs/features/
  ~~~

2.  Si 1. n'a pas résolu le problème :

  ~~~
      sudo ln -s usr/local/qwtpolar-1.0.0/lib/libqwtpolar.so       /usr/lib/
      sudo ln -s usr/local/qwtpolar-1.0.0/lib/libqwtpolar.so.1     /usr/lib/
      sudo ln -s usr/local/qwtpolar-1.0.0/lib/libqwtpolar.so.1.0   /usr/lib/
      sudo ln -s usr/local/qwtpolar-1.0.0/lib/libqwtpolar.so.1.0.0 /usr/lib/
  ~~~

##### Si le widget QwtPolarPlot n'apparait pas dans la liste des widgets de QtDesigner :

~~~
  cd /usr/local/qwtpolar-1.0.0/plugins/designer/
  sudo ln -s libqwt_polar_designer_plugin.so /usr/lib/x86_64-linux-gnu/qt4/plugins/designer/
~~~


### 7. DGtalTools
[Top](#dépendances)

Nécessite l'installation de DGtal :

~~~
  git checkout git://github.com/DGtal-team/DGtal.git DGtal
  mkdir DGtal/build
  cd DGtal/build
  cmake .. -DWITH_GMP=true -DWITH_ITK=true -DWITH_QGLVIEWER=true
  make
  sudo make install
~~~

On peut ensuite installer DGtalTools :

~~~
  git checkout git://github.com/DGtal-team/DGtalTools.git DGtalTools
  mkdir DGtalTools/build
  cd DGTalTools/build
  cmake .. -DWITH_VISU3D_QGLVIEWER=true
  make
  sudo make install
~~~


### 8. ImaGene
[Top](#dépendances)

DGLib utilise des fonctionalité de d'ImaGene. Il faut donc installer ImaGene par exemple à partir d'une version sans dépendance:

~~~
  git clone git://github.com/kerautret/ImaGeneNoDep.git
  mkdir ImaGeneNoDep/build
  cd ImaGeneNoDep/build
  cmake ..
  make
  sudo make install
~~~


### 9. DGLib
[Top](#dépendances)

Doit être téléchargé ici : http://www.loria.fr/~krahenbu/dgci2013/DGLib_for_TKDetection.zip

~~~
  unzip DGLib_for_TKDetection
  mkdir DGLib_for_TKDetection/build
  cd DGLib_for_TKDetection/build
  cmake ..
  make
  sudo make install
~~~
