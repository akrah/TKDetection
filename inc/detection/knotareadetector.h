#ifndef KNOTAREADETECTOR_H
#define KNOTAREADETECTOR_H

#include "def/def_billon.h"

#include <QVector>
#include <QRect>

class KnotAreaDetector
{
public:
	KnotAreaDetector();
	virtual ~KnotAreaDetector();

	virtual void execute( const Billon &billon ) = 0;
	virtual void clear();

	const QVector<QRect> &knotAreas() const;


protected:
	QVector<QRect> _knotAreas;
};

#endif // KNOTAREADETECTOR_H
