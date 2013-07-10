#include "inc/plotconcavitypointseriecurve.h"

#include "inc/concavitypointseriecurve.h"
#include "inc/coordinate.h"

PlotConcavityPointSerieCurve::PlotConcavityPointSerieCurve()
{
	_leftConcavityPointsData.setPen(QPen(Qt::green));
	_rightConcavityPointsData.setPen(QPen(Qt::blue));
}

PlotConcavityPointSerieCurve::~PlotConcavityPointSerieCurve()
{
}

/*******************************
 * Public setters
 *******************************/

void PlotConcavityPointSerieCurve::attach( QwtPlot * const plot )
{
	if ( plot )
	{
		_leftConcavityPointsData.attach(plot);
		_rightConcavityPointsData.attach(plot);
	}
}

void PlotConcavityPointSerieCurve::clear()
{
	const QVector<QPointF> emptyData(0);
	_leftConcavityPointsData.setSamples(emptyData);
	_rightConcavityPointsData.setSamples(emptyData);
}

void PlotConcavityPointSerieCurve::update( const ConcavityPointSerieCurve &curve )
{
	QVector<QPointF> datasLeftConcavity(0);
	QVector<QPointF> datasRightConcavity(0);
	if ( curve.nbLeftConcavityPoints() > 0 )
	{
		datasLeftConcavity.reserve(curve.nbLeftConcavityPoints());
		QVector<rCoord2D>::ConstIterator begin = curve.leftConcavityPointsSerie().begin();
		const QVector<rCoord2D>::ConstIterator end = curve.leftConcavityPointsSerie().end();
		while ( begin != end )
		{
			datasLeftConcavity.append(QPointF(begin->x,begin->y));
			++begin;
		}
	}
	if ( curve.nbRightConcavityPoints() > 0 )
	{
		datasRightConcavity.reserve(curve.nbRightConcavityPoints());
		QVector<rCoord2D>::ConstIterator begin = curve.rightConcavityPointsSerie().begin();
		const QVector<rCoord2D>::ConstIterator end = curve.rightConcavityPointsSerie().end();
		while ( begin != end )
		{
			datasRightConcavity.append(QPointF(begin->x,begin->y));
			++begin;
		}
	}
	_leftConcavityPointsData.setSamples(datasLeftConcavity);
	_rightConcavityPointsData.setSamples(datasRightConcavity);

}
