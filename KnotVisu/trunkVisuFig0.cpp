#include <qapplication.h>
#include <DGtal/base/Common.h>
#include <DGtal/io/viewers/Viewer3D.h>
#include <fstream>
#include "DGtal/io/readers/VolReader.h"
#include "DGtal/io/Color.h"
#include "DGtal/io/colormaps/GradientColorMap.h"
#include "DGtal/io/colormaps/HueShadeColorMap.h"
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


struct ColorizeFonctor{
  ColorizeFonctor(double ratioRed=1.0, double ratioGreen=1.0, double ratioBlue=1.0 ): myRatioRed(ratioRed), 
                                                                                      myRatioBlue(ratioBlue),
                                                                                      myRatioGreen(ratioGreen)
  {
  }
  inline
  unsigned int operator() (unsigned char aVal) const
  {
    return (((unsigned int) (aVal*myRatioRed)) <<  16) 
      | (((unsigned int) (aVal*myRatioGreen)) << 8 ) 
      | ((unsigned int) (aVal*myRatioBlue)) ;
        }
  double myRatioRed, myRatioGreen, myRatioBlue; 
};




void 
meshQuadZCylinder( const Image3D &image, My3DViewer &aViewer, DGtal::Mesh<DGtal::Z3i::RealPoint> &aMesh, DGtal::Z2i::RealPoint center, 
                   double radiusMin, double radiusMax, bool fillBorders,   
                   unsigned int startSliceZ, unsigned int endSliceZ, double angleMin, double angleMax,
                   double angularStep, double gridSize, DGtal::Z3i::RealPoint vectTranslationNonMesh=DGtal::Z3i::RealPoint(0,0,0))
{
 
  unsigned int stepSample = endSliceZ- startSliceZ;
  
  double angleMinRad = (angleMin/180.0)*3.142;
  double angleMaxRad = (angleMax/180.0)*3.142;
  double angularStepRad = (angularStep/180.0)*3.142;
  bool fullCylinder=false;
  if(angleMax==angleMin){
    fullCylinder=true;
    angleMaxRad +=6.284;
  }
  
  if (angleMinRad < 0) angleMinRad+=2.0*3.14159265;
  if (angleMaxRad < 0) angleMaxRad+=2.0*3.14159265;
  
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
     trace.info() << "processing section :" << i << std::endl;
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
    
       // Generating mesh faces:
       unsigned int posRef = nbVertex-nbVertexSlice;
       for (unsigned int j = 0; j< nbVertexSlice-2; j=j+2){
         aMesh.addQuadFace(posRef+2+j, posRef+3+j, posRef+1+j,posRef+j );
       }
       // last face
       if(fullCylinder){
         aMesh.addQuadFace(posRef, posRef+1, nbVertex-1, nbVertex-2);
       }
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
           &&  ((anglePoint>angleMinRad && anglePoint<realAngleMax)||fullCylinder) ){
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
         &&  ((anglePoint>angleMinRad && anglePoint<realAngleMax)||fullCylinder) ){
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

template<typename TFunctor>
void 
embeddMeshInVol(const Image3D &anImage, My3DViewer &aViewer, DGtal::Mesh<DGtal::Z3i::RealPoint> &aMesh, 
                TFunctor aFunctor, DGtal::Z3i::RealPoint vectTranslationNonMesh=DGtal::Z3i::RealPoint(0,0,0))
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
    
    
    aViewer << AddTextureImage2DWithFunctor<ImageAdapterExtractor, TFunctor, Z3i::Space, Z3i::KSpace> (extractedImage, aFunctor , My3DViewer::RGBMode);
    
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
    ("cutSectionFirstIndex",  po::value<unsigned int>()->default_value(10), "indicate the cut section to be extracted (with the first index)")
    ("cutSectionLastIndex",  po::value<unsigned int>()->default_value(20), "indicate the cut section to be extracted (with the last index)")
    ("cutSectionSpace",  po::value<unsigned int>()->default_value(5), "indicate the space between billon and  cut section")
    ("colorizeCutSection", po::value<std::vector <double > >()->multitoken(), "ratioR, ratioG, ratioB: specify the ratio of each color channel to illustrate cut section (1.0, 1.0, 1.0 = white)" )
    ("startBillon",  po::value<unsigned int>()->default_value(0), "indicate the index of the first slice section to be extracted.")
    ("endBillon",  po::value<unsigned int>()->default_value(40), "indicate the index of the first slice section to be extracted.")
    
    ("gridSize",  po::value<double>()->default_value(0.5), "specify the grid size for top/bottom pixels filling.")
    ("angleStep",  po::value<double>()->default_value(5.0), "specify the angle step in degree to define the cylinder mesh.")
    
    
    ("cylinderRadius1Min",  po::value<unsigned int>()->default_value(300), "specify the min radius of first cylinder (bigger radius).")
    ("cylinderRadius1Max",  po::value<unsigned int>()->default_value(300), "specify the max radius of first cylinder (bigger radius).")
    ("cylinderRadius2Min",  po::value<unsigned int>()->default_value(300), "specify the min radius of second cylinder.")
    ("cylinderRadius2Max",  po::value<unsigned int>()->default_value(300), "specify the max radius of second cylinder.")
    ("cylinderRadius3Max",  po::value<unsigned int>()->default_value(300), "specify the max radius of third cylinder (no min since the third cylinder is plain).")
    
    ("translateCylinderIncluded", po::value<int>()->default_value(-50), "set the distance betwwen the cylindriic view") 
    ("translateSourceCylinder", po::value<int>()->default_value(150), "set the distance betwwen the cylindriic view") 
    ("displayAllSector", "display all sectors together (source cylinder)") 
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

  if(!vm.count("centerCoord")){
    trace.error() << "You have to sepecify center coordinates" << std::endl;
    return 0;
  }
  if(!vm.count("volumeFile")){
    trace.error() << "You have to sepecify the input volume file" << std::endl;
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
  DGtal::Mesh<Z3i::RealPoint> meshAubie(true);
  DGtal::Mesh<Z3i::RealPoint> meshCenter(true);
  DGtal::Mesh<Z3i::RealPoint> meshPith(true);  
  
  DGtal::Mesh<Z3i::RealPoint> meshALL(true);  
  
  double distanceTranslation= 70;
  double dir = (350/180.0)*3.142;
  Z3i::RealPoint translationVect(distanceTranslation*cos(dir), distanceTranslation*sin(dir), 0);

  unsigned int startBillonSection = vm["startBillon"].as<unsigned int > ();
  unsigned int endBillonSection = vm["endBillon"].as<unsigned int > ();  
  unsigned int cylinderRadius1Min = vm["cylinderRadius1Min"].as<unsigned int> ();
  unsigned int cylinderRadius1Max = vm["cylinderRadius1Max"].as<unsigned int> ();

  unsigned int cylinderRadius2Min = vm["cylinderRadius2Min"].as<unsigned int> ();
  unsigned int cylinderRadius2Max = vm["cylinderRadius2Max"].as<unsigned int> ();


  unsigned int cylinderRadius3Max = vm["cylinderRadius3Max"].as<unsigned int> ();

  
  unsigned int cutSectionFirstIndex = vm["cutSectionFirstIndex"].as<unsigned int > ();  
  unsigned int cutSectionLastIndex = vm["cutSectionLastIndex"].as<unsigned int > ();  
  unsigned int cutSectionSpace = vm["cutSectionSpace"].as<unsigned int > ();     
  
  double angleStep = vm["angleStep"].as<double>();
  double gridSize = vm["gridSize"].as<double>();
  std::vector<double> colorCoefs;
  if(vm.count("colorizeCutSection")){
    colorCoefs = vm["colorizeCutSection"].as<std::vector<double > >();
  }else{
    colorCoefs.push_back(1.0);
    colorCoefs.push_back(1.0);
    colorCoefs.push_back(1.0);
  }
  
  // Aubié mesh:
  meshQuadZCylinder(imageVol, viewer, meshAubie, center, cylinderRadius1Min,  cylinderRadius1Max, true,
                    startBillonSection, endBillonSection, 0, 0, angleStep, gridSize); 
  meshQuadZCylinder(imageVol, viewer, meshAubie, center, 0,  cylinderRadius1Min, false,
                    startBillonSection, endBillonSection, 0, 0, angleStep, gridSize); 
  embeddMeshInVol(imageVol, viewer, meshAubie, ColorizeFonctor(), Z3i::RealPoint(0,0, 0));
  
  // central area
  Z3i::RealPoint vectTranslate1 (0,0,vm["translateCylinderIncluded"].as<int>());
  meshQuadZCylinder(imageVol, viewer, meshCenter, center, cylinderRadius2Min,  cylinderRadius2Max, true,
                    startBillonSection, endBillonSection, 0, 0, angleStep, gridSize, vectTranslate1); 
  meshQuadZCylinder(imageVol, viewer, meshCenter, center, 0,  cylinderRadius2Min, false,
                    startBillonSection, endBillonSection, 0, 0, angleStep, gridSize, vectTranslate1); 
  
  embeddMeshInVol(imageVol, viewer, meshCenter, ColorizeFonctor(), vectTranslate1);  


  // pith area
  //Z3i::RealPoint vectTranslate2 (0,0,2*vm["translateCylinderIncluded"].as<int>());
  //meshQuadZCylinder(imageVol, viewer, meshPith, center,0,  cylinderRadius3Max, true,
  //                 startBillonSection, endBillonSection, 0, 0, angleStep, gridSize, vectTranslate2);   
  //embeddMeshInVol(imageVol, viewer, meshPith, ColorizeFonctor(), vectTranslate2);  
  


  
  // All Area 
  if(vm.count("displayAllSector")){
    Z3i::RealPoint vectTranslate3 (0,0, vm["translateSourceCylinder"].as<int>());
    
    meshQuadZCylinder(imageVol, viewer, meshPith, center,0,  cylinderRadius1Max, true,
                      startBillonSection, endBillonSection, 0, 0, angleStep, gridSize, vectTranslate3);   
    embeddMeshInVol(imageVol, viewer, meshPith, ColorizeFonctor(), vectTranslate3);  
    
  }
 
  viewer <<  My3DViewer::updateDisplay;
  
  return application.exec();
}
