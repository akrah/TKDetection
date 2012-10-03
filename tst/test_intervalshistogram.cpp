#include "tst/test_intervalshistogram.h"

#include "inc/histogram.h"
#include "inc/define.h"

#include <iostream>

namespace {
	void coutHistogram( QVector<qreal>::const_iterator begin, QVector<qreal>::const_iterator end ) {
		while ( begin != end ) {
			std::cout << *begin++ <<  ' ';
		}
		std::cout << std::endl;
	}
	void coutHistogram( QVector<int>::const_iterator begin, QVector<int>::const_iterator end ) {
		while ( begin != end ) {
			std::cout << *begin++ <<  ' ';
		}
		std::cout << std::endl;
	}
	void coutHistogram( QVector< Interval<int> >::const_iterator begin, QVector< Interval<int> >::const_iterator end ) {
		while ( begin != end ) {
			std::cout << '[' << (*begin).min() <<  ", " << (*begin).max() << "] ";
			begin++;
		}
		std::cout << std::endl;
	}
}

namespace Test_IntervalsHistogram {
	void allTests() {
		std::cout << "==> TEST 1" << std::endl;
		test1();
		std::cout << "==> TEST 2" << std::endl;
		test2();
		std::cout << "==> TEST 3" << std::endl;
		test3();
		std::cout << "==> TEST 4" << std::endl;
		test4();
		std::cout << "==> TEST 5" << std::endl;
		test5();
	}

	void test1() {
		Histogram<qreal> hist;
		hist << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1;

		std::cout << "Histogramme constant sans boucle" << std::endl;
		routineCommune(hist,false);

		hist.clear();
		hist << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1;

		std::cout << "Histogramme constant avec boucle" << std::endl;
		routineCommune(hist,true);
	}

	void test2() {
		Histogram<qreal> hist;
		hist << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 0 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1;

		std::cout << "Histogramme avec une valeur différente sans boucle" << std::endl;
		routineCommune(hist,false);

		hist.clear();
		hist << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 0 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1;

		std::cout << "Histogramme avec une valeur différente avec boucle" << std::endl;
		routineCommune(hist,true);
	}

	void test3() {
		Histogram<qreal> hist;
		hist << 0 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1;

		std::cout << "Histogramme avec une valeur différente en début sans boucle" << std::endl;
		routineCommune(hist,false);

		hist.clear();
		hist << 0 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1;

		std::cout << "Histogramme avec une valeur différente en début avec boucle" << std::endl;
		routineCommune(hist,true);
	}

	void test4() {
		Histogram<qreal> hist;
		hist << 1 << 1 << 1 << 1 << 1 << 1 << 2 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1;

		std::cout << "Histogramme avec un max en 6 sans boucle" << std::endl;
		routineCommune(hist,false);

		hist.clear();
		hist << 1 << 1 << 1 << 1 << 1 << 1 << 2 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1;

		std::cout << "Histogramme avec un max en 6 avec boucle" << std::endl;
		routineCommune(hist,true);
	}

	void test5() {
		Histogram<qreal> hist;
		hist << 2 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1;

		std::cout << "Histogramme avec un max en 0 sans boucle" << std::endl;
		routineCommune(hist,false);

		hist.clear();
		hist << 2 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1 << 1;

		std::cout << "Histogramme avec un max en 0 avec boucle" << std::endl;
		routineCommune(hist,true);
	}

	void routineCommune( Histogram<qreal> &hist, bool loop ) {
		std::cout << "Avant : ";
		coutHistogram( hist.begin(), hist.end() );
		hist.meansSmoothing(DEFAULT_MASK_RADIUS,loop);
		std::cout << "Après : ";
		coutHistogram( hist.begin(), hist.end() );
		hist.computeMaximums( DEFAULT_PERCENTAGE_FOR_MAXIMUM_CANDIDATE, DEFAULT_MINIMUM_WIDTH_OF_NEIGHBORHOOD, loop );
		std::cout << "Maximums : ";
		coutHistogram(hist.maximums().begin(),hist.maximums().end());
	}

}
