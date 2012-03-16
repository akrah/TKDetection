#ifndef BILLON_DEF_H
#define BILLON_DEF_H

template<typename T> class BillonTpl;

typedef int __billon_type__;
typedef BillonTpl<__billon_type__>   Billon;

namespace arma {
	template<typename T> class Mat;
	typedef Mat<__billon_type__>   Slice;
}

#endif // BILLON_DEF_H
