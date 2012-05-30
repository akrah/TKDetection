#ifndef TEST_INTERVALSHISTOGRAM_H
#define TEST_INTERVALSHISTOGRAM_H

#include <QVector>

namespace Test_IntervalsHistogram {
	void allTests();
	void test1();
	void test2();
	void test3();
	void test4();
	void test5();

	void routineCommune( QVector<qreal> &hist, bool loop );
}

#endif // TEST_INTERVALSHISTOGRAM_H
