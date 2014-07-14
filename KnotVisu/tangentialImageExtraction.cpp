#include <DGtal/base/Common.h>
#include "DGtal/kernel/BasicPointFunctors.h"

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
#include "DGtal/images/ConstImageAdapter.h"
#include "DGtal/io/readers/DicomReader.h"
#include "DGtal/kernel/BasicPointFunctors.h"

#include "DGtal/io/readers/GenericReader.h"
#include "DGtal/io/writers/GenericWriter.h"
#include "DGtal/io/writers/ITKwriter.h"

#include "DGtal/shapes/implicit/ImplicitBall.h"
#include <DGtal/shapes/Shapes.h>
#include "DGtal/io/readers/MeshReader.h"

#include <sstream>

using namespace std;
using namespace DGtal;
using namespace Z3i;

namespace po = boost::program_options;
typedef DGtal::ImageContainerBySTLVector<DGtal::Z2i::Domain,   unsigned char > Image2D;
typedef DGtal::ImageContainerBySTLVector<DGtal::Z3i::Domain, unsigned char> Image3D;
typedef DGtal::ConstImageAdapter<Image3D, Z2i::Domain, DGtal::functors::Point2DEmbedderIn3D<DGtal::Z3i::Domain>,
                                 Image3D::Value,  DGtal::functors::Identity >  ImageAdapterExtractor;

DGtal::functors::Identity idV;
typedef DGtal::functors::Rescaling<int ,unsigned char > RescalFCT;






int main(int argc, char** argv)
{   
 
 // parse command line ----------------------------------------------
  po::options_description general_opt("Allowed options are: ");
  general_opt.add_options()
    ("help,h", "display this message")
    ("inputFile,i", po::value<std::string>(), "source image used to extract the tangential images." )    
    ("outputFile,o", po::value<std::string>(), "set output filename (default output)." )
    ("center,c",  po::value<std::vector <int> >()->multitoken(), "The coordinates of the center to define the seed (default (0,0)). ")
    ("height",  po::value<unsigned int>()->default_value(50), "Height of the extracted image. ")
    ("distanceImage,d",  po::value<double >()->default_value(100.0), "define the distance of the extracted image from the center . ")
    ("startAngle,s",  po::value<double >()->default_value(0), "specify the start angle which will define the image. ")
    ("endAngle,e",  po::value<double >()->default_value(45), "specify the end angle which will define the image. ")
    ("points,p",  po::value<std::vector <int> >()->multitoken(), "The 2D coordinates of the points of the slice image. ");

  
  
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
		<< "Extract tangential image from volumetric images. "
		<< general_opt << "\n";
      return 0;
    }

  if(! (vm.count("inputFile")))
    {
      trace.error() << " No input file was given" << endl;
      return 0;
    }
  if(! (vm.count("outputFile")))
    {
      trace.error() << " No outputFile file was given" << endl;
      return 0;
    }
  string inputFilename =  vm["inputFile"].as<std::string>();    
  string outputFilename =  vm["outputFile"].as<std::string>();    
  Image3D imageVol = DicomReader< Image3D,  RescalFCT  >::importDicom(inputFilename, RescalFCT(-900,
                                                                                               500,
                                                                                               0, 255));

  trace.info() << imageVol.domain();
  trace.info()<< "Reading dicom [done]"<<std:: endl;
   
  
  Z3i::Point center(0,0, 0);
  if(vm.count("center")){
    std::vector<int> centerC= vm["center"].as<std::vector <int> >();
    if(centerC.size()!=3){
      trace.info() << "Incomplete option \"--center\""<< std::endl;
      return 0;
    }
    center[0]= centerC[0];
    center[1]= centerC[1];
    center[2]= centerC[2];
  }
  
  // Dimension of the image domain:
  double distanceImage = vm["distanceImage"].as<double>();
  double startAngle = ((vm["startAngle"].as<double>())/180.0)*3.14;
  double endAngle = ((vm["endAngle"].as<double>())/180)*3.14;

  
  if (startAngle>endAngle)
    endAngle+=6.28;
  double angleImage = endAngle-startAngle;

  unsigned int height = vm["height"].as<unsigned int>();
  unsigned int width = 2*(unsigned int)(sin(angleImage/2.0)*distanceImage);
  
  trace.info() << "Extracted Image Dimension: " << width << " " << height << std::endl;
  DGtal::Z2i::Domain domainImage2D (Z2i::Point(0, 0), Z2i::Point(width, height));
  
  //domain p1 p2, p3, p4 (counter clockwise)
  //bottom middle pcb
  Z3i::RealPoint p1, p2, p3, p4;
  if(!vm.count("points")){
    Z3i::Point pcb(center[0]+(unsigned int) distanceImage*cos(startAngle+angleImage/2.0),
                   center[1]+(unsigned int) distanceImage*sin(startAngle+angleImage/2.0) ,center[2]-height/2 );
    Z3i::Point pct(center[0]+(unsigned int) distanceImage*cos(startAngle+angleImage/2.0), 
                   center[1]+(unsigned int)distanceImage*sin(startAngle+angleImage/2.0) , center[2]+height/2);
    
    p1 = pcb+sin(angleImage/2.0)*distanceImage*(Z3i::RealPoint(-sin(startAngle+angleImage/2.0), cos(startAngle+angleImage/2.0), 0));
     p2 = pcb-sin(angleImage/2.0)*distanceImage*(Z3i::RealPoint(-sin(startAngle+angleImage/2.0), cos(startAngle+angleImage/2.0), 0));
    
     p3 = pct-sin(angleImage/2.0)*distanceImage*(Z3i::RealPoint(-sin(startAngle+angleImage/2.0), cos(startAngle+angleImage/2.0), 0));
     p4 = pct+sin(angleImage/2.0)*distanceImage*(Z3i::RealPoint(-sin(startAngle+angleImage/2.0), cos(startAngle+angleImage/2.0), 0));
    
    trace.info() << "Points of the 3D domain: " << std::endl;
    trace.info() << "p1: "<< p1 << std::endl;
    trace.info() << "p2: "<< p2 << std::endl;
    trace.info() << "p3: "<< p3 << std::endl;
    trace.info() << "p4: "<< p4 << std::endl;
  }else{
    std::vector< int> vectCoord = vm["points"].as<std::vector <int> >();
    p1[0]=vectCoord[0]; p1[1]=vectCoord[1]; p1[2]=center[2]-height/2;
    p2[0]=vectCoord[2]; p2[1]=vectCoord[3]; p2[2]=center[2]-height/2;
    p4[0]=p1[0]; p4[1]=p1[1]; p4[2]=center[2]+height/2; 
  }
  
  DGtal::functors::Point2DEmbedderIn3D<DGtal::Z3i::Domain >  embedder(imageVol.domain(), p1, p2, p4);
  trace.info() << imageVol.domain();
  ImageAdapterExtractor extractedImage(imageVol, domainImage2D, embedder, idV);        
  GenericWriter< ImageAdapterExtractor>::exportFile(outputFilename , extractedImage);


}

 






