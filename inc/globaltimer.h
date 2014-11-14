#ifndef GLOBALTIMER_H
#define GLOBALTIMER_H

#include <QString>
#include <QVector>
#include <QTextStream>

#include <chrono>

struct _TimeStat_ {
	QString _name;
	uint _level;
	std::chrono::system_clock::time_point _start;
	std::chrono::system_clock::time_point _end;
	double _duration;
};

class GlobalTimer
{
private:
	GlobalTimer();
	~GlobalTimer() {}

public:
	static GlobalTimer *getInstance()
	{
		if ( !_singleton ) _singleton =  new GlobalTimer();
		return _singleton;
	}

	static void kill()
	{
		if (_singleton)
		{
			delete _singleton;
			_singleton = 0;
		}
	}

	void start(const QString &name);
	void end();
	void reset();

	const QVector< _TimeStat_ > &times() const;

	void print( QTextStream & stream );

private:

	static GlobalTimer *_singleton;
	QVector<_TimeStat_> _times;
	uint _currentLevel;
};

#endif // GLOBALTIMER_H
