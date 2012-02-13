#include "inc/marrowextractor.h"

#include "inc/marrow.h"
#include "inc/marrowextractor_def.h"
#include "inc/global.h"

MarrowExtractor::MarrowExtractor() :
	_falseCutPercent(MarrowExtractorDefaultsParameters::FALSE_CUT_PERCENT),
	_windowWidth(MarrowExtractorDefaultsParameters::NEIGHBORHOOD_WINDOW_WIDTH),
	_windowHeight(MarrowExtractorDefaultsParameters::NEIGHBORHOOD_WINDOW_HEIGHT),
	_binarizationThreshold(MarrowExtractorDefaultsParameters::BINARIZATION_THRESHOLD),
	_marrowLag(MarrowExtractorDefaultsParameters::MARROW_LAG)
{
}

/********************************************************
 * Fonction principale d'extraction de la moelle
 ********************************************************/

Marrow* MarrowExtractor::process( const icube &image, int sliceMin, int sliceMax ) {
	const int width = image.n_cols;
	const int height = image.n_rows;
	const int depth = image.n_slices;
	sliceMin = RESTRICT_TO_INTERVAL(sliceMin,0,depth-1);
	sliceMax = RESTRICT_TO_INTERVAL(sliceMax,sliceMin,depth-1);

	Coord2D coordPrec,coordCurrent;
	float maxStandList[depth];
	float *maxStandList2 = 0;
	float shift,houghStandThreshold;
	int max,x,y, nbContourPoints;

	max = x = y = 0;

	Marrow *marrow = new Marrow(sliceMin,sliceMax);

	//extraction de la moelle de la premiere coupe sur la totalité de la coupe
	//nous permet d'avoir un coordonnée a laquelle appliqué la fenetre
	transHough( image.slice(sliceMin), width, height, &x, &y, &max, &nbContourPoints );



	//calcul la coordonnée d'origine de la fenetre
	x = x - (_windowWidth/2);
	x = x < 0 ? 0 : (x > (width - _windowWidth) ? width - _windowWidth : x);
	y = y - (_windowHeight/2);
	y = y < 0 ? 0 : (y > (height - _windowHeight) ? height - _windowHeight : y);
	//extraction de la moelle de la premiere coupe sur la coupe redimensionnée
	coordCurrent = transHough( image.slice(sliceMin), _windowWidth, _windowHeight, &x, &y, &max, &nbContourPoints );

	for (int i = 0; i < sliceMin; ++ i) {
		marrow->append(coordCurrent);
		maxStandList[i]=0.;
	}
	marrow->append(coordCurrent);	// pour sliceMin
	maxStandList[sliceMin] = ((float)max)/nbContourPoints;

	//extraction des coupes suivantes
	std::cerr << "extractMoelle : ";
	for(int i=sliceMin + 1; i<= sliceMax; i++) {
		//~ if(!listeCoupe->contains(i)){
			std::cerr << " " << i;
			coordPrec = marrow->last();
			x = x - (_windowWidth/2);
			x = x < 0 ? 0 : (x > (width - _windowWidth) ? width - _windowWidth : x);
			y = y - (_windowHeight/2);
			y = y < 0 ? 0 : (y > (height - _windowHeight) ? height - _windowHeight : y);
			//extraction de la moelle de la coupe i
			coordCurrent = transHough( image.slice(i), _windowWidth, _windowHeight, &x, &y, &max, &nbContourPoints );
			marrow->append(coordCurrent);
			shift = sqrt(pow((double) (marrow->last().x - coordPrec.x),(double) 2.0) + pow( (double)(marrow->last().y - coordPrec.y), (double)2.0) );
			//si le resultat obtenu a un decalage trop important avec la coupe precedente alors on recommence l'extraction sur l'ensemble de la coupe
			if(shift > _marrowLag && width > _windowWidth && height > _windowHeight){
				std::cerr << "*";
				// std::cerr << "   :> decalage=" << decalage << ">" << _marrowLag << "\n";

				marrow->removeLast();
				x = y = 0;
				transHough(image.slice(i), width, height, &x, &y, &max, &nbContourPoints );
				x = x - (_windowWidth/2);
				x = x < 0 ? 0 : (x > (width - _windowWidth) ? width - _windowWidth : x);
				y = y - (_windowHeight/2);
				y = y < 0 ? 0 : (y > (height - _windowHeight) ? height - _windowHeight : y);
				coordCurrent = transHough( image.slice(i), _windowWidth, _windowHeight, &x, &y, &max, &nbContourPoints );
				marrow->append(coordCurrent);
			}
			maxStandList[i] = ((float)max)/nbContourPoints;
			if (i % 20 == 0) {
				std::cerr << std::endl;
			}
		//~ }else{
			//~ listMoelle->append(listMoelle->last());
			//~ listMaxStand[i] = 0;
		//~ }
	}
	std::cerr << "\n";

	coordCurrent = marrow->last();
	for ( int i = sliceMax+1 ; i<depth ; i++ ) {
		marrow->append(coordCurrent);
		maxStandList[i]=0.;
	}

	std::cerr << "nbCoupes=" << depth << " listMoelle.size()=" << marrow->size() << "\n";

	//calcul du seuil a partir du quel les coupes sont considerées comme erronées
	maxStandList2 = new float[depth];
	memcpy(maxStandList2, maxStandList, depth*sizeof(float));

	qsort(maxStandList2, depth, sizeof(float), &floatCompare);

	int nfc = (_falseCutPercent*depth)/100;
	houghStandThreshold = nfc > 0 ? (maxStandList2[nfc]+maxStandList2[nfc-1])/2 : maxStandList2[nfc];

	delete [] maxStandList2;

	// applique la coorection a la moelle
	correctMarrow(*marrow, maxStandList, houghStandThreshold);

	return marrow;
}

/******************************************************************
 * Fonctions secondaires appelées lors de l'extraction de la moelle
 ******************************************************************/
Coord2D MarrowExtractor::transHough(const imat &slice, int width, int height, int *x, int *y, int *sliceMaxValue, int *nbContourPoints) {
	int x_accu, y_accu, longueur, min;
	int *droite;
	imat *tabaccu;
	fmat *orientation, *cont;
	Coord2D coordmax;
	orientation = 0;
	{ // bloc de limitation de vie de la variable voisinage
		// attention x represente les colonne et y les lignes
		const imat voisinage = slice.submat( *y, *x, (uint)( (*y)+height-1 ), (uint)( (*x)+width-1 ) );
		cont = contour(voisinage, &orientation);
	}
	tabaccu = new imat(width, height);
	tabaccu->zeros();
	//verifie orientation et table d'accumlation tous les points se trouvent a 0
	*nbContourPoints = 0;
	for(uint i=0; i<cont->n_rows; i++){
		for(uint j=0; j<cont->n_cols; j++){
			if(cont->at(i,j) == 1){
				*nbContourPoints += 1;
				droite = drawLine(j, i, orientation->at(i,j), width, height, &longueur);
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
	min = *sliceMaxValue = tabaccu->at(0,0);
	coordmax.x = coordmax.y = 0;

	minSlice(*tabaccu, &min, sliceMaxValue, &coordmax);

	delete tabaccu;

	*x += coordmax.x;
	*y += coordmax.y;

	return Coord2D(*x, *y);
}

fmat * MarrowExtractor::contour(const imat &slice, fmat **orientation) {
	const uint height = slice.n_rows;
	const uint width = slice.n_cols;

	fcolvec filtre1 = "1 2 1";			//filtre Sobel Gx
	frowvec filtre2 = "-1 0 1";
	imat *resultatGX, *resultatGY;
	fmat norm = fmat(height, width);
	fmat *contour = new fmat(height, width);

	*orientation = new fmat(height, width);
	(*orientation)->zeros();
	resultatGX = convolution(slice, filtre1, filtre2);		//convolution Sobel GX
	filtre1 = "1 0 -1";										//filtre Sobel Gy
	filtre2 = "1 2 1";
	resultatGY = convolution(slice, filtre1, filtre2);		//convolution Sobel GY

	for (uint i=0; i<height; i++) {
		for (uint j=0; j<width; j++) {
			norm.at(i,j) = sqrt(resultatGX->at(i,j)*resultatGX->at(i,j)+resultatGY->at(i,j)*resultatGY->at(i,j))/4;
			if (norm.at(i,j)>_binarizationThreshold && i>0 && i<norm.n_rows-1 && j>0 && j<norm.n_cols-1 ) {
				contour->at(i,j) = 1;
				if(resultatGX->at(i,j) != 0) {
					// Fred : pas de raison de faire une division entière ici :
					// (*orientation)->at(i,j) = resultatGY->at(i,j) / resultatGX->at(i,j);	//tangente teta
					(*orientation)->at(i,j) = double (resultatGY->at(i,j)) / resultatGX->at(i,j);	//tangente teta
				}
			} else {
				contour->at(i,j) = 0;
			}
		}
	}
	delete resultatGX;
	delete resultatGY;

	return contour;
}

imat * MarrowExtractor::convolution(const imat &slice, fcolvec verticalFilter, frowvec horizontalFilter) {
	const uint height = slice.n_rows;
	const uint width = slice.n_cols;
	imat *resultat = new imat(height, width);

	fmat tmpSum;
	fvec sum = fvec(width);
	resultat->zeros();
	int kOffset;
	int kCenter;
	int lag;
	int endIndex;

	//convolve horizontal direction

	//find center position of kernel
	kCenter = horizontalFilter.n_cols/2;
	endIndex = width-kCenter;

	tmpSum = fmat(height, width);
	tmpSum.zeros();
	for (uint i=0; i<height; i++) { //height pictures (nb rows)
		kOffset = 0;
		//index=0 to index=kCenter-1
		for (int j=0; j<kCenter; j++) {
			for (int k=kCenter+kOffset, m=0; k>=0; k--, m++) {
				tmpSum(i,j) += slice.at(i,m) * horizontalFilter[k];
			}
			kOffset++;
		}
		//index=kCenter to index=(width pictures)-kCenter-1)
		for (int j=kCenter; j<endIndex; j++) {
			lag = j-kCenter;
			for (int k=horizontalFilter.n_cols-1, m=0; k >= 0; k--, m++) {
				tmpSum(i,j) += slice.at(i,(lag+m)) * horizontalFilter[k];
			}
		}
		//index=(width pictures)-kCenter to index=(width pictures)-1
		kOffset = 1;
		for (uint j=endIndex; j<width; j++) {
			lag = j-kCenter;
			for (int k=horizontalFilter.n_cols-1, m=0; k >= kOffset; k--, m++) {
				tmpSum(i,j) += slice.at(i,lag+m) * horizontalFilter[k];
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
				resultat->at(i,n) = (int)(sum[n] + 0.5f);
			else
				resultat->at(i,n) = (int)(sum[n] - 0.5f);
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
				resultat->at(i,n) = (int)(sum[n] + 0.5f);
			else
				resultat->at(i,n) = (int)(sum[n] - 0.5f);
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
				resultat->at(i,n) = (int)(sum[n] + 0.5f);
			else
				resultat->at(i,n) = (int)(sum[n] - 0.5f);
			sum[n] = 0;											//reset before next summing
		}
		kOffset++;
	}

	return resultat;
}


int * MarrowExtractor::drawLine(int xOrigine, int yOrigine, float orientation_ORI, int width, int height, int *length) {
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

int MarrowExtractor::floatCompare(const void *first, const void *second)
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

void MarrowExtractor::minSlice(const imat &slice, int *minValue, int *maxValue, Coord2D *coordmax) {
	const uint height = slice.n_rows-1;
	const uint width = slice.n_cols-1;

	for ( uint i=1 ; i<height ; i++ ) {
		for ( uint j=1; j<width ; j++ ) {
			const int value = slice.at(i,j);
			if ( value < *minValue ) {
				*minValue = value;
			}
			else if ( value > *maxValue ) {
				*maxValue = value;
				*coordmax = Coord2D(j,i);
			}
		}
	}
}

void MarrowExtractor::correctMarrow( QList<Coord2D> &moelle, float *listMax, float seuilHough ) {
	const int marrowSize = moelle.size();
	int startSlice, endSlice, i=0, x1, y1, x2, y2, newx, newy;
	float ax, ay;
	std::cerr << "Coupes interpolées :\n";
	while(i < marrowSize && marrowSize >2){
		if(listMax[i] < seuilHough){
			startSlice = i;
			i++;
			while(listMax[i]<seuilHough && i < marrowSize) {i++;}
			endSlice = i-1;
			if(startSlice == 0){
				ax = 9999;
				ay = 9999;
				x1 = moelle.at(endSlice+1).x;
				y1 = moelle.at(endSlice+1).y;
			}else if(endSlice == marrowSize-1){
				ax = 9999;
				ay = 9999;
				x1 = moelle.at(startSlice-1).x;
				y1 = moelle.at(startSlice-1).y;
			}else {
				x1 = moelle.at(startSlice-1).x;
				x2 = moelle.at(endSlice+1).x;
				y1 = moelle.at(startSlice-1).y;
				y2 = moelle.at(endSlice+1).y;
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
				moelle.replace(j, Coord2D(newx, newy));
			}
		}
		i++;
	}
	std::cerr << '\n';
}
