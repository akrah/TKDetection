#include <qapplication.h>
#include <DGtal/base/Common.h>
#include <DGtal/io/viewers/Viewer3D.h>
#include "DGtal/io/readers/VolReader.h"
#include "DGtal/io/Color.h"
#include "DGtal/io/colormaps/GradientColorMap.h"
#include "DGtal/images/ImageSelector.h"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include "DGtal/topology/helpers/Surfaces.h"
#include "DGtal/images/imagesSetsUtils/SetFromImage.h"
#include "DGtal/images/imagesSetsUtils/ImageFromSet.h"
#include "DGtal/io/readers/GenericReader.h"
#include "DGtal/io/writers/GenericWriter.h"
#include "DGtal/images/ConstImageAdapter.h"

#include <DGtal/shapes/Shapes.h>
#include "DGtal/io/readers/MeshReader.h"


using namespace std;
using namespace DGtal;
using namespace Z3i;

namespace po = boost::program_options;


int main(int argc, char** argv)
{
  typedef DGtal::ImageContainerBySTLVector<DGtal::Z2i::Domain,  unsigned char > Image2D;
  typedef DGtal::ImageContainerBySTLVector<DGtal::Z3i::Domain,  unsigned char > Image3D;
  typedef DGtal::ConstImageAdapter<Image3D, Image2D::Domain, DGtal::Projector< DGtal::Z3i::Space>,
				   Image3D::Value,  DGtal::DefaultFunctor >  SliceImageAdapter;

  typedef DGtal::Viewer3D<Z3i::Space, Z3i::KSpace> My3DViewer;  

  QApplication application(argc,argv);
  My3DViewer viewer;
   
  viewer.setWindowTitle("specific view");
  viewer.show();
  
     
  //viewer.setScale(sx,sy,sz);
  Image3D img = GenericReader<Image3D>::import("woodSPR1T.pgm3d");


  // slice longueur
  DGtal::Projector<DGtal::Z2i::Space>  invFunctorZ; invFunctorZ.initRemoveOneDim(1);
  DGtal::Z2i::Domain domain2DZ(invFunctorZ(img.domain().lowerBound()), 
			       invFunctorZ(img.domain().upperBound()));
  unsigned int slicePosZ=190;
  DGtal::Projector<DGtal::Z3i::Space> aSliceFunctorZ(slicePosZ); aSliceFunctorZ.initAddOneDim(1);
  SliceImageAdapter sliceImageZ(img, domain2DZ, aSliceFunctorZ, DGtal::DefaultFunctor());
  viewer << sliceImageZ;
  viewer << DGtal::UpdateImagePosition<Z3i::Space, Z3i::KSpace> (0, My3DViewer::yDirection, 0.0, slicePosZ,0.0 );



  // tranche classiques
  DGtal::Projector<DGtal::Z2i::Space>  invFunctorX; invFunctorX.initRemoveOneDim(2);
  DGtal::Z2i::Domain domain2DX(invFunctorX(img.domain().lowerBound()), 
			       invFunctorX(img.domain().upperBound()));
  unsigned int slicePosX=150;
  DGtal::Projector<DGtal::Z3i::Space> aSliceFunctorX(slicePosX); aSliceFunctorX.initAddOneDim(2);
  SliceImageAdapter sliceImageX(img, domain2DX, aSliceFunctorX, DGtal::DefaultFunctor());
  viewer << sliceImageX;
  viewer << DGtal::UpdateImagePosition<Z3i::Space, Z3i::KSpace> (1, My3DViewer::zDirection,  0.0,0.0, slicePosX );




  // // tranche classiques 2
  // DGtal::Projector<DGtal::Z2i::Space>  invFunctorX2; invFunctorX2.initRemoveOneDim(2);
  // DGtal::Z2i::Domain domain2DX2(invFunctorX2(img.domain().lowerBound()), 
  // 			       invFunctorX2(img.domain().upperBound()));
  // unsigned int slicePosX2=1120;
  // DGtal::Projector<DGtal::Z3i::Space> aSliceFunctorX2(slicePosX2); aSliceFunctorX2.initAddOneDim(2);
  // SliceImageAdapter sliceImageX2(img, domain2DX2, aSliceFunctorX2, DGtal::DefaultFunctor());
  // viewer << sliceImageX2;
  // viewer << DGtal::UpdateImagePosition(2, DGtal::Display3D::zDirection,  0.0,0.0, slicePosX2 );



  // Display mesh of center trunk
  
  Mesh<Z3i::RealPoint> anImportedMesh(DGtal::Color(250,200,200,255));
  viewer.setLineColor(DGtal::Color(250,200,200,255));
  anImportedMesh <<"moelleSPR1T.ofs";
  viewer << anImportedMesh;
  

  
    
  viewer << My3DViewer::updateDisplay;
 



  return application.exec();
}
