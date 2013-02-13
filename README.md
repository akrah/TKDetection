TKDetection
===========

TKDetection is a software to detect and segment wood knots.

1. [Dépendances](#dépendances)
2. [Installation des dépendances sur Ubuntu](#installation-des-dépendances-sur-ubuntu)
3. [Installation de TKDetection](#installation-de-TKDetection)


Dépendances
-----------

|   |            Bibliothèques            |  Version testée  |   |      Bibliothèques      |  Version testée  |   |        Bibliothèques        |  Version testée  |
|:-:|:-----------------------------------:|:----------------:|---|:-----------------------:|:----------------:|---|:---------------------------:|:----------------:|
| 1 | [Qt](#1-qt)                         |       4.8        | 4 | [Qwt](#4-qwt)           |       6.0.0      | 7 | [DGtalTools](#7-dgtaltools) |      dépôt       |
| 2 | [Armadillo](#2-armadillo)           |       3.6.1      | 5 | [Qxt](#5-qxt)           |       0.6.2      | 8 | [DGLib](#9-dglib)           |     en ligne     |
| 3 | [InsightToolkit](#3-insighttoolkit) |       4.1        | 6 | [QwtPolar](#6-qwtpolar) |       1.0.1      | 9 | [ImaGene](#8-imagene)       |      dépôt       |


Installation des dépendances sur Ubuntu
---------------------------------------

### Pré-requis

Installez les paquets "build-essential" et "cmake".

~~~
  sudo apt-get install build-essential cmake
~~~


### 1. QT
[Top](#dépendances)

Installez le packet *qtcreator* disponible dans le dépôt *Universe*.
C'est un méta-packet qui installe l'ensemble des dépendances de Qt.

~~~
  sudo apt-get install qtcreator
~~~


### 2. Armadillo
[Top](#dépendances)

Téléchargez de la dernière version disponible sur le site http://arma.sourceforge.net.

Il est recommandé d'installer les bibliothèques LAPACK, BLAS, ATLAS et Boost pour améliorer les performances, particulièrement les additions et les multiplications de matrices.

~~~
  sudo apt-get install liblapack-dev libblas-dev libatlas-dev libboost-dev
~~~

Installez ensuite Armadillo en remplaçant *x-x-x* par le numéro de la version téléchargée :

~~~
  tar xvf armadillo-x.x.x.tar.gz
  cd armadillo-x.x.x/
  ./configure
  make
  sudo make install
~~~


### 3. InsightToolkit
[Top](#dépendances)

Téléchargez la version disponible sur le site http://www.itk.org/ITK/resources/software.html.
Installez-la en remplaçant *x-x-x* par le numéro de la version téléchargée :

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
[Top](#dépendances)

Utilisez de la version disponible dans le dépôt *main*.

~~~
  sudo apt-get install libqwt-dev
~~~


### 5. Qxt
[Top](#dépendances)

Téléchargez la version disponible sur le site http://www.libqxt.org.
La version disponible via les dépôts n'installe pas les fichiers de configuration *qxt.prf* et *qxtvars.prf* nécaissaires à TKDetection.pro.

Remplacez *xxxxxxxxx* par le numéro attribué au téléchargement.

~~~
  tar xvf libqxt-libqxt-xxxxxxxxx.tar.bz2
  cd libqxt-libqxt-xxxxxxxxx/
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

Utilisez la version disponible sur le site http://sourceforge.net/projects/qwtpolar.
Installez-la en remplaçant *x-x-x* par le numéro de la version téléchargée :

~~~
  unzip qwtpolar-x-x-x.zip
  cd qwtpolar-x-x-x/
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
      sudo ln -s /usr/local/qwtpolar-x.x.x/features/qwtpolar.prf /usr/share/qt4/mkspecs/features/
      sudo ln -s /usr/local/qwtpolar-x.x.x/features/qwtpolarconfig.pri /usr/share/qt4/mkspecs/features/
  ~~~

2.  Si 1. n'a pas résolu le problème :

  ~~~
      sudo ln -s usr/local/qwtpolar-x.x.x/lib/libqwtpolar.so       /usr/lib/
      sudo ln -s usr/local/qwtpolar-x.x.x/lib/libqwtpolar.so.1     /usr/lib/
      sudo ln -s usr/local/qwtpolar-x.x.x/lib/libqwtpolar.so.1.0   /usr/lib/
      sudo ln -s usr/local/qwtpolar-x.x.x/lib/libqwtpolar.so.1.0.0 /usr/lib/
  ~~~

##### Si le widget QwtPolarPlot n'apparait pas dans la liste des widgets de QtDesigner

N'oubliez pas de remplacer x.x.x par le numéro de la version téléchargée :

~~~
  sudo cp /usr/local/qwtpolar-x.x.x/plugins/designer/libqwt_polar_designer_plugin.so /usr/lib/x86_64-linux-gnu/qt4/plugins/designer/
~~~


### 7. DGtalTools
[Top](#dépendances)

Nécessite l'installation de DGtal.
Faites-le par exemple en clonant le dépôt :

~~~
  git clone git://github.com/DGtal-team/DGtal.git DGtal
  cd DGtal
  mkdir build && cd build
  cmake .. -DWITH_GMP=true -DWITH_ITK=true -DWITH_QGLVIEWER=true
  make
  sudo make install
~~~

Installez ensuite DGtalTools :

~~~
  git clone git://github.com/DGtal-team/DGtalTools.git DGtalTools
  cd DGtalTools
  mkdir build && cd build
  cmake .. -DWITH_VISU3D_QGLVIEWER=true
  make
  sudo make install
~~~


### 8. ImaGene
[Top](#dépendances)

DGLib utilise des fonctionalité de d'ImaGene.
Installez ImaGene, par exemple à partir d'une version sans dépendance :

~~~
  git clone git://github.com/kerautret/ImaGeneNoDep.git
  cd ImaGeneNoDep
  mkdir build && cd build
  cmake ..
  make
  sudo make install
~~~


### 9. DGLib
[Top](#dépendances)

Doit être téléchargé ici : http://www.loria.fr/~krahenbu/dgci2013/DGLib_for_TKDetection.zip

~~~
  unzip DGLib_for_TKDetection
  cd DGLib_for_TKDetection
  mkdir build && cd build
  cmake ..
  make
  sudo make install
~~~

Installation de TKDetection
---------------------------
[Top](#dépendances)

~~~
  git clone https://github.com/adrien057/TKDetection.git
  cd TKDetection
  mkdir build && cd build
  qmake ..
  make
~~~

L'exécutable **TKDetection** se trouve alors dans le répertoire TKDetection.
