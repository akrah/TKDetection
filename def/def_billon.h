#ifndef DEF_BILLON_H
#define DEF_BILLON_H

template<typename T> class BillonTpl;
typedef int __billon_type__;
typedef BillonTpl<__billon_type__> Billon;

namespace arma
{
	template<typename T> class Mat;
}
typedef arma::Mat<__billon_type__> Slice;

#endif // DEF_BILLON_H
