#include "inc/plotconcavitypointseriecurve.h"

#include "inc/concavitypointseriecurve.h"
#include "inc/coordinate.h"
#include "inc/define.h"
#include "inc/interval.h"

PlotConcavityPointSerieCurve::PlotConcavityPointSerieCurve()
{
	_maxConcavityPointsData.setPen(QPen(Qt::green));
	_minConcavityPointsData.setPen(QPen(Qt::blue));
	_maxKnotAreaAngle.setPen(QPen(Qt::red));
	_minKnotAreaAngle.setPen(QPen(Qt::red));
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
		_maxConcavityPointsData.attach(plot);
		_minConcavityPointsData.attach(plot);
		_maxKnotAreaAngle.attach(plot);
		_minKnotAreaAngle.attach(plot);
	}
}

void PlotConcavityPointSerieCurve::clear()
{
	const QVector<QPointF> emptyData(0);
	_maxConcavityPointsData.setSamples(emptyData);
	_minConcavityPointsData.setSamples(emptyData);
	_maxKnotAreaAngle.setSamples(emptyData);
	_minKnotAreaAngle.setSamples(emptyData);
}

void PlotConcavityPointSerieCurve::update( const ConcavityPointSerieCurve &curve, const Interval<qreal> &angularInterval )
{
	QVector<QPointF> datasMaxConcavity(0);
	QVector<QPointF> datasMinConcavity(0);
	QVector<QPointF> datasMaxKnotArea(0);
	QVector<QPointF> datasMinKnotArea(0);

	const int nbMaxConcavityPoints = curve.nbMaxConcavityPoints();
	const int nbMinConcavityPoints = curve.nbMinConcavityPoints();

	if ( nbMaxConcavityPoints || nbMinConcavityPoints )
	{
		int firstX, lastX;
		if ( nbMaxConcavityPoints>0 && nbMinConcavityPoints>0 )
		{
			firstX = qMin(curve.maxConcavityPointsSerie().first().x,curve.minConcavityPointsSerie().first().x);
			lastX = qMax(curve.maxConcavityPointsSerie().last().x,curve.minConcavityPointsSerie().last().x);
		}
		else if ( nbMaxConcavityPoints>0 )
		{
			firstX = curve.maxConcavityPointsSerie().first().x;
			lastX = curve.maxConcavityPointsSerie().last().x;
		}
		else if ( nbMinConcavityPoints>0 )
		{
			firstX = curve.minConcavityPointsSerie().first().x;
			lastX = curve.minConcavityPointsSerie().last().x;
		}

		if ( curve.nbMaxConcavityPoints() > 0 )
		{
			datasMaxConcavity.reserve(curve.nbMaxConcavityPoints());
			QVector<rCoord2D>::ConstIterator begin = curve.maxConcavityPointsSerie().begin();
			const QVector<rCoord2D>::ConstIterator end = curve.maxConcavityPointsSerie().end();
			while ( begin != end )
			{
				datasMaxConcavity.append(QPointF(begin->x,begin->y));
				++begin;
			}
			datasMaxKnotArea.resize(2);
			datasMaxKnotArea[0] = QPointF( firstX, angularInterval.max()*RAD_TO_DEG_FACT );
			datasMaxKnotArea[1] = QPointF( lastX, angularInterval.max()*RAD_TO_DEG_FACT );
		}
		if ( curve.nbMinConcavityPoints() > 0 )
		{
			datasMinConcavity.reserve(curve.nbMinConcavityPoints());
			QVector<rCoord2D>::ConstIterator begin = curve.minConcavityPointsSerie().begin();
			const QVector<rCoord2D>::ConstIterator end = curve.minConcavityPointsSerie().end();
			while ( begin != end )
			{
				datasMinConcavity.append(QPointF(begin->x,begin->y));
				++begin;
			}
			datasMinKnotArea.resize(2);
			datasMinKnotArea[0] = QPointF( firstX, angularInterval.min()*RAD_TO_DEG_FACT );
			datasMinKnotArea[1] = QPointF( lastX, angularInterval.min()*RAD_TO_DEG_FACT );
		}
	}

	_maxConcavityPointsData.setSamples(datasMaxConcavity);
	_minConcavityPointsData.setSamples(datasMinConcavity);
	_maxKnotAreaAngle.setSamples(datasMaxKnotArea);
	_minKnotAreaAngle.setSamples(datasMinKnotArea);
}
