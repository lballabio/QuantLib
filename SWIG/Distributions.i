
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/* $Source$
   $Log$
   Revision 1.7  2001/04/06 18:46:21  nando
   changed Authors, Contributors, Licence and copyright header

   Revision 1.6  2001/03/27 17:39:39  lballabio
   Making sure dist target is complete (and added distributions to Ruby module)

   Revision 1.5  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_distributions_i
#define quantlib_distributions_i

%{
using QuantLib::Math::NormalDistribution;
using QuantLib::Math::CumulativeNormalDistribution;
using QuantLib::Math::InvCumulativeNormalDistribution;
%}

class NormalDistribution {
  public:
    NormalDistribution(double average = 0.0, double sigma = 1.0);
    ~NormalDistribution();
    double derivative(double x);
};

%addmethods NormalDistribution {
    #if defined(SWIGRUBY)
    void crash() {}
    #endif
    #if defined(SWIGPYTHON) || defined(SWIGRUBY)
    double __call__(double x) {
        return (*self)(x);
    }
    #endif
}

class CumulativeNormalDistribution {
  public:
     CumulativeNormalDistribution(double average = 0.0, double sigma = 1.0);
     ~CumulativeNormalDistribution();
     double derivative(double x);
};

%addmethods CumulativeNormalDistribution {
    #if defined(SWIGRUBY)
    void crash() {}
    #endif
    #if defined(SWIGPYTHON) || defined(SWIGRUBY)
    double __call__(double x) {
        return (*self)(x);
    }
    #endif
}

class InvCumulativeNormalDistribution {
  public:
    InvCumulativeNormalDistribution(double average = 0.0,  double sigma = 1.0);
    ~InvCumulativeNormalDistribution();
};

%addmethods InvCumulativeNormalDistribution {
    #if defined(SWIGRUBY)
    void crash() {}
    #endif
    #if defined(SWIGPYTHON) || defined(SWIGRUBY)
    double __call__(double x) {
        return (*self)(x);
    }
    #endif
}



#endif

