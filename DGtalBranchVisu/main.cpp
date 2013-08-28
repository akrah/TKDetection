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

#include "DGtal/io/readers/GenericReader.h"


#include "DGtal/io/readers/MeshReader.h"


using namespace std;
using namespace DGtal;
using namespace Z3i;

namespace po = boost::program_options;

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



int main(int argc, char** argv)
{


  typedef DGtal::ImageContainerBySTLVector<DGtal::Z2i::Domain,  unsigned char > Image2D;
  typedef DGtal::ImageContainerBySTLVector<DGtal::Z3i::Domain,  unsigned char > Image3D;
  typedef DGtal::ConstImageAdapter<Image3D, Image2D::Domain, DGtal::Projector< DGtal::Z3i::Space>,
				   Image3D::Value,  DGtal::DefaultFunctor >  SliceImageAdapter;
  // parse command line ----------------------------------------------
  po::options_description general_opt("Allowed options are: ");
  general_opt.add_options()
	("help,h", "display this message")
	("input-file,i", po::value<std::string>(), "vol file (.vol) , pgm3d (.p3d or .pgm3d) file or sdp (sequence of discrete points)" )
	("volImageName,s", po::value<std::string>(), "specify the 3D image name to be used to display slice image (.vol, pgm3D, ...)." )
    ("sliceZ",po::value<unsigned int>(), "add a SliceZ image of number <arg>" )
    ("sliceY",po::value<unsigned int>(), "add a SliceY image of number <arg>" )
	
    ("trunkBark-mesh,t", po::value<std::string>(), "mesh of the trunk bark in format OFS non normalized (.ofs)" )
	("marrow-mesh,a", po::value<std::string>(), "mesh of trunk marrow  in format OFS non normalized (.ofs)" )
    ("scaleX,x",  po::value<float>()->default_value(1.0), "set the scale value in the X direction (default 1.0)" )
	("scaleY,y",  po::value<float>()->default_value(1.0), "set the scale value in the Y direction (default 1.0)" )
	("scaleZ,z",  po::value<float>()->default_value(1.0), "set the scale value in the Z direction (default 1.0)")
    ("filterMinSize,m",  po::value<int>(), "filter according min size of connected component. The CC is given only of its size is bigger or eq than [arg]  " )
("filterMaxX",  po::value<int>(), "filter according maximal X coordinate of the barycenter connected component. The CC is given only of its barycenter is less or eq than [arg]  " )
    ("filterMaxY",  po::value<int>(), "filter according maximal Y coordinate of the barycenter connected component. The CC is given only of its barycenter is less or eqthan [arg]  " )
    ("filterMaxZ",  po::value<int>(), "filter according maximal Z coordinate of the barycenter connected component. The CC is given only of its barycenter is less or eq than [arg]  " )
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


  
  if(vm.count("volImageName") &&(vm.count("sliceX")||vm.count("sliceY"))){
    string imageName = vm["volImageName"].as<std::string>();
    Image3D img = GenericReader<Image3D>::import(imageName);
    unsigned int sliceNum = vm.count("sliceZ")? vm["sliceZ"].as<unsigned int>() : vm["sliceY"].as<unsigned int>();
    
    if(vm.count("sliceZ")){
      DGtal::Projector<DGtal::Z2i::Space>  invFunctorZ; invFunctorZ.initRemoveOneDim(2);
      DGtal::Z2i::Domain domain2DZ(invFunctorZ(img.domain().lowerBound()), 
				   invFunctorZ(img.domain().upperBound()));
      DGtal::Projector<DGtal::Z3i::Space> aSliceFunctorZ(sliceNum); aSliceFunctorZ.initAddOneDim(2);
      SliceImageAdapter sliceImageZ(img, domain2DZ, aSliceFunctorZ, DGtal::DefaultFunctor());
      viewer << sliceImageZ;
      viewer << DGtal::UpdateImagePosition(0, DGtal::Display3D::zDirection,  0.0,0.0, sliceNum );
    }

    if(vm.count("sliceY")){      
      DGtal::Projector<DGtal::Z2i::Space>  invFunctorY; invFunctorY.initRemoveOneDim(1);
      DGtal::Z2i::Domain domain2DY(invFunctorY(img.domain().lowerBound()), 
				   invFunctorY(img.domain().upperBound()));

      DGtal::Projector<DGtal::Z3i::Space> aSliceFunctorY(sliceNum); aSliceFunctorY.initAddOneDim(1);
      SliceImageAdapter sliceImageY(img, domain2DY, aSliceFunctorY, DGtal::DefaultFunctor());
      viewer << sliceImageY;
      viewer << DGtal::UpdateImagePosition(0, DGtal::Display3D::yDirection, 0.0, sliceNum,0.0 );

      
    }
    
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


	cerr << "[done]"<< endl;



	GradientColorMap<long> gradient( 0,6);
	gradient.addColor(DGtal::Color::Red);
	gradient.addColor(DGtal::Color::Yellow);
	gradient.addColor(DGtal::Color::Green);
	gradient.addColor(DGtal::Color::Cyan);
	gradient.addColor(DGtal::Color::Blue);
	gradient.addColor(DGtal::Color::Magenta);
	gradient.addColor(DGtal::Color::Red);
	unsigned int cptComp=0;
	viewer << SetMode3D(vectConnectedSCell.at(0).at(0).className(), "Basic");
	for(uint i=0; i< vectConnectedSCell.size();i++){
	  DGtal::Color c= gradient(cptComp%7);
	  bool display=true;
	  if(vm.count("filterMinSize")){
	    int minSize = vm["filterMinSize"].as<int>();
	    display= display && (vectConnectedSCell.at(i).size()>=minSize);
	  }
	  if(vm.count("filterMaxZ") || vm.count("filterMaxY") || vm.count("filterMaxX")){
	    RealPoint barycenter =  getBarycenter(K,vectConnectedSCell.at(i)); 	    
	    if(vm.count("filterMaxZ")){
	      int maxZ = vm["filterMaxZ"].as<int>();
	      display=display && (barycenter[2]<= maxZ);
	    }
	    if(vm.count("filterMaxY")){
	      int maxY = vm["filterMaxY"].as<int>();
	      display=display && (barycenter[1]<= maxY);
	    }
	    if(vm.count("filterMaxX")){
	      int maxY = vm["filterMaxX"].as<int>();
	      display=display && (barycenter[0]<= maxY);
	    }
	    
	  }
	  if(display){
	    cptComp++;
	    viewer << CustomColors3D(Color(250, 0, 0, transp), Color(c.red(),
								     c.green(),
								     c.blue(),transp));
	    for(uint j=0; j< vectConnectedSCell.at(i).size();j++){
	      viewer << vectConnectedSCell.at(i).at(j);
	    }
	  }
	}
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
