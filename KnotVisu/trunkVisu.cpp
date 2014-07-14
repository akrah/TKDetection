#include <qapplication.h>
#include <DGtal/base/Common.h>
#include <DGtal/io/viewers/Viewer3D.h>
#include <fstream>
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
#include "DGtal/io/readers/TableReader.h"
#include "DGtal/kernel/BasicPointFunctors.h"
#include "DGtal/shapes/Mesh.h"

using namespace std;
using namespace DGtal;
using namespace Z3i;

namespace po = boost::program_options;
             
typedef DGtal::ImageContainerBySTLVector<DGtal::Z2i::Domain,   unsigned char > Image2D;
typedef DGtal::ImageContainerBySTLVector<DGtal::Z3i::Domain,   unsigned char  > Image3D;
typedef DGtal::ConstImageAdapter<Image3D, Z2i::Domain, DGtal::functors::Point2DEmbedderIn3D<DGtal::Z3i::Domain>,
                                 Image3D::Value,  DGtal::functors::Identity >  ImageAdapterExtractor;
typedef DGtal::Viewer3D<Z3i::Space, Z3i::KSpace> My3DViewer;  
typedef DGtal::functors::Rescaling<int ,unsigned char > RescalFCT;
DGtal::functors::Identity idV;




void 
meshQuadZCylinder( const Image3D &image, My3DViewer &aViewer, DGtal::Mesh<DGtal::Z3i::RealPoint> &aMesh, DGtal::Z2i::RealPoint center, 
                   double radiusMin, double radiusMax, bool fillBorders,   unsigned int stepSample,
                   unsigned int startSliceZ, unsigned int endSliceZ, double angleMin, double angleMax,
                   double angularStep, double gridSize, DGtal::Z3i::RealPoint vectTranslationNonMesh=DGtal::Z3i::RealPoint(0,0,0))
{
 
  double angleMinRad = (angleMin/180.0)*3.142;
  double angleMaxRad = (angleMax/180.0)*3.142;
  double angularStepRad = (angularStep/180.0)*3.142;
  
  if (angleMinRad < 0) angleMinRad+=2.0*3.14159265;
  if (angleMaxRad < 0) angleMaxRad+=2.0*3.14159265;
  trace.info() << "angle min" << angleMinRad << std::endl;
  trace.info() << "angle max" << angleMaxRad << std::endl;
  trace.info() << "angle step" << angularStepRad << std::endl;
  bool cross0=false;
  if (angleMinRad > angleMaxRad){
    angleMaxRad+=2.0*3.14159265;
    cross0=true;
  }
  double realAngleMax=0;
  unsigned int nbVertex=aMesh.nbVertex();
  unsigned int nbVertexIni =nbVertex;
  for (unsigned int i=startSliceZ; i < endSliceZ; i=i+stepSample)
   {
     trace.info() << i << std::endl;
     DGtal::Z3i::RealPoint ptRefOrigin = Z3i::RealPoint(center[0], center[1], i);
     DGtal::Z3i::RealPoint uDir1(1.0,0,0);
     DGtal::Z3i::RealPoint uDir2(0, 1.0,0);
     unsigned int nbVertexSlice=0;
     
       for (double j = angleMinRad; j<= angleMaxRad; j=j+ angularStepRad){
         DGtal::Z3i::RealPoint pt1 = ptRefOrigin+(uDir1*cos(j)*radiusMax)+(uDir2*sin(j)*radiusMax);
         DGtal::Z3i::RealPoint pt2 = ptRefOrigin+Z3i::Point(0,0,stepSample)+(uDir1*cos(j)*radiusMax)+(uDir2*sin(j)*radiusMax);      
         aMesh.addVertex(pt1);
         aMesh.addVertex(pt2);
         nbVertexSlice+=2;
         nbVertex+=2;
         realAngleMax=j;
       }
       trace.info() << "circular generated" << i << std::endl;
       // Generating mesh faces:
       unsigned int posRef = nbVertex-nbVertexSlice;
       for (unsigned int j = 0; j< nbVertexSlice-2; j=j+2){
         aMesh.addQuadFace(posRef+2+j, posRef+3+j, posRef+1+j,posRef+j );
       }
       // last face
       if(angleMaxRad==angleMinRad)
         aMesh.addQuadFace(posRef, posRef+1, nbVertex-1, nbVertex-2);
       else if (fillBorders){
         // Generate two end/start face
         DGtal::Z3i::RealPoint pt1 = ptRefOrigin+(uDir1*cos(angleMaxRad)*radiusMin)+(uDir2*sin(angleMaxRad)*radiusMin);
         DGtal::Z3i::RealPoint pt2 = ptRefOrigin+Z3i::Point(0,0,stepSample)+
           (uDir1*cos(angleMaxRad)*radiusMin)+(uDir2*sin(angleMaxRad)*radiusMin);
         aMesh.addVertex(pt1);
         aMesh.addVertex(pt2);
         aMesh.addQuadFace(nbVertex-1, nbVertex-2, nbVertex, nbVertex+1);
         DGtal::Z3i::RealPoint pt3 = ptRefOrigin+(uDir1*cos(angleMinRad)*radiusMin)+(uDir2*sin(angleMinRad)*radiusMin);
         DGtal::Z3i::RealPoint pt4 = ptRefOrigin+Z3i::Point(0,0,stepSample)+
           (uDir1*cos(angleMinRad)*radiusMin)+(uDir2*sin(angleMinRad)*radiusMin);
         aMesh.addVertex(pt3);
         aMesh.addVertex(pt4);
         aMesh.addQuadFace(nbVertexIni, nbVertex+2, nbVertex+3, nbVertexIni+1);
         
         
       }
   }
  //Remplissage face avant:
    
  if (fillBorders){
    for (double i=0; i<2*radiusMax; i=i+gridSize){
      for (double j=0; j<2*radiusMax; j=j+gridSize){
        DGtal::Z3i::RealPoint pt (center[0]-radiusMax+i,
                                  center[1]-radiusMax+j,startSliceZ);
        DGtal::Z3i::RealPoint center3d(center[0], center[1], startSliceZ);
        double anglePoint = atan2((pt-center3d)[1],(pt-center3d)[0]); 
        if (anglePoint < 0 || cross0) anglePoint+=2.0*3.14159265;
        if((pt-center3d).norm()<radiusMax && (pt-center3d).norm()>radiusMin
           &&  anglePoint>angleMinRad && anglePoint<realAngleMax){
          unsigned int imageVal = image(pt);
          aViewer.setFillColor(DGtal::Color(imageVal, imageVal, imageVal));
          aViewer.addQuad(DGtal::Z3i::RealPoint(pt[0]-gridSize/2.0, pt[1]-gridSize/2.0, startSliceZ )+vectTranslationNonMesh,
                          DGtal::Z3i::RealPoint(pt[0]+gridSize/2.0, pt[1]-gridSize/2.0, startSliceZ )+vectTranslationNonMesh,
                          DGtal::Z3i::RealPoint(pt[0]+gridSize/2.0, pt[1]+gridSize/2.0, startSliceZ )+vectTranslationNonMesh,
                          DGtal::Z3i::RealPoint(pt[0]-gridSize/2.0, pt[1]+gridSize/2.0, startSliceZ )+vectTranslationNonMesh);
          
        }
      }
      
    }    //Remplissage face arrière:
  for (double i=0; i<2*radiusMax; i=i+gridSize){
    for (double j=0; j<2*radiusMax; j=j+gridSize){
      DGtal::Z3i::RealPoint pt (center[0]-radiusMax+i,
                                center[1]-radiusMax+j,endSliceZ);
      DGtal::Z3i::RealPoint center3d(center[0], center[1], endSliceZ);
      double anglePoint = atan2((pt-center3d)[1],(pt-center3d)[0]); 
      if (anglePoint < 0 || cross0) anglePoint+=2.0*3.14159265;

      if((pt-center3d).norm()<radiusMax && (pt-center3d).norm()>radiusMin
         &&  anglePoint>angleMinRad && anglePoint<realAngleMax){
        unsigned int imageVal = image(pt);
        aViewer.setFillColor(DGtal::Color(imageVal, imageVal, imageVal));
        aViewer.addQuad(DGtal::Z3i::RealPoint(pt[0]-gridSize/2.0, pt[1]-gridSize/2.0, endSliceZ )+vectTranslationNonMesh,
                        DGtal::Z3i::RealPoint(pt[0]+gridSize/2.0, pt[1]-gridSize/2.0, endSliceZ )+vectTranslationNonMesh,
                         DGtal::Z3i::RealPoint(pt[0]+gridSize/2.0, pt[1]+gridSize/2.0, endSliceZ )+vectTranslationNonMesh,
                         DGtal::Z3i::RealPoint(pt[0]-gridSize/2.0, pt[1]+gridSize/2.0, endSliceZ )+vectTranslationNonMesh);
      }
    }
  }
  } 
}

       
void 
embeddMeshInVol(const Image3D &anImage, My3DViewer &aViewer, DGtal::Mesh<DGtal::Z3i::RealPoint> &aMesh, 
                DGtal::Z3i::RealPoint vectTranslationNonMesh=DGtal::Z3i::RealPoint(0,0,0) )
{
  unsigned int  numImageDisplayed = aViewer.getCurrentGLImageNumber ();
  for (unsigned int i =0 ; i < aMesh.nbFaces(); i++){
    DGtal::Mesh<Z3i::RealPoint>::MeshFace aFace = aMesh.getFace(i);
    Z3i::RealPoint point1 = aMesh.getVertex(aFace.at(0));
    Z3i::RealPoint point2 = aMesh.getVertex(aFace.at(1));
    Z3i::RealPoint point3 = aMesh.getVertex(aFace.at(2));
    Z3i::RealPoint point4 = aMesh.getVertex(aFace.at(3));
    DGtal::Z2i::Domain domainImage2D (DGtal::Z2i::RealPoint(0,0), 
                                      DGtal::Z2i::RealPoint((point2-point1).norm(), (point4-point1).norm() ));    
    
    DGtal::functors::Point2DEmbedderIn3D<DGtal::Z3i::Domain >  embedder(anImage.domain(), 
                                                              point1, point2, point4, DGtal::Z3i::Point(0,0,0));      
    ImageAdapterExtractor extractedImage(anImage, domainImage2D, embedder, idV);        
    aViewer << extractedImage;
    aViewer << DGtal::UpdateImage3DEmbedding<Z3i::Space, Z3i::KSpace>(numImageDisplayed, 
                                                                     point1+vectTranslationNonMesh,
                                                                     point2+vectTranslationNonMesh,
                                                                     point3+vectTranslationNonMesh,
                                                                     point4+vectTranslationNonMesh);
    numImageDisplayed++;    
  }
}



int main(int argc, char** argv)
{
  
  po::options_description general_opt("Allowed options are: ");
  general_opt.add_options()
    ("help,h", "display this message")
    ("volumeFile,v", po::value<std::string>(), "import volume image (dicom format)" )
    ("scaleX,x",  po::value<float>()->default_value(1.0), "set the scale value in the X direction (default 1.0)" )
    ("scaleY,y",  po::value<float>()->default_value(1.0), "set the scale value in the Y direction (default 1.0)" )
    ("scaleZ,z",  po::value<float>()->default_value(1.0), "set the scale value in the Z direction (default 1.0)")
    ("centerCoord", po::value<std::vector <unsigned int> >()->multitoken(), "x, y, z coordinate of the center" );
  
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
		<< "Display  trunk as patches images  "
		<< general_opt << "\n";
      return 0;
    }


  QApplication application(argc,argv);
  My3DViewer viewer;
  viewer.setWindowTitle("visu patches");
  viewer.show();
  
  float sx = vm["scaleX"].as<float>();
  float sy = vm["scaleY"].as<float>();
  float sz = vm["scaleZ"].as<float>();      
  viewer.setGLScale(sx,sy,sz);
  Image3D imageVol = DicomReader< Image3D,  RescalFCT  >::importDicom(vm["volumeFile"].as<std::string>(), RescalFCT(-900,
                                                                                                                    530,
                                                                                                                    0, 255));
  Z2i::RealPoint center (vm["centerCoord"].as<std::vector<unsigned int> >()[0], 
                         vm["centerCoord"].as<std::vector<unsigned int> >()[1]);
  
  Z3i::Point upper = imageVol.domain().upperBound();
  Z3i::Point lower = imageVol.domain().lowerBound();
  Z2i::RealPoint center2Dz ((upper[0]-lower[0])/2, (upper[1]-lower[1])/2);
  DGtal::Mesh<Z3i::RealPoint> meshTrunk(true);
  DGtal::Mesh<Z3i::RealPoint> meshTrunk2(true);
  double distanceTranslation= 70;
  double dir = (350/180.0)*3.142;
  Z3i::RealPoint translationVect(distanceTranslation*cos(dir), distanceTranslation*sin(dir), 0);
  meshQuadZCylinder(imageVol, viewer, meshTrunk, center, 0,  170.0, true, 33, 367, 400, 31, 324, 5, 0.5); 
  meshQuadZCylinder(imageVol, viewer, meshTrunk2, center, 0,  170.0, true,33,  367, 400, 324, 31, 5, 0.5, translationVect); 

  embeddMeshInVol(imageVol, viewer, meshTrunk );
  embeddMeshInVol(imageVol, viewer, meshTrunk2, translationVect );
  

 
  viewer <<  My3DViewer::updateDisplay;
  
  return application.exec();
}
