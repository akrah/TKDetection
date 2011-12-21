#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include "defines.h"
#include <QPixmap>

class Billon;

class SliceView : public QObject, public QPixmap
{
	Q_OBJECT

public:
	explicit SliceView();

	void setModel( const Billon * billon );

	int currentSlice() const;

public slots:
	void drawSlice( const int &sliceNumber = -1 );
	void update();
	void setLowThreshold(const int &threshold);
	void setHighThreshold(const int &threshold);
	void setTypeOfView(const int &type = SliceType::CURRENT);

private :
	void drawCurrentSlice();
	void drawAverageSlice();
	void drawMedianSlice();

signals:
	void updated(const QPixmap &);
	void thresholdUpdated();
	void typeOfViewChanged(const SliceType::SliceType &);

private:
	const Billon * _billon;

	int _currentSlice;
	int _lowThreshold;
	int _highThreshold;
	SliceType::SliceType _typeOfView;
};

inline int SliceView::currentSlice() const {
	return _currentSlice;
}

#endif // SLICEVIEW_H
