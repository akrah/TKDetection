#include <DGtal/base/Common.h>

#include "DGtal/topology/helpers/Surfaces.h"
#include "DGtal/images/imagesSetsUtils/SetFromImage.h"
#include "DGtal/images/imagesSetsUtils/ImageFromSet.h"
#include "DGtal/io/readers/GenericReader.h"
#include "DGtal/io/writers/GenericWriter.h"

#include "DGtal/shapes/implicit/ImplicitBall.h"
#include <DGtal/shapes/Shapes.h>
#include "DGtal/images/imagesSetsUtils/ImageFromSet.h"
#include <sstream>

using namespace std;
using namespace DGtal;
using namespace Z3i;


int main(int argc, char** argv)
{

  typedef DGtal::ImageContainerBySTLMap<DGtal::Z3i::Domain, unsigned char> Image3D;  
  Domain aDom (Point(0,0,0), Point(128, 128, 128));

  ImplicitBall<Z3i::Space> impBallReference (Point  (64, 64, 64), 40);
  ImplicitBall<Z3i::Space> impBallDecal (Point(60,60, 60), 40);
  ImplicitBall<Z3i::Space> impSmallBallNeg (Point(50,50, 50), 5);
  ImplicitBall<Z3i::Space> impSmallBallPos (Point(110, 110, 90), 3);
  
  Z3i::DigitalSet aSetReference( aDom );
  Z3i::DigitalSet aSetDecal( aDom );
  Z3i::DigitalSet aSetSub( aDom );
  Z3i::DigitalSet aSetAdd( aDom );

  Shapes<Domain>::euclideanShaper( aSetReference, impBallReference);
  Shapes<Domain>::euclideanShaper( aSetDecal, impBallDecal);
  Shapes<Domain>::euclideanShaper( aSetSub, impSmallBallNeg);
  Shapes<Domain>::euclideanShaper( aSetAdd, impSmallBallPos);

  Image3D imgReference(aDom);
  Image3D imgDecal(aDom);
  DGtal::ImageFromSet<Image3D >::append(imgReference, aSetReference, 128);
  DGtal::ImageFromSet<Image3D >::append(imgDecal, aSetDecal, 128);

  // Removing some voxels in the reference image:
  for(Z3i::DigitalSet::ConstIterator it= aSetSub.begin();  it!= aSetSub.end(); ++it){
    imgDecal.setValue(*it, 0);    
  }
  
  // Adding some voxels in the reference image:
  for(Z3i::DigitalSet::ConstIterator it= aSetAdd.begin();  it!= aSetAdd.end(); ++it){
    imgDecal.setValue(*it, 128);    
  }

  trace.info() << "exporting reference image ... ";
  imgReference >> "imageBallReference.p3d";
  trace.info() << " [done]" << endl;
  trace.info() << "exporting comp image ... ";  
  imgDecal >> "imageBallDecal.p3d";
  trace.info() << "[done]" << endl;
  
  unsigned int nbVoxelTruePos = 0;
  unsigned int nbVoxelTrueNeg = 0;
  unsigned int nbVoxelFalsePos = 0;
  unsigned int nbVoxelFalseNeg = 0;
  unsigned int nbVoxelTotComp=0;
  for(Image3D::Domain::ConstIterator it = aDom.begin(); it!=aDom.end(); it++){ 
    if(imgDecal(*it) == 128)  {
      nbVoxelTotComp++;
    }
    if((imgReference(*it) == 128) && (imgDecal(*it) == 128))  {
      nbVoxelTruePos++;
    }
    if((imgReference(*it) == 0) && (imgDecal(*it) == 128))  {
      nbVoxelFalsePos++;
    }
    if((imgReference(*it) == 128) && (imgDecal(*it) == 0))  {
      nbVoxelFalseNeg++;
    }    
    if((imgReference(*it) == 0) && (imgDecal(*it) == 0))  {
      nbVoxelTrueNeg++;
    }    
  }
  trace.info()<< "Number True Positive:" << nbVoxelTruePos << std::endl;
  trace.info()<< "Number True Negative:" << nbVoxelTrueNeg << std::endl;
  trace.info()<< "Number False Positive:" << nbVoxelFalsePos << std::endl;
  trace.info()<< "Number False Negative:" << nbVoxelFalseNeg << std::endl;
  trace.info() << "Number of reference voxel:" << aSetReference.size() << std::endl;
  trace.info() << "Number of comp voxel:" << nbVoxelTotComp  << std::endl;
}
