#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include <QPixmap>

class Billon;

enum SliceType {
	CURRENT_SLICE,
	AVERAGE_SLICE,
	MEDIAN_SLICE
};

class SliceView : public QObject, public QPixmap
{
	Q_OBJECT

public:
	explicit SliceView();

	void setModel( const Billon * billon );

	int currentSlice() const;

public slots:
	void drawSlice( const int &sliceNumber = -1 );
	void setLowThreshold(const int &threshold);
	void setHighThreshold(const int &threshold);
	void setTypeOfView(const SliceType &type = CURRENT_SLICE);

signals:
	void updated(const QPixmap &);
	void thresholdUpdated();
	void typeOfViewChanged(const SliceType &);

private:
	const Billon * _billon;

	unsigned int _currentSlice;
	int _lowThreshold;
	int _highThreshold;
	int _typeOfView;
};

#endif // SLICEVIEW_H
