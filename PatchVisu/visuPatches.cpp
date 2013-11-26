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
#include "DGtal/io/readers/DicomReader.h"
#include "DGtal/io/readers/PointListReader.h"
#include "DGtal/kernel/BasicPointFunctors.h"


using namespace std;
using namespace DGtal;
using namespace Z3i;

namespace po = boost::program_options;


int main(int argc, char** argv)
{
  typedef DGtal::ImageContainerBySTLVector<DGtal::Z2i::Domain,   unsigned char > Image2D;
  typedef DGtal::ImageContainerBySTLVector<DGtal::Z3i::Domain,   unsigned char  > Image3D;
  typedef DGtal::ConstImageAdapter<Image3D, Z2i::Domain, DGtal::Point2DEmbedderIn3D<DGtal::Z3i::Domain>,
                                   Image3D::Value,  DGtal::DefaultFunctor >  ImageAdapterExtractor;

  typedef DGtal::Viewer3D<Z3i::Space, Z3i::KSpace> My3DViewer;  


  typedef DGtal::RescalingFunctor<int ,unsigned char > RescalFCT;

   
   
  po::options_description general_opt("Allowed options are: ");
  general_opt.add_options()
    ("help,h", "display this message")
    ("volumeFile,v", po::value<std::string>(), "import volume image" )
    ("pointsPk,p", po::value<std::string>(), "import the set of points Pk used to determine the image position." )
    ("scaleX,x",  po::value<float>()->default_value(1.0), "set the scale value in the X direction (default 1.0)" )
    ("scaleY,y",  po::value<float>()->default_value(1.0), "set the scale value in the Y direction (default 1.0)" )
    ("scaleZ,z",  po::value<float>()->default_value(1.0), "set the scale value in the Z direction (default 1.0)")
    ("trunkBark-mesh,t", po::value<std::string>(), "mesh of the trunk bark in format OFS non normalized (.ofs)" )
    ("marrow-mesh,m", po::value<std::string>(), "mesh of trunk marrow  in format OFS non normalized (.ofs)" );
  
 
  bool parseOK=true;
  po::variables_map vm;
  try{
    po::store(po::parse_command_line(argc, argv, general_opt), vm);
  }catch(const std::exception& ex){
    parseOK=false;
    trace.info()<< "Error checking program options: "<< ex.what()<< endl;
  }
  po::notify(vm);
  if( !parseOK || vm.count("help")||argc<=1)
    {
      std::cout << "Usage: " << argv[0] << " [input-file]\n"
		<< "Display patches images from volume image and from a set of 3D points Pk  (the image patches are oriented according to the normal vector defined from Pk, P(k+1).)  "
		<< general_opt << "\n";
      return 0;
    }



  QApplication application(argc,argv);
  My3DViewer viewer;
  
  viewer.setWindowTitle("visu patches");
  viewer.show();
  
  if(! vm.count("volumeFile") || ! vm.count("pointsPk") )
    {
      trace.error() << " Missing input file" << endl;
      return 0;
    }
  if(vm.count("marrow-mesh")){
     string meshFilename = vm["marrow-mesh"].as<std::string>();
     Mesh<Z3i::RealPoint> anImportedMesh(DGtal::Color(70,70,70,255));
     bool import = anImportedMesh << meshFilename;
     if(import){
       viewer << anImportedMesh;
     }
   }
  if(vm.count("trunkBark-mesh")){
	string meshFilename = vm["trunkBark-mesh"].as<std::string>();
	DGtal::Mesh<Z3i::RealPoint> anImportedMesh(DGtal::Color(160, 30, 30,20));
	anImportedMesh.invertVertexFaceOrder();
	bool import = anImportedMesh << meshFilename;
	if(import){
	  viewer << anImportedMesh;
	}
  }
  
  float sx = vm["scaleX"].as<float>();
  float sy = vm["scaleY"].as<float>();
  float sz = vm["scaleZ"].as<float>();      
  viewer.setGLScale(sx,sy,sz);
  Image3D imageVol = DicomReader< Image3D,  RescalFCT  >::importDicom(vm["volumeFile"].as<std::string>(), RescalFCT(-900,
                                                                                                                    0,
                                                                                                                    0, 255));
  std::vector<unsigned int> indexTopLeft; indexTopLeft.push_back(3); indexTopLeft.push_back(4); indexTopLeft.push_back(5);
  std::vector<unsigned int> indexTopRight; indexTopRight.push_back(6); indexTopRight.push_back(7); indexTopRight.push_back(8);
  
  std::vector<Z3i::RealPoint> vectPointsCenter = DGtal::PointListReader<Z3i::RealPoint>::getPointsFromFile(vm["pointsPk"].as<std::string>());
  std::vector<Z3i::RealPoint> vectPointsLeft = DGtal::PointListReader<Z3i::RealPoint>::getPointsFromFile(vm["pointsPk"].as<std::string>(), indexTopLeft);
  std::vector<Z3i::RealPoint> vectPointsRight = DGtal::PointListReader<Z3i::RealPoint>::getPointsFromFile(vm["pointsPk"].as<std::string>(), indexTopRight);
  std::vector<Z3i::RealPoint>::iterator itUpL = vectPointsLeft.begin();
  std::vector<Z3i::RealPoint>::iterator itUpR = vectPointsRight.begin();
  DGtal::DefaultFunctor idV;
  unsigned k =0;
  for (unsigned int i =0; i< vectPointsCenter.size()-15; 
       i=i+1){
    viewer.setFillColor(DGtal::Color(250,20,20,255));
    viewer << vectPointsCenter.at(i);
    if(i%10==0&& i< vectPointsCenter.size()-20){
      unsigned int  width = (50<(vectPointsRight.at(i) - vectPointsLeft.at(i)).norm()) ? 50:
        (vectPointsRight.at(i) - vectPointsLeft.at(i)).norm();
      DGtal::Z2i::Domain domainImage2D (DGtal::Z2i::Point(0,0), 
                                        DGtal::Z2i::Point(width, width)); 
      
      DGtal::Point2DEmbedderIn3D<DGtal::Z3i::Domain >  embedder(imageVol.domain(), 
                                                                vectPointsCenter.at(i), 
                                                                vectPointsCenter.at(i) - vectPointsCenter.at(i+10), 
                                                                width);
      ImageAdapterExtractor extractedImage(imageVol, domainImage2D, embedder, idV);
      viewer << extractedImage;
      viewer << DGtal::UpdateImage3DEmbedding<Z3i::Space, Z3i::KSpace>(k, 
                                                                       embedder(Z2i::RealPoint(0,0), false),
                                                                       embedder(Z2i::RealPoint(width,0), false),
                                                                       embedder(domainImage2D.upperBound(), false),
                                                                       embedder(Z2i::RealPoint(0, width), false));
      k++;
   
    }
    itUpL++;
    itUpR++;  
  } 
  
  
    
  viewer << My3DViewer::updateDisplay; 



  return application.exec();
}
