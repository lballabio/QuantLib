
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_financial_i
#define quantlib_financial_i

%module Financial

%{
#include "yield.h"
#include "spread.h"
#include "discountfactor.h"

QL_USING(QuantLib,Yield)
QL_USING(QuantLib,Spread)
QL_USING(QuantLib,DiscountFactor)
%}

typedef double Yield;
typedef double Spread;
typedef double DiscountFactor;


#endif
