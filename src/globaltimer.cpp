#include "inc/globaltimer.h"

#define TIMER_TITLE_WIDTH 55
#define TIMER_INDENT_WIDTH 3

GlobalTimer *GlobalTimer::_singleton = 0;

GlobalTimer::GlobalTimer() : _currentLevel(1)
{
}

void GlobalTimer::start(const QString &name)
{
	_times.append(_TimeStat_());
	_times.last()._name = name;
	_times.last()._level = _currentLevel++;
	_times.last()._start = std::chrono::system_clock::now();
}

void GlobalTimer::end()
{
	std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
	if ( !_currentLevel ) return;
	_currentLevel--;
	QVector<_TimeStat_>::Iterator statIter = _times.end();
	while ( statIter != _times.constBegin() && statIter->_level != _currentLevel ) statIter--;
	_TimeStat_ &endingStat = *statIter;
	endingStat._end = endTime;
	endingStat._duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - endingStat._start).count();
}

void GlobalTimer::reset()
{
	_times.clear();
	_currentLevel = 1;
}

const QVector< _TimeStat_ > &GlobalTimer::times() const
{
	return _times;
}

void GlobalTimer::print( QTextStream & stream )
{
	if (_times.isEmpty()) return;
	QVector<_TimeStat_> ::ConstIterator timerIter = _times.constBegin();
	uint currentLevel = _times.first()._level;
	QVector<double> subTotals;
	while( timerIter != _times.constEnd() )
	{
		const _TimeStat_ &stat = *timerIter;
		if ( stat._level == currentLevel )
		{
			subTotals.last() += stat._duration;
		}
		else if ( stat._level > currentLevel )
		{
			subTotals.append(stat._duration);
			currentLevel = stat._level;
		}
		else {
			double subTotal = subTotals.last();
			subTotals.pop_back();
			currentLevel--;
			stream << qSetFieldWidth(TIMER_TITLE_WIDTH) << right << "Total " << qSetFieldWidth(8) << right << subTotal << " ms" << endl;
		}
		stream << qSetFieldWidth(TIMER_INDENT_WIDTH*(currentLevel-1)) << "" << qSetFieldWidth(TIMER_TITLE_WIDTH-TIMER_INDENT_WIDTH*(currentLevel-1)) << left << stat._name << qSetFieldWidth(8) << right << stat._duration << " ms" << endl;
		timerIter++;
	}
}
