
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_operator_traits_h
#define quantlib_operator_traits_h

#include "qldefines.h"
#include "array.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(PDE)

template<class Operator>
class OperatorTraits {
  public:
	typedef Array arrayType;
};

QL_END_NAMESPACE(PDE)

QL_END_NAMESPACE(QuantLib)


#endif
