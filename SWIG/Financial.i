
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_financial_i
#define quantlib_financial_i

%module Financial

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%{
#include "yield.h"
#include "spread.h"
#include "discountfactor.h"

using QuantLib::Rate;
using QuantLib::Spread;
using QuantLib::DiscountFactor;
%}

typedef double Rate;
typedef double Spread;
typedef double DiscountFactor;


#endif
