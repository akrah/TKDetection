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

#include "DGtal/shapes/implicit/ImplicitBall.h"
#include <DGtal/shapes/Shapes.h>
#include "DGtal/io/readers/MeshReader.h"


using namespace std;
using namespace DGtal;
using namespace Z3i;

namespace po = boost::program_options;




Domain 
getBoundingBoxDomain(const KSpace &K,  const vector<SCell> &aScellVect){
  int minZ = K.sCoord( aScellVect.at(0), 2 );
  int maxZ = K.sCoord( aScellVect.at(0), 2 );

  int minY = K.sCoord( aScellVect.at(0), 1 );
  int maxY = K.sCoord( aScellVect.at(0), 1 );

  int minX = K.sCoord( aScellVect.at(0), 0 );
  int maxX = K.sCoord( aScellVect.at(0), 0 );

  for (unsigned int i= 1; i<aScellVect.size(); i++){
    int z = K.sCoord( aScellVect.at(i), 2 );
    int y = K.sCoord( aScellVect.at(i), 1 );
    int x = K.sCoord( aScellVect.at(i), 0 );
    if(z<minZ)
      minZ =z;
    if(z>maxZ)
      maxZ=z;
    if(y<minY)
      minY =y;
    if(y>maxY)
      maxY=y;
    if(z<minZ)
      minZ =z;
    if(z>maxZ)
      maxZ=z;
  }
  return Domain(Point(minX, minY, minZ),Point(maxX, maxY, maxZ));
}



RealPoint 
getBarycenter(const KSpace &K,  const vector<SCell> &aScellVect){
  RealPoint pointResu;
  unsigned int nb=0;
  for(vector<SCell>::const_iterator it = aScellVect.begin(); it!= aScellVect.end(); it++){
      pointResu[0] += K.sCoord( *it, 0 );
      pointResu[1] += K.sCoord( *it, 1 );
      pointResu[2] += K.sCoord( *it, 2 );
      nb++; 
  }
  pointResu[0] /=nb;
  pointResu[1] /=nb;
  pointResu[2] /=nb;
  return pointResu;
}




void 
exportSDP(std::ofstream &out,  const DigitalSet &aSet){
  for(DigitalSet::ConstIterator it = aSet.begin(); it!= aSet.end(); it++){
    out <<  (*it)[ 0 ] << " " ;
    out <<  (*it)[ 1 ] << " " ;
    out <<  (*it)[ 2 ] << " " <<std::endl;
  }
}



DigitalSet 
getMakerFromKnot(const Domain &aDomain, const KSpace &K, const vector<SCell> & aSCellVect, 
		 const RealPoint &center, double radius=5.0, double step=2, unsigned int length = 30,  int dec=0){
  DigitalSet result(aDomain);
  RealPoint barycenter = getBarycenter(K, aSCellVect);
  RealPoint vectDirection(barycenter[0]-center[0], barycenter[1]-center[1], barycenter[2]-center[2]) ;
  double norm = sqrt(vectDirection[0]*vectDirection[0]+vectDirection[1]*vectDirection[1]+ vectDirection[2]*vectDirection[2]);
  vectDirection[0]= vectDirection[0]/norm;
  vectDirection[1]= vectDirection[1]/norm;
  vectDirection[2]= vectDirection[2]/norm;
  
  for (unsigned int i=0; i<length; i+=step){
    RealPoint pt( barycenter[0]+i*vectDirection[0], barycenter[1]+i*vectDirection[1], barycenter[2]+i*vectDirection[2]);
    pt[0]+=dec*vectDirection[0];pt[1]+=dec*vectDirection[1]; pt[2]+=dec*vectDirection[2];
    ImplicitBall<Z3i::Space> impBall (pt, radius);
    Z3i::DigitalSet aSet( aDomain );
    Shapes<Domain>::euclideanShaper( aSet, impBall);
    result.insert(aSet.begin(), aSet.end());
     
  }
  
  return result;
}




int main(int argc, char** argv)
{

  // parse command line ----------------------------------------------
  po::options_description general_opt("Allowed options are: ");
  general_opt.add_options()
    ("help,h", "display this message")
    ("input-file,i", po::value<std::string>(), "vol file (.vol) , pgm3d (.p3d or .pgm3d) file or sdp (sequence of discrete points)" )
    ("trunkBark-mesh,t", po::value<std::string>(), "mesh of the trunk bark in format OFS non normalized (.ofs)" )
    ("marrow-mesh,a", po::value<std::string>(), "mesh of trunk marrow  in format OFS non normalized (.ofs)" )
    ("scaleX,x",  po::value<float>()->default_value(1.0), "set the scale value in the X direction (default 1.0)" )
    ("scaleY,y",  po::value<float>()->default_value(1.0), "set the scale value in the Y direction (default 1.0)" )
    ("scaleZ,z",  po::value<float>()->default_value(1.0), "set the scale value in the Z direction (default 1.0)")
    ("center,c",  po::value<std::vector <int> >()->multitoken(), "The coordinates of the center to define the seed ")
    ("minSizeBoundary,m",  po::value<unsigned int >()->default_value(100.0), "set the min size of the boundary to be extracted (default 100)" )
    ("transparency,T",  po::value<uint>()->default_value(100), "transparency") ;
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
		<< "Display volume file as a voxel set by using QGLviewer"
		<< general_opt << "\n";
      return 0;
    }

  if(! vm.count("input-file") && ! vm.count("mesh-file"))
    {
      trace.error() << " No input file was given" << endl;
      return 0;
    }
  string inputFilename;
  string extension;
  if( vm.count("input-file")){
    inputFilename = vm["input-file"].as<std::string>();
    extension = inputFilename.substr(inputFilename.find_last_of(".") + 1);
  }
  unsigned char transp = vm["transparency"].as<uint>();
  float sx = vm["scaleX"].as<float>();
  float sy = vm["scaleY"].as<float>();
  float sz = vm["scaleZ"].as<float>();

  

  QApplication application(argc,argv);
  Viewer3D viewer;

  viewer.setScale(sx,sy,sz);
  viewer.setWindowTitle("simple Volume Viewer");
  viewer.show();

  Point center(0,0,0);
  
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

  // Point for the domain
  Z3i::Point pMin;
  Z3i::Point pMax;

  if(extension=="sdp"){
    vector<Z3i::Point> vectVoxels = PointListReader<Z3i::Point>::getPointsFromFile(inputFilename);
    for(uint i=0;i< vectVoxels.size(); i++){
      const Z3i::Point &p= vectVoxels.at(i);
      if( p[0]<pMin[0]){
	pMin[0]=p[0];
      }
      if( p[1]<pMin[1]){
	pMin[1]=p[1];
      }
      if( p[2]<pMin[2]){
	pMin[2]=p[2];
      }

      if( p[0]>pMax[0]){
	pMax[0]=p[0];
      }
      if( p[1]>pMax[1]){
	pMax[1]=p[1];
      }
      if( p[2]>pMax[2]){
	pMax[2]=p[2];
      }
    }
    
    Domain domain(pMin, pMax);
    Z3i::DigitalSet set3d(domain);
    for(uint i=0;i< vectVoxels.size(); i++){
      set3d.insert(vectVoxels.at(i));
    }
    //A KhalimskySpace is constructed from the domain boundary points.
    KSpace K;
    K.init(pMin, pMax, true);

    SurfelAdjacency<3> SAdj( true );
    vector<vector<SCell> > vectConnectedSCell;

    cerr << "Extracting digital set .." ;

    Surfaces<KSpace>::extractAllConnectedSCell(vectConnectedSCell,K, SAdj, set3d, true);
    bool filterSizeBoundary=false;
    filterSizeBoundary= vm.count("minSizeBoundary");
    unsigned int minSize=0;
    if(filterSizeBoundary){
      minSize  = vm["minSizeBoundary"].as<unsigned int>();
    }
    cerr << "[done]"<< endl;



    GradientColorMap<long> gradient( 0, vectConnectedSCell.size());
    gradient.addColor(DGtal::Color::Red);
    gradient.addColor(DGtal::Color::Yellow);
    gradient.addColor(DGtal::Color::Green);
    gradient.addColor(DGtal::Color::Cyan);
    gradient.addColor(DGtal::Color::Blue);
    gradient.addColor(DGtal::Color::Magenta);
    gradient.addColor(DGtal::Color::Red);

    viewer << SetMode3D(vectConnectedSCell.at(0).at(0).className(), "Basic");
    ImageContainerBySTLVector<Domain, unsigned char> markerImage(domain);
    
    //default domain defined from set fo voxel
    Domain domainMarker(pMin, pMax);
    //domain given from parameters
    if(vm.count("domain")){
      std::vector<int> domainCoords= vm["domain"].as<std::vector <int> >();	
      Z3i::Point ptLower(domainCoords[0],domainCoords[1], domainCoords[2]);
      Z3i::Point ptUpper(domainCoords[3],domainCoords[4], domainCoords[5]);
      domainMarker= Domain(ptLower, ptUpper);
    }
    
    std::string filename= "exportedMarker.sdp";
    ofstream out; 
    out.open(filename.c_str());
	
    ImageContainerBySTLVector<Domain, unsigned char> markerImage(domainMarker);
    viewer << SetMode3D(vectConnectedSCell.at(0).at(0).className(), "Basic");    
    for(uint i=0; i< vectConnectedSCell.size();i++){
      Domain bDomain= getBoundingBoxDomain(K, vectConnectedSCell.at(i)); 
      Point lowerPt = bDomain.lowerBound();
      Point upperPt = bDomain.upperBound();
      
      unsigned int width = upperPt[2] - lowerPt[2] ;
      // trace.info() << "width= " << width <<endl;
      
      if(width>5){

	Z3i::DigitalSet aSet = getMakerFromKnot(domain, K, vectConnectedSCell.at(i), center, 5, 2, 100, -60 );
	exportSDP(out, aSet);  
	
	DGtal::ImageFromSet<ImageContainerBySTLVector<Domain, unsigned char> >::append(markerImage,aSet, 128);
	viewer << aSet;
	DGtal::Color c= gradient(i);
	viewer << CustomColors3D(Color(250, 0,0,transp), Color(c.red(),
							       c.green(),
							       c.blue(),120));	    
	    
	for(uint j=0; j< vectConnectedSCell.at(i).size();j++){
	  viewer << vectConnectedSCell.at(i).at(j);
	}
      }
    }
    GenericWriter< ImageContainerBySTLVector<Domain, unsigned char> >::exportFile("marker.pgm3D", markerImage);
    ImageContainerBySTLVector<Domain, unsigned char> markerImageSRC = 	DGtal::ImageFromSet<ImageContainerBySTLVector<Domain, unsigned char> >::create(set3d, 128);
    GenericWriter< ImageContainerBySTLVector<Domain, unsigned char> >::exportFile("marker2.pgm3D", markerImageSRC);

    
    
  }
  if(vm.count("trunkBark-mesh")){
    string meshFilename = vm["trunkBark-mesh"].as<std::string>();
    Mesh<Display3D::pointD3D> anImportedMesh(DGtal::Color(160, 30, 30,20));
    anImportedMesh.invertVertexFaceOrder();
    bool import = anImportedMesh << meshFilename;
    if(import){
      viewer << anImportedMesh;
    }
  }


  if(vm.count("marrow-mesh")){
    string meshFilename = vm["marrow-mesh"].as<std::string>();
    Mesh<Display3D::pointD3D> anImportedMesh(DGtal::Color(70,70,70,255));
    bool import = anImportedMesh << meshFilename;
    if(import){
      viewer << anImportedMesh;
    }
  }

  
 

  viewer << Viewer3D::updateDisplay;



  return application.exec();
}


