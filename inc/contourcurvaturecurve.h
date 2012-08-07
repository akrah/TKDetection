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

	const QVector<iCoord2D> &curvatureEdges() const;

	void constructCurve( const Billon &billon, const Marrow *marrow, const int &sliceNumber, const int &componentNumber );

private:
	QwtPlotCurve _curveCurvature;
	QwtPlotCurve _curveCurrentPosition;

	QVector<qreal> _datasCurvature;
	QVector<iCoord2D> _datasEdges;
};

#endif // CONTOURCURVATURECURVE_H
