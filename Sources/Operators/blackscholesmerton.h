
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef black_scholes_merton_h
#define black_scholes_merton_h

#include "qldefines.h"
#include "tridiagonaloperator.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Operators)

class BSMOperator : public TridiagonalOperator {
  public:
	BSMOperator() : TridiagonalOperator() {}
	BSMOperator(int size, double dx, double r, double q, double sigma);
};


QL_END_NAMESPACE(Operators)

QL_END_NAMESPACE(QuantLib)


#endif
