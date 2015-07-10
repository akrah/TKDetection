#include "inc/detection/knotareadetector.h"

KnotAreaDetector::KnotAreaDetector() {}
KnotAreaDetector::~KnotAreaDetector() {}

const QVector<QRect> &KnotAreaDetector::knotAreas() const
{
	return _knotAreas;
}

void KnotAreaDetector::clear()
{
	_knotAreas.clear();
}
