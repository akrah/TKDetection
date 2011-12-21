#include "inc/marrowextractor.h"

MarrowExtractor::MarrowExtractor() {
	falseCutPercent = FALSE_CUT_PERCENT;
	windowWidth = NEIGHBORHOOD_WINDOW_WIDTH;
	windowHeight = NEIGHBORHOOD_WINDOW_HEIGHT;
	binarizationThreshold = BINARIZATION_THRESHOLD;
	marrowLag = MARROW_LAG;
}

/********************************************************
 * Get functions
 ********************************************************/
int MarrowExtractor::getFalseCutPercent() {
	return falseCutPercent;
}

int MarrowExtractor::getWindowWidth() {
	return windowWidth;
}

int MarrowExtractor::getWindowHeight() {
	return windowHeight;
}

int MarrowExtractor::getBinarizationThreshold() {
	return binarizationThreshold;
}

int MarrowExtractor::getMarrowLag() {
	return marrowLag;
}

/********************************************************
 * Set functions
 ********************************************************/
void MarrowExtractor::setFalseCutPercent( int percentage ) {
	falseCutPercent = percentage;
}

void MarrowExtractor::setWindowWidth( int width ) {
	windowWidth = width;
}

void MarrowExtractor::setWindowHeight( int height ) {
	windowHeight = height;
}

void MarrowExtractor::setBinarizationThreshold( int threshold ) {
	binarizationThreshold = threshold;
}

void MarrowExtractor::setMarrowLag( int lag ) {
	marrowLag = lag;
}

/********************************************************
 * Fonction principale d'extraction de la moelle
 ********************************************************/

RegressiveElementLinearly * MarrowExtractor::extractMoelle( icube *matrix, QList<int> *listeCoupe,
		int coupe_min, int coupe_max)
{
	// ARRANGER : à quoi sert listeCoupe ?

	RegressiveElementLinearly *listMoelle = new RegressiveElementLinearly();
	float listMaxStand[matrix->n_slices], *listMaxStand2;
	Coord coordprec,coordcurrent;
	int max,x,y,z,xprec,yprec,width,height, nbptcontour;
	float decalage,seuilhoughstand;
	width = matrix->n_cols;
	height = matrix->n_rows;
	max=x=y=0;

	coupe_min = std::min (std::max (coupe_min, 0), int (matrix->n_slices) - 1);
	coupe_max = std::min (std::max (coupe_max, coupe_min), int (matrix->n_slices) - 1);
	z = coupe_min;

	//extraction de la moelle de la premiere coupe sur la totalité de la coupe
	//nous permet d'avoir un coordonnée a laquelle appliqué la fenetre
	transHough( &( matrix->slice(coupe_min) ), width, height, &x, &y, &z, &max, &nbptcontour );



	//calcul la coordonnée d'origine de la fenetre
	x = x - (windowWidth/2);
	x = x < 0 ? 0 : (x > (width - windowWidth) ? width - windowWidth : x);
	y = y - (windowHeight/2);
	y = y < 0 ? 0 : (y > (height - windowHeight) ? height - windowHeight : y);
	//extraction de la moelle de la premiere coupe sur la coupe redimensionnée
	coordcurrent = transHough( &( matrix->slice(coupe_min) ), windowWidth, windowHeight, &x, &y, &z, &max, &nbptcontour );

	for (int i = 0; i < coupe_min; ++ i) {
		listMoelle->append(coordcurrent);
		listMaxStand[i]=0.;
	}
	listMoelle->append(coordcurrent);	// pour coupe_min
	listMaxStand[coupe_min] = ((float)max)/nbptcontour;

	//extraction des coupes suivantes
	std::cerr << "extractMoelle : ";
	for(int i=coupe_min + 1; i<= coupe_max; i++){
		//~ if(!listeCoupe->contains(i)){
			std::cerr << " " << i;
			z=i;
			xprec = x;
			yprec = y;
			coordprec = listMoelle->last();
			x = x - (windowWidth/2);
			x = x < 0 ? 0 : (x > (width - windowWidth) ? width - windowWidth : x);
			y = y - (windowHeight/2);
			y = y < 0 ? 0 : (y > (height - windowHeight) ? height - windowHeight : y);
			//extraction de la moelle de la coupe i
			coordcurrent = transHough( &( matrix->slice(i) ), windowWidth, windowHeight, &x, &y, &z, &max, &nbptcontour );
			listMoelle->append(coordcurrent);
			decalage = sqrt(pow((double) (listMoelle->last().x - coordprec.x),(double) 2.0) + pow( (double)(listMoelle->last().y - coordprec.y), (double)2.0) );
			//si le resultat obtenu a un decalage trop important avec la coupe precedente alors on recommence l'extraction sur l'ensemble de la coupe
			if(decalage > marrowLag && width > windowWidth && height > windowHeight){
				std::cerr << "*";
				// std::cerr << "   :> decalage=" << decalage << ">" << marrowLag << "\n";

				listMoelle->removeLast();
				x = y = 0;
				transHough( &( matrix->slice(i) ), width, height, &x, &y, &z, &max, &nbptcontour );
				x = x - (windowWidth/2);
				x = x < 0 ? 0 : (x > (width - windowWidth) ? width - windowWidth : x);
				y = y - (windowHeight/2);
				y = y < 0 ? 0 : (y > (height - windowHeight) ? height - windowHeight : y);
				coordcurrent = transHough( &( matrix->slice(i) ), windowWidth, windowHeight, &x, &y, &z, &max, &nbptcontour );
				listMoelle->append(coordcurrent);
			}
			listMaxStand[i] = ((float)max)/nbptcontour;
			if (i % 20 == 0) {
				std::cerr << std::endl;
			}
		//~ }else{
			//~ listMoelle->append(listMoelle->last());
			//~ listMaxStand[i] = 0;
		//~ }
	}
	std::cerr << "\n";

	coordcurrent = listMoelle->last();
	for (int i = coupe_max +1; i < int (matrix->n_slices); ++ i) {
		listMoelle->append(coordcurrent);
		listMaxStand[i]=0.;
	}

	std::cerr << "nbCoupes=" << matrix->n_slices << " listMoelle.size()=" << listMoelle->size() << "\n";

	//calcul du seuil a partir du quel les coupes sont considerées comme erronées
	listMaxStand2 = new float[matrix->n_slices];
	memcpy(listMaxStand2, listMaxStand, matrix->n_slices*sizeof(float));

	qsort(listMaxStand2, matrix->n_slices, sizeof(float), &float_cmp);

	int nfc = (falseCutPercent*matrix->n_slices)/100;
	seuilhoughstand = nfc > 0 ? (listMaxStand2[nfc]+listMaxStand2[nfc-1])/2 : listMaxStand2[nfc];

	delete [] listMaxStand2;

	// applique la coorection a la moelle
	corrige_moelle(listMoelle, listMaxStand, seuilhoughstand);

	return listMoelle;
}

/******************************************************************
 * Fonctions secondaires appelées lors de l'extraction de la moelle
 ******************************************************************/
Coord MarrowExtractor::transHough(imat *coupe, int width, int height, int *x, int *y, int *z, int *max, int *nbptcontour) {
	int x_accu, y_accu, longueur, min;
	int *droite;
	imat *tabaccu;
	fmat *orientation, *cont;
	Coord coordmax;
	orientation = 0;
	{ // bloc de limitation de vie de la variable voisinage
		// attention x represente les colonne et y les lignes
		imat voisinage = coupe->submat( *y, *x, (unsigned int)( (*y)+height-1 ), (unsigned int)( (*x)+width-1 ) );
		cont = contour(&voisinage, &orientation);
	}
	tabaccu = new imat(width, height);
	tabaccu->zeros();
	//verifie orientation et table d'accumlation tous les points se trouvent a 0
	*nbptcontour = 0;
	for(unsigned int i=0; i<cont->n_rows; i++){
		for(unsigned int j=0; j<cont->n_cols; j++){
			if(cont->at(i,j) == 1){
				*nbptcontour += 1;
				droite = trace_droite(j, i, orientation->at(i,j), width, height, &longueur);
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
	min = *max = tabaccu->at(0,0);
	coordmax.x = coordmax.y = 0;

	minMatrix(tabaccu, &min, max, &coordmax);

	delete tabaccu;
	*x += coordmax.x;
	*y += coordmax.y;

	return (Coord){*x, *y, *z};
}

fmat * MarrowExtractor::contour(imat *coupe, fmat **orientation) {
	fcolvec filtre1 = "1 2 1";			//filtre Sobel Gx
	frowvec filtre2 = "-1 0 1";
	imat *resultatGX, *resultatGY;
	fmat norm = fmat(coupe->n_rows, coupe->n_cols);
	fmat *contour = new fmat(coupe->n_rows, coupe->n_cols);

	*orientation = new fmat(coupe->n_rows, coupe->n_cols);
	(*orientation)->zeros();
	resultatGX = convolution(coupe, filtre1, filtre2);		//convolution Sobel GX
	filtre1 = "1 0 -1";										//filtre Sobel Gy
	filtre2 = "1 2 1";
	resultatGY = convolution(coupe, filtre1, filtre2);		//convolution Sobel GY

	for (unsigned int i=0; i<norm.n_rows; i++) {
		for (unsigned int j=0; j<norm.n_cols; j++) {
			norm(i,j) = sqrt(resultatGX->at(i,j)*resultatGX->at(i,j)+resultatGY->at(i,j)*resultatGY->at(i,j))/4;
			if (norm(i,j)>binarizationThreshold && i>0 && i<norm.n_rows-1 && j>0 && j<norm.n_cols-1 ) {
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

imat * MarrowExtractor::convolution(imat *pictures, fcolvec filtre1, frowvec filtre2) {
	imat *resultat = new imat(pictures->n_rows, pictures->n_cols);
	fmat tmpSum;
	fvec sum = fvec(pictures->n_cols);
	resultat->zeros();
	int kOffset;
	int kCenter;
	int lag;
	int endIndex;

	//convolve horizontal direction

	//find center position of kernel
	kCenter = filtre2.n_cols/2;
	endIndex = pictures->n_cols-kCenter;

	tmpSum = fmat(pictures->n_rows, pictures->n_cols);
	tmpSum.zeros();
	for (unsigned int i=0; i<pictures->n_rows; i++) { //height pictures (nb rows)
		kOffset = 0;
		//index=0 to index=kCenter-1
		for (int j=0; j<kCenter; j++) {
			for (int k=kCenter+kOffset, m=0; k>=0; k--, m++) {
				tmpSum(i,j) += pictures->at(i,m) * filtre2[k];
			}
			kOffset++;
		}
		//index=kCenter to index=(width pictures)-kCenter-1)
		for (int j=kCenter; j<endIndex; j++) {
			lag = j-kCenter;
			for (int k=filtre2.n_cols-1, m=0; k >= 0; k--, m++) {
				tmpSum(i,j) += pictures->at(i,(lag+m)) * filtre2[k];
			}
		}
		//index=(width pictures)-kCenter to index=(width pictures)-1
		kOffset = 1;
		for (unsigned int j=endIndex; j<pictures->n_cols; j++) {
			lag = j-kCenter;
			for (int k=filtre2.n_cols-1, m=0; k >= kOffset; k--, m++) {
				tmpSum(i,j) += pictures->at(i,lag+m) * filtre2[k];
			}
			kOffset++;
		}
	}


	//convolve vertical direction

	//find center position of kernel
	kCenter = filtre1.n_rows/2;
	endIndex = pictures->n_cols-kCenter;

	sum.zeros();

	kOffset = 0;
	//index=0 to index=(kCenter-1)
	for (int i=0; i <kCenter; i++) {
		lag = -i;
		for (int k = kCenter + kOffset; k>=0; k--) {			//convolve with partial kernel
			for (unsigned int j=0; j<pictures->n_cols; j++) {
				sum[j] += tmpSum(i+lag,j) * filtre1[k];
			}
			lag++;
		}
		for (unsigned int n=0; n<pictures->n_cols; n++) {				//convert to output format
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
		for (int k=filtre1.n_rows-1; k>=0; k--) {				//convolve with full kernel
			for(unsigned int j=0; j<pictures->n_cols; j++){
				sum[j] += tmpSum(i+lag,j) * filtre1[k];
			}
			lag++;
		}
		for (unsigned int n=0; n<pictures->n_cols; n++) {				//convert to output format
			if(sum[n]>=0)
				resultat->at(i,n) = (int)(sum[n] + 0.5f);
			else
				resultat->at(i,n) = (int)(sum[n] - 0.5f);
			sum[n] = 0;											//reset before next summing
		}
	}
	//index=(height pictures)-kcenter to index=(height pictures)-1
	kOffset = 1;
	for (unsigned int i=endIndex; i<pictures->n_rows; i++) {
		lag = -kCenter;
		for (int k=filtre1.n_rows-1; k>=kOffset; k--) {			//convolve with partial kernel
			for (unsigned int j=0; j<pictures->n_cols; j++) {			//height p	corrigeictures
				sum[j] += tmpSum(i+lag,j) * filtre1[k];
			}
			lag++;
		}
		for (unsigned int n=0; n<pictures->n_cols; n++) {				//convert to output format
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


int * MarrowExtractor::trace_droite(int x_origine, int y_origine, float orientation_ORI, int width, int height, int *longueur) {
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
		temp = y_origine;
		for(x=x_origine; x<width; x++){
			 y= arrondi(temp);
			 if(y>=height)
				 break;
			 droite1[k1] = y*width+x;
			 k1++;
			 temp += orientation;
		}

		//5e octant
		temp = y_origine - orientation;
		for(x=x_origine-1; x>=0; x--){
			y = arrondi(temp);
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
		temp = y_origine;
		for(x=x_origine; x<width; x++){
			y=arrondi(temp);
			if(y<0)
				break;
			droite1[k1] = y*width+x;
			k1++;
			temp += orientation;
		}

		//4e octant
		temp = y_origine-orientation;
		for(x=x_origine-1; x>=0; x--){
			y = arrondi(temp);
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
		temp = x_origine;
		for(y=y_origine; y<height; y++){
			x = arrondi(temp);
			if(x>=width)
				break;
			droite1[k1] = y*width+x;
			k1++;
			temp += (1/orientation);
		}

		//6e octant
		temp=x_origine-(1/orientation);
		for(y=y_origine-1; y>=0; y--){
			x = arrondi(temp);
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
		temp = x_origine-(1/orientation);
		for (y=y_origine-1; y>=0; y--) {
			x = arrondi(temp);
			if(x>=width)
				break;
			droite1[k1] = y*width+x;
			k1++;
			temp -= 1/orientation;
		}
		//3e octant
		temp = x_origine;
		for (y=y_origine; y<height; y++) {
			x = arrondi(temp);
			if(x<0)
				break;
			droite2[k2] = y*width+x;
			k2++;
			temp += 1/orientation;
		}
	}
	*longueur = k1 + k2;

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

inline
int MarrowExtractor::arrondi(float x) {
	return roundf (x);
}

void MarrowExtractor::minMatrix(imat *matrix, int *min, int *max, Coord *coordmax) {
	for(unsigned int i=1; i<matrix->n_rows-1; i++){
		for(unsigned int j=1; j<matrix->n_cols-1; j++){
			if(matrix->at(i,j) < *min){
				*min = matrix->at(i,j);
			}else if(matrix->at(i,j) > *max){
				*max = matrix->at(i,j);
				*coordmax = (Coord){j,i,0};
			}
		}
	}
}

int MarrowExtractor::float_cmp(const void *a, const void *b)
{
	// const float *fa = (const float *)a; // casting pointer types
	// const float *fb = (const float *)b;
	// Ca ne marche pas !!! : return *fa  - *fb;
	// return (*fb > *fa) ? -1: ((*fa > *fb) ? 1 : 0);
	const float va = * (const float *)a;
	const float vb = * (const float *)b;
	return (vb > va)
		? -1
		: ((va > vb) ? 1 : 0);
	/* integer comparison: returns negative if b > a
	and positive if a > b */
}

void MarrowExtractor::corrige_moelle(RegressiveElementLinearly *moelle, float *listMax, float seuilHough ) {
	int coupedebut, coupefin, i=0, x1, y1, x2, y2, newx, newy;
	float ax, ay;
	std::cerr << "Coupes interpolées :\n";
	while(i < moelle->size() && moelle->size() >2){
		if(listMax[i] < seuilHough){
			coupedebut = i;
			i++;
			while(listMax[i]<seuilHough && i < moelle->size()) {i++;}
			coupefin = i-1;
			if(coupedebut == 0){
				ax = 9999;
				ay = 9999;
				x1 = moelle->at(coupefin+1).x;
				y1 = moelle->at(coupefin+1).y;
			}else if(coupefin == moelle->size()-1){
				ax = 9999;
				ay = 9999;
				x1 = moelle->at(coupedebut-1).x;
				y1 = moelle->at(coupedebut-1).y;
			}else {
				x1 = moelle->at(coupedebut-1).x;
				x2 = moelle->at(coupefin+1).x;
				y1 = moelle->at(coupedebut-1).y;
				y2 = moelle->at(coupefin+1).y;
				if(x1!=x2){
					ax = (float)((coupefin+1) - (coupedebut-1) ) / (x2-x1);
				}else{
					ax = 9999;
				}
				if(y1!=y2){
					ay = (float)((coupefin+1) - (coupedebut-1) ) / (y2-y1);
				}else{
					ay = 9999;
				}
			}

			if (coupefin > coupedebut) {
				std::cerr << coupedebut << "-" << coupefin << ' ';
			} else if (coupefin == coupedebut) {
				std::cerr << coupedebut << ' ';
			}

			for(int j=coupedebut; j<=coupefin; j++){
				if(ax != 9999){
					newx = arrondi((j-(coupedebut-1)+ax*x1)/ax);
				}else{
					newx = x1;
				}
				if(ay != 9999){
					newy = arrondi((j-(coupedebut-1)+ay*y1)/ay);
				}else{
					newy = y1;
				}
				moelle->replace(j, (Coord){newx, newy, moelle->at(j).z});
			}
		}
		i++;
	}
	std::cerr << '\n';
}
