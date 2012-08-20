#ifndef CONTOURCURVATURECURVE_H
#define CONTOURCURVATURECURVE_H

#include "billon_def.h"
#include "marrow_def.h"
#include <qwt_plot_curve.h>

class Marrow;

class ContourCurvatureCurve
{
public:
	ContourCurvatureCurve();
	~ContourCurvatureCurve();

	void attach( QwtPlot * const plot );
	void detach();
	void clear();
	void setCurvePosition( const int &position );

<<<<<<< HEAD
	const QVector<iCoord2D> &contourPoints() const;
	const QVector<iCoord2D> &dominantPoints() const;
	QVector<iCoord2D> mainDominantPoints() const;

	void constructCurve( const Billon &billon, const Marrow *marrow, const int &sliceNumber, const int &componentNumber, const int &blurredSegmentThickness, const iCoord2D &startPoint = iCoord2D(-1,-1) );
=======
	const QVector<iCoord2D> &curvatureEdges() const;

	void constructCurve( const Billon &billon, const Marrow *marrow, const int &sliceNumber, const int &componentNumber );
>>>>>>> d51d019e6f6e4c1b23ddfd74c670e687d428b1ff

private:
	QwtPlotCurve _curveCurvature;
	QwtPlotCurve _curveCurrentPosition;

	QVector<qreal> _datasCurvature;
<<<<<<< HEAD
	QVector<iCoord2D> _datasContourPoints;
	QVector<iCoord2D> _datasDominantPoints;
=======
	QVector<iCoord2D> _datasEdges;
>>>>>>> d51d019e6f6e4c1b23ddfd74c670e687d428b1ff
};

#endif // CONTOURCURVATURECURVE_H
