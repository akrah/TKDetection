#include "inc/pithextractor.h"

#include "inc/billon.h"
#include "inc/coordinate.h"
#include "inc/pith.h"

PithExtractor::PithExtractor( const int &falseCutPercent, const int &windowWidth, const int &windowHeight, const int &binarizationThreshold, const int &pithLag ) :
	_falseCutPercent(falseCutPercent), _windowWidth(windowWidth), _windowHeight(windowHeight), _binarizationThreshold(binarizationThreshold), _pithLag(pithLag)
{
}

PithExtractor::~PithExtractor()
{
}

/********************************************************
 * Fonction principale d'extraction de la moelle
 ********************************************************/

void PithExtractor::process( Billon &billon ) const
{
	billon._pith.clear();

	const int width = billon.n_cols;
	const int height = billon.n_rows;
	const int depth = billon.n_slices;

	uiCoord2D coordPrec, coordCurrent;
	float maxStandList[depth];
	float *maxStandList2 = 0;
	float shift,houghStandThreshold;
	int max,x,y, nbContourPoints;

	max = x = y = 0;

	//extraction de la moelle de la premiere coupe sur la totalité de la coupe
	//nous permet d'avoir un coordonnée a laquelle appliqué la fenetre
	transHough( billon.slice(0), width, height, &x, &y, &max, &nbContourPoints );

	//calcul la coordonnée d'origine de la fenetre
	x = x - (_windowWidth/2);
	x = x < 0 ? 0 : (x > (width - _windowWidth) ? width - _windowWidth : x);
	y = y - (_windowHeight/2);
	y = y < 0 ? 0 : (y > (height - _windowHeight) ? height - _windowHeight : y);
	//extraction de la moelle de la premiere coupe sur la coupe redimensionnée
	coordCurrent = transHough( billon.slice(0), _windowWidth, _windowHeight, &x, &y, &max, &nbContourPoints );

	billon._pith.append(coordCurrent);
	maxStandList[0] = ((float)max)/nbContourPoints;

	//extraction des coupes suivantes
	std::cerr << "extractMoelle : ";
	for(int i=1; i<depth; i++) {
		//~ if(!listeCoupe->contains(i)){
			std::cerr << " " << i;
			coordPrec = billon._pith.last();
			x = x - (_windowWidth/2);
			x = x < 0 ? 0 : (x > (width - _windowWidth) ? width - _windowWidth : x);
			y = y - (_windowHeight/2);
			y = y < 0 ? 0 : (y > (height - _windowHeight) ? height - _windowHeight : y);
			//extraction de la moelle de la coupe i
			coordCurrent = transHough( billon.slice(i), _windowWidth, _windowHeight, &x, &y, &max, &nbContourPoints );
			billon._pith.append(coordCurrent);
			shift = sqrt(pow((double) ((int)(billon._pith.last().x) - (int)(coordPrec.x)),(double) 2.0) + pow( (double)((int)(billon._pith.last().y) - (int)(coordPrec.y)), (double)2.0) );
			//si le resultat obtenu a un decalage trop important avec la coupe precedente alors on recommence l'extraction sur l'ensemble de la coupe
			if(shift > _pithLag && width > _windowWidth && height > _windowHeight){
				std::cerr << "*";
				// std::cerr << "   :> decalage=" << decalage << ">" << _pithLag << "\n";

				billon._pith.pop_back();
				x = y = 0;
				transHough(billon.slice(i), width, height, &x, &y, &max, &nbContourPoints );
				x = x - (_windowWidth/2);
				x = x < 0 ? 0 : (x > (width - _windowWidth) ? width - _windowWidth : x);
				y = y - (_windowHeight/2);
				y = y < 0 ? 0 : (y > (height - _windowHeight) ? height - _windowHeight : y);
				coordCurrent = transHough( billon.slice(i), _windowWidth, _windowHeight, &x, &y, &max, &nbContourPoints );
				billon._pith.append(coordCurrent);
			}
			maxStandList[i] = ((float)max)/nbContourPoints;
			if (!(i % 20)) {
				std::cerr << std::endl;
			}
		//~ }else{
			//~ listMoelle->append(listMoelle->last());
			//~ listMaxStand[i] = 0;
		//~ }
	}
	std::cerr << "\nnbCoupes=" << depth << " listMoelle.size()=" << billon._pith.size() << "\n";

	//calcul du seuil a partir du quel les coupes sont considerées comme erronées
	maxStandList2 = new float[depth];
	memcpy(maxStandList2, maxStandList, depth*sizeof(float));

	qsort(maxStandList2, depth, sizeof(float), &floatCompare);

	int nfc = (_falseCutPercent*depth)/100;
	houghStandThreshold = nfc > 0 ? (maxStandList2[nfc]+maxStandList2[nfc-1])/2 : maxStandList2[nfc];

	delete [] maxStandList2;

	// applique la coorection a la moelle
	correctPith(billon._pith, maxStandList, houghStandThreshold);
}

/******************************************************************
 * Fonctions secondaires appelées lors de l'extraction de la moelle
 ******************************************************************/
uiCoord2D PithExtractor::transHough(const Slice &slice, int width, int height, int *x, int *y, int *sliceMaxValue, int *nbContourPoints) const
{
	int x_accu, y_accu, longueur, min;
	int *droite;
	Slice *tabaccu;
	arma::fmat *orientation, *cont;
	uiCoord2D coordmax;
	orientation = 0;
	{ // bloc de limitation de vie de la variable voisinage
		// attention x represente les colonne et y les lignes
		const Slice &voisinage = slice.submat( *y, *x, (uint)( (*y)+height-1 ), (uint)( (*x)+width-1 ) );
		cont = contour(voisinage, &orientation);
	}
	tabaccu = new Slice(width, height);
	tabaccu->zeros();
	//verifie orientation et table d'accumlation tous les points se trouvent a 0
	*nbContourPoints = 0;
	for(uint i=0; i<cont->n_rows; i++){
		for(uint j=0; j<cont->n_cols; j++){
			if((*cont)(i,j) == 1){
				*nbContourPoints += 1;
				droite = drawLine(j, i, (*orientation)(i,j), width, height, &longueur);
				for(int k=0; k<longueur; k++){
					y_accu = droite[k]/width;
					x_accu = droite[k]-(y_accu*width);
					(*tabaccu) (y_accu, x_accu) ++;
				}
				delete [] droite;
			}
		}
	}
	// std::cerr << "nbptcontour=" << * nbptcontour << "\n";
	delete cont;
	delete orientation;

	//min et max globaux
	min = *sliceMaxValue = (*tabaccu)(0,0);
	coordmax.x = coordmax.y = 0;

	minSlice(*tabaccu, &min, sliceMaxValue, &coordmax);

	delete tabaccu;

	*x += coordmax.x;
	*y += coordmax.y;

	return uiCoord2D(*x, *y);
}

arma::fmat * PithExtractor::contour(const Slice &slice, arma::fmat **orientation) const
{
	const uint height = slice.n_rows;
	const uint width = slice.n_cols;

	arma::fcolvec filtre1 = "1 2 1";			//filtre Sobel Gx
	arma::frowvec filtre2 = "-1 0 1";
	Slice *resultatGX, *resultatGY;
	arma::fmat norm = arma::fmat(height, width);
	arma::fmat *contour = new arma::fmat(height, width);

	*orientation = new arma::fmat(height, width);
	(*orientation)->zeros();
	resultatGX = convolution(slice, filtre1, filtre2);		//convolution Sobel GX
	filtre1 = "1 0 -1";										//filtre Sobel Gy
	filtre2 = "1 2 1";
	resultatGY = convolution(slice, filtre1, filtre2);		//convolution Sobel GY

	for (uint i=0; i<height; i++) {
		for (uint j=0; j<width; j++) {
			norm(i,j) = sqrt((*resultatGX)(i,j)*(*resultatGX)(i,j)+(*resultatGY)(i,j)*(*resultatGY)(i,j))/4;
			if (norm(i,j)>_binarizationThreshold && i>0 && i<norm.n_rows-1 && j>0 && j<norm.n_cols-1 ) {
				(*contour)(i,j) = 1;
				if((*resultatGX)(i,j)) {
					// Fred : pas de raison de faire une division entière ici :
					// (**orientation)(i,j) = (*resultatGY)(i,j) / (*resultatGX)(i,j);	//tangente teta
					(**orientation)(i,j) = double ((*resultatGY)(i,j)) / (*resultatGX)(i,j);	//tangente teta
				}
			} else {
				(*contour)(i,j) = 0;
			}
		}
	}
	delete resultatGX;
	delete resultatGY;

	return contour;
}

Slice * PithExtractor::convolution(const Slice &slice, arma::fcolvec verticalFilter, arma::frowvec horizontalFilter) const {
	const uint height = slice.n_rows;
	const uint width = slice.n_cols;
	Slice *resultat = new Slice(height, width);

	arma::fmat tmpSum;
	arma::fvec sum = arma::fvec(width);
	resultat->zeros();
	int kOffset;
	int kCenter;
	int lag;
	int endIndex;

	//convolve horizontal direction

	//find center position of kernel
	kCenter = horizontalFilter.n_cols/2;
	endIndex = width-kCenter;

	tmpSum = arma::fmat(height, width);
	tmpSum.zeros();
	for (uint i=0; i<height; i++) { //height pictures (nb rows)
		kOffset = 0;
		//index=0 to index=kCenter-1
		for (int j=0; j<kCenter; j++) {
			for (int k=kCenter+kOffset, m=0; k>=0; k--, m++) {
				tmpSum(i,j) += slice(i,m) * horizontalFilter[k];
			}
			kOffset++;
		}
		//index=kCenter to index=(width pictures)-kCenter-1)
		for (int j=kCenter; j<endIndex; j++) {
			lag = j-kCenter;
			for (int k=horizontalFilter.n_cols-1, m=0; k >= 0; k--, m++) {
				tmpSum(i,j) += slice(i,(lag+m)) * horizontalFilter[k];
			}
		}
		//index=(width pictures)-kCenter to index=(width pictures)-1
		kOffset = 1;
		for (uint j=endIndex; j<width; j++) {
			lag = j-kCenter;
			for (int k=horizontalFilter.n_cols-1, m=0; k >= kOffset; k--, m++) {
				tmpSum(i,j) += slice(i,lag+m) * horizontalFilter[k];
			}
			kOffset++;
		}
	}


	//convolve vertical direction

	//find center position of kernel
	kCenter = verticalFilter.n_rows/2;
	endIndex = width-kCenter;

	sum.zeros();

	kOffset = 0;
	//index=0 to index=(kCenter-1)
	for (int i=0; i <kCenter; i++) {
		lag = -i;
		for (int k = kCenter + kOffset; k>=0; k--) {			//convolve with partial kernel
			for (uint j=0; j<width; j++) {
				sum[j] += tmpSum(i+lag,j) * verticalFilter[k];
			}
			lag++;
		}
		for (uint n=0; n<width; n++) {				//convert to output format
			if(sum[n]>=0)
				(*resultat)(i,n) = (int)(sum[n] + 0.5f);
			else
				(*resultat)(i,n) = (int)(sum[n] - 0.5f);
			sum[n] = 0;											//reset before next summing
		}
		kOffset++;
	}
	//index=kCenter to index=(height pictures)-kCenter-1
	for (int i=kCenter; i<endIndex; i++) {
		lag = -kCenter;
		for (int k=verticalFilter.n_rows-1; k>=0; k--) {				//convolve with full kernel
			for(uint j=0; j<width; j++){
				sum[j] += tmpSum(i+lag,j) * verticalFilter[k];
			}
			lag++;
		}
		for (uint n=0; n<width; n++) {				//convert to output format
			if(sum[n]>=0)
				(*resultat)(i,n) = (int)(sum[n] + 0.5f);
			else
				(*resultat)(i,n) = (int)(sum[n] - 0.5f);
			sum[n] = 0;											//reset before next summing
		}
	}
	//index=(height pictures)-kcenter to index=(height pictures)-1
	kOffset = 1;
	for (uint i=endIndex; i<height; i++) {
		lag = -kCenter;
		for (int k=verticalFilter.n_rows-1; k>=kOffset; k--) {			//convolve with partial kernel
			for (uint j=0; j<width; j++) {			//height p	corrigeictures
				sum[j] += tmpSum(i+lag,j) * verticalFilter[k];
			}
			lag++;
		}
		for (uint n=0; n<width; n++) {				//convert to output format
			if(sum[n]>=0)
				(*resultat)(i,n) = (int)(sum[n] + 0.5f);
			else
				(*resultat)(i,n) = (int)(sum[n] - 0.5f);
			sum[n] = 0;											//reset before next summing
		}
		kOffset++;
	}

	return resultat;
}


int * PithExtractor::drawLine(int xOrigine, int yOrigine, float orientation_ORI, int width, int height, int *length) const
{
	float orientation = -orientation_ORI;	// orientation au sens de Fleur = - orientation_ORI
	int x, y, k1=0, k2=0;
	int dim = floor(sqrt(pow(width,2.0)+pow(height,2.0)));
	int *droite1, *droite2, *droite;
	float temp;

	droite1 = new int[dim];
	droite2 = new int[dim];
	droite = new int[dim];

	// 1er et 5e octant
	if ((orientation >= 0)  && (orientation < 1)) {		//entre 0 et 45		[0;45[
		// 1er octant
		temp = yOrigine;
		for(x=xOrigine; x<width; x++){
			 y= roundf(temp);
			 if(y>=height)
				 break;
			 droite1[k1] = y*width+x;
			 k1++;
			 temp += orientation;
		}

		//5e octant
		temp = yOrigine - orientation;
		for(x=xOrigine-1; x>=0; x--){
			y = roundf(temp);
			if(y<0)
				break;
			droite2[k2] = y*width+x;
			k2++;
			temp -= orientation;
		}
	}

	// 4e et 8e octant
	if ((orientation < 0) && (orientation > -1)) {		//entre 0 et -45	]0;-45[
		//8e octant
		temp = yOrigine;
		for(x=xOrigine; x<width; x++){
			y=roundf(temp);
			if(y<0)
				break;
			droite1[k1] = y*width+x;
			k1++;
			temp += orientation;
		}

		//4e octant
		temp = yOrigine-orientation;
		for(x=xOrigine-1; x>=0; x--){
			y = roundf(temp);
			if(y>=height)
				break;
			droite2[k2] = y*width+x;
			k2++;
			temp -= orientation;
		}
	}

	// 2e et 6e octant	corrigé
	if (orientation >= 1) {		//entre 45 et 90	[45;90]
		//2e octant
		temp = xOrigine;
		for(y=yOrigine; y<height; y++){
			x = roundf(temp);
			if(x>=width)
				break;
			droite1[k1] = y*width+x;
			k1++;
			temp += (1/orientation);
		}

		//6e octant
		temp=xOrigine-(1/orientation);
		for(y=yOrigine-1; y>=0; y--){
			x = roundf(temp);
			if(x<0)
				break;
			droite2[k2] = y*width+x;
			k2++;
			temp -= (1/orientation);
		}
	}

	// 3e et 7e octant	corrigé
	if (orientation <= -1) {	//entre -45 et -90		[-45;-90]
		//7e octant
		temp = xOrigine-(1/orientation);
		for (y=yOrigine-1; y>=0; y--) {
			x = roundf(temp);
			if(x>=width)
				break;
			droite1[k1] = y*width+x;
			k1++;
			temp -= 1/orientation;
		}
		//3e octant
		temp = xOrigine;
		for (y=yOrigine; y<height; y++) {
			x = roundf(temp);
			if(x<0)
				break;
			droite2[k2] = y*width+x;
			k2++;
			temp += 1/orientation;
		}
	}
	*length = k1 + k2;

	for (int i=0, j=k2-1; i<k2; i++, j--) {
		droite[i]=droite2[j];
	}
	for(int i=k2, j=0; i<k2+k1; i++, j++){
		droite[i]=droite1[j];
	}
	delete [] droite1;
	delete [] droite2;

	return droite;

}

int PithExtractor::floatCompare(const void *first, const void *second)
{
	// const float *fa = (const float *)first; // casting pointer types
	// const float *fb = (const float *)second;
	// Ca ne marche pas !!! : return *fa  - *fb;
	// return (*fb > *fa) ? -1: ((*fa > *fb) ? 1 : 0);
	const float va = * (const float *)first;
	const float vb = * (const float *)second;
	return (vb > va)
		? -1
		: ((va > vb) ? 1 : 0);
	/* integer comparison: returns negative if b > a
	and positive if a > b */
}

void PithExtractor::minSlice(const Slice &slice, int *minValue, int *maxValue, uiCoord2D *coordmax) const {
	const uint height = slice.n_rows-1;
	const uint width = slice.n_cols-1;

	for ( uint i=1 ; i<height ; i++ ) {
		for ( uint j=1; j<width ; j++ ) {
			const int value = slice(i,j);
			if ( value < *minValue ) {
				*minValue = value;
			}
			else if ( value > *maxValue ) {
				*maxValue = value;
				*coordmax = uiCoord2D(j,i);
			}
		}
	}
}

void PithExtractor::correctPith( QVector<rCoord2D> &moelle, float *listMax, float seuilHough ) const {
	const int pithSize = moelle.size();
	if ( pithSize < 3 ) return;
	int startSlice, endSlice, i=0, x1, y1, x2, y2, newx, newy;
	float ax, ay;
	std::cerr << "Coupes interpolées :\n";
	while(i < pithSize){
		if(listMax[i] < seuilHough){
			startSlice = i;
			i++;
			while(listMax[i]<seuilHough && i < pithSize) {i++;}
			endSlice = i-1;
			if(!startSlice){
				ax = 9999;
				ay = 9999;
				x1 = moelle[endSlice+1].x;
				y1 = moelle[endSlice+1].y;
			}else if(endSlice == pithSize-1){
				ax = 9999;
				ay = 9999;
				x1 = moelle[startSlice-1].x;
				y1 = moelle[startSlice-1].y;
			}else {
				x1 = moelle[startSlice-1].x;
				x2 = moelle[endSlice+1].x;
				y1 = moelle[startSlice-1].y;
				y2 = moelle[endSlice+1].y;
				if(x1!=x2){
					ax = (float)((endSlice+1) - (startSlice-1) ) / (x2-x1);
				}else{
					ax = 9999;
				}
				if(y1!=y2){
					ay = (float)((endSlice+1) - (startSlice-1) ) / (y2-y1);
				}else{
					ay = 9999;
				}
			}

			if (endSlice > startSlice) {
				std::cerr << startSlice << "-" << endSlice << ' ';
			} else if (endSlice == startSlice) {
				std::cerr << startSlice << ' ';
			}

			for(int j=startSlice; j<=endSlice; j++){
				if(ax != 9999){
					newx = roundf((j-(startSlice-1)+ax*x1)/ax);
				}else{
					newx = x1;
				}
				if(ay != 9999){
					newy = roundf((j-(startSlice-1)+ay*y1)/ay);
				}else{
					newy = y1;
				}
				moelle.replace(j, rCoord2D(newx, newy));
			}
		}
		i++;
	}
	std::cerr << '\n';
}
