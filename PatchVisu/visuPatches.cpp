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
#include "DGtal/shapes/Mesh.h"

using namespace std;
using namespace DGtal;
using namespace Z3i;

namespace po = boost::program_options;



void 
meshFromMarrow(DGtal::Mesh<DGtal::Z3i::RealPoint> &aMesh, unsigned int aRadius, 
                  std::vector<DGtal::Z3i::RealPoint> anVector, unsigned int stepSample, unsigned int nbAngularStep=36)
{
  unsigned int nbVertex=0;
 for (unsigned int i=0; i < anVector.size()-stepSample; i++)
   {
     DGtal::Z3i::RealPoint ptRefOrigin = anVector.at(i);
     DGtal::Z3i::RealPoint ptOrigin = anVector.at(i);
     DGtal::Z3i::RealPoint vectNormal = anVector.at(i+stepSample) - anVector.at(i);
     double d = -vectNormal[0]*ptOrigin[0] - vectNormal[1]*ptOrigin[1] - vectNormal[2]*ptOrigin[2];
       if(vectNormal[0]!=0){
        ptRefOrigin [0]= -d/vectNormal[0];
        ptRefOrigin [1]= 0.0;
        ptRefOrigin [2]= 0.0;
      }else if (vectNormal[1]!=0){
        ptRefOrigin [0]= 0.0;
        ptRefOrigin [1]= -d/vectNormal[1];
        ptRefOrigin [2]= 0.0;
      }else if (vectNormal[2]!=0){
        ptRefOrigin [0]= 0.0;
        ptRefOrigin [1]= 0.0;
        ptRefOrigin [2]= -d/vectNormal[2];
      }
       DGtal::Z3i::RealPoint uDir1;
       uDir1=(ptRefOrigin-ptOrigin)/((ptRefOrigin-ptOrigin).norm());
       DGtal::Z3i::RealPoint uDir2;
       uDir2[0] = uDir1[1]*vectNormal[2]-uDir1[2]*vectNormal[1];
       uDir2[1] = uDir1[2]*vectNormal[0]-uDir1[0]*vectNormal[2];
       uDir2[2] = uDir1[0]*vectNormal[1]-uDir1[1]*vectNormal[0];
       uDir2/=uDir2.norm();
       std::vector<DGtal::Z3i::RealPoint> vectRing1;
       std::vector<DGtal::Z3i::RealPoint> vectRing2;
       for (unsigned int j = 0; j< nbAngularStep; j++){
         double angle = (6.28/nbAngularStep)*j;
         DGtal::Z3i::RealPoint pt1 = ptOrigin+(uDir1*cos(angle)*aRadius)+(uDir2*sin(angle)*aRadius);
         DGtal::Z3i::RealPoint pt2 = anVector.at(i+stepSample)+(uDir1*cos(angle)*aRadius)+(uDir2*sin(angle)*aRadius);
      
         aMesh.addVertex(pt1);
         aMesh.addVertex(pt2);

         vectRing1.push_back(pt1);
         vectRing2.push_back(pt2);
         nbVertex+=2;
       }
       // Generating mesh faces:
       unsigned int posRef = nbVertex-nbAngularStep*2;
       for (unsigned int j = 0; j< 2*nbAngularStep-4; j=j+2){
         aMesh.addQuadFace(posRef+j, posRef+1+j, posRef+3+j, posRef+2+j);
       }
       
   }

}




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
    ("volumeFile,v", po::value<std::string>(), "import volume image (dicom format)" )
    ("tangentialView", "Display tangential view defined from the set of the four imported points")
    ("crossSectionView", "Display cross section view defined from the set of simple points and the normal direction")
    ("pointsPk,p", po::value<std::string>(), "import the set of points Pk used to determine the image position." )
    ("scaleX,x",  po::value<float>()->default_value(1.0), "set the scale value in the X direction (default 1.0)" )
    ("scaleY,y",  po::value<float>()->default_value(1.0), "set the scale value in the Y direction (default 1.0)" )
    ("scaleZ,z",  po::value<float>()->default_value(1.0), "set the scale value in the Z direction (default 1.0)")
    ("patchMaxWidth,w",  po::value<unsigned int>()->default_value(50), "set the maximal patch width (default 50)")
    ("sampleStep,s",  po::value<unsigned int>()->default_value(20), "set the step between each patch images (default 20)")
    ("cutEnd,s",  po::value<unsigned int>()->default_value(0), "remove some end points of the profiles. ")
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
  unsigned int patchMaxWidth = vm["patchMaxWidth"].as<unsigned int>();
  unsigned int sampleStep = vm["sampleStep"].as<unsigned int>();
  unsigned int cutEnd = vm["cutEnd"].as<unsigned int> ();
  
  viewer.setGLScale(sx,sy,sz);
  Image3D imageVol = DicomReader< Image3D,  RescalFCT  >::importDicom(vm["volumeFile"].as<std::string>(), RescalFCT(-900,
                                                                                                                    530,
                                                                                                                    0, 255));
  
  std::vector<unsigned int> indexTopLeft; indexTopLeft.push_back(3); indexTopLeft.push_back(4); indexTopLeft.push_back(5);
  std::vector<unsigned int> indexBottomLeft; indexBottomLeft.push_back(6); indexBottomLeft.push_back(7); indexBottomLeft.push_back(8);
  std::vector<unsigned int> indexBottomRight; indexBottomRight.push_back(9); indexBottomRight.push_back(10); indexBottomRight.push_back(11);
  std::vector<unsigned int> indexTopRight; indexTopRight.push_back(12); indexTopRight.push_back(13); indexTopRight.push_back(14);
  
  std::vector<Z3i::RealPoint> vectPointsCenter = DGtal::PointListReader<Z3i::RealPoint>::getPointsFromFile(vm["pointsPk"].as<std::string>());
  std::vector<Z3i::RealPoint> vectPointsTopLeft = DGtal::PointListReader<Z3i::RealPoint>::getPointsFromFile(vm["pointsPk"].as<std::string>(), indexTopLeft);
  std::vector<Z3i::RealPoint> vectPointsTopRight = DGtal::PointListReader<Z3i::RealPoint>::getPointsFromFile(vm["pointsPk"].as<std::string>(), indexTopRight);
  std::vector<Z3i::RealPoint> vectPointsBottomLeft = DGtal::PointListReader<Z3i::RealPoint>::getPointsFromFile(vm["pointsPk"].as<std::string>(), indexBottomLeft);
  std::vector<Z3i::RealPoint> vectPointsBottomRight = DGtal::PointListReader<Z3i::RealPoint>::getPointsFromFile(vm["pointsPk"].as<std::string>(), indexBottomRight);

  trace.info() << "list of center point, contentains " << vectPointsCenter.size() << " points" << std::endl;
  trace.info() << "list of top left point, contentains " << vectPointsTopLeft.size() << " points" << std::endl;
  trace.info() << "list of top right point, contains " << vectPointsTopRight.size() << " points" << std::endl;
  trace.info() << "list of bottom left point, contains " << vectPointsBottomLeft.size() << " points" << std::endl;
  trace.info() << "list of bottom right point, contains " << vectPointsBottomRight.size() << " points" << std::endl;
  

  
  
  DGtal::DefaultFunctor idV;
  unsigned k =0;
  for (unsigned int i =0; i< vectPointsCenter.size() - cutEnd; 
       i=i+1){
    viewer.setFillColor(DGtal::Color(250,20,20,255));
    viewer << vectPointsCenter.at(i);

    if(i%sampleStep==0&& i< vectPointsCenter.size()-sampleStep){
      
      if(vm.count("crossSectionView")){
        unsigned int  width = (patchMaxWidth<(vectPointsTopRight.at(i) - vectPointsTopLeft.at(i)).norm()) ? patchMaxWidth:
          (vectPointsTopRight.at(i) - vectPointsTopLeft.at(i)).norm();
        DGtal::Z2i::Domain domainImage2D (DGtal::Z2i::Point(0,0), 
                                          DGtal::Z2i::Point(width, width)); 
        
        DGtal::Point2DEmbedderIn3D<DGtal::Z3i::Domain >  embedder(imageVol.domain(), 
                                                                  vectPointsCenter.at(i), 
                                                                  vectPointsCenter.at(i) - vectPointsCenter.at(i+sampleStep), 
                                                                  width);
        ImageAdapterExtractor extractedImage(imageVol, domainImage2D, embedder, idV);
        viewer << extractedImage;
        viewer << DGtal::UpdateImage3DEmbedding<Z3i::Space, Z3i::KSpace>(k, 
                                                                         embedder(Z2i::RealPoint(0,0), false),
                                                                         embedder(Z2i::RealPoint(width,0), false),
                                                                         embedder(domainImage2D.upperBound(), false),
                                                                         embedder(Z2i::RealPoint(0, width), false));
        
      }else if (vm.count("tangentialView")){
        
        DGtal::Z2i::Domain domainImage2D (DGtal::Z2i::Point(0,0), 
                                          DGtal::Z2i::Point((vectPointsTopRight.at(i) - vectPointsTopLeft.at(i)).norm(),
                                                            (vectPointsTopRight.at(i) - vectPointsBottomRight.at(i)).norm())); 
            
        DGtal::Point2DEmbedderIn3D<DGtal::Z3i::Domain >  embedder(imageVol.domain(), 
                                                                   vectPointsBottomLeft.at(i),vectPointsBottomRight.at(i), vectPointsTopLeft.at(i),
                                                                  DGtal::Z3i::Point(0,0, 0));
         ImageAdapterExtractor extractedImage(imageVol, domainImage2D, embedder, idV);        
         viewer << extractedImage;
         viewer << DGtal::UpdateImage3DEmbedding<Z3i::Space, Z3i::KSpace>(k, 
                                                                          embedder(Z2i::RealPoint(0,0), false),
                                                                          embedder(Z2i::RealPoint((vectPointsTopRight.at(i) - vectPointsTopLeft.at(i)).norm(),0), false),
                                                                          embedder(domainImage2D.upperBound(), false),
                                                                          embedder(Z2i::RealPoint(0, (vectPointsTopRight.at(i) - vectPointsBottomRight.at(i)).norm()), false));
      }     
      
      k++;
   
    }

  }
  DGtal::Mesh<Z3i::RealPoint> meshMoelle(true);
  meshFromMarrow(meshMoelle, 10.0, vectPointsCenter, 10, 30);
  
  
  viewer << meshMoelle;  
  viewer << My3DViewer::updateDisplay; 
  return application.exec();
}
