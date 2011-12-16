#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include <QPixmap>

class Billon;

class SliceView : public QObject, public QPixmap
{
	Q_OBJECT

public:
	explicit SliceView();

	int currentSlice() const;

	void setModel( const Billon * billon );
	void drawSlice( const int &sliceNumber );
	void drawAverageSlice();
	void drawMedianSlice();

public slots:
	void setLowThreshold(const int &threshold);
	void setHighThreshold(const int &threshold);

signals:
	void updated(const QPixmap &);
	void thresholdUpdated();

private:
	const Billon * _billon;

	unsigned int _currentSlice;
	int _lowThreshold;
	int _highThreshold;
};

#endif // SLICEVIEW_H
