
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file normaldistribution.cpp
    \brief normal and cumulative normal distributions
    
    $Source$
    $Name$
    $Log$
    Revision 1.9  2001/01/08 16:22:55  nando
    added InverseCumulativeNormalDistribution

    Revision 1.8  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks
    
*/

#include "normaldistribution.h"
#include "qlerrors.h"
#include "dataformatters.h"

namespace QuantLib {

    namespace Math {
    
        const double NormalDistribution::pi_ = 3.14159265358979323846;
        
        // For the following formula see M. Abramowitz and I. Stegun,
        // Handbook of Mathematical Functions,
        // Dover Publications, New York (1972)
        
        const double CumulativeNormalDistribution::a1_ = 0.319381530;
        const double CumulativeNormalDistribution::a2_ = -0.356563782;
        const double CumulativeNormalDistribution::a3_ = 1.781477937;
        const double CumulativeNormalDistribution::a4_ = -1.821255978;
        const double CumulativeNormalDistribution::a5_ = 1.330274429;
        const double CumulativeNormalDistribution::gamma_ = 0.2316419;
        const double CumulativeNormalDistribution::precision_ = 1e-6;
        
        double CumulativeNormalDistribution::operator()(double x) const {
            if (x >= 0.0) {
                double k = 1.0/(1.0+gamma_*x);
                double temp = gaussian_(x)*k*(a1_+k*(a2_+k*(a3_+k*(a4_+k*a5_))));
                if (temp<precision_) return 1.0;
                temp = 1.0-temp;
                if (temp<precision_) return 0.0;
                return temp;
            } else {
                return 1.0-(*this)(-x);
            }
        }


        const double InverseCumulativeNormalDistribution::p0_ = 2.515517;
        const double InverseCumulativeNormalDistribution::p1_ = 0.802853;
        const double InverseCumulativeNormalDistribution::p2_ = 0.010328;
        const double InverseCumulativeNormalDistribution::q1_ = 1.432788;
        const double InverseCumulativeNormalDistribution::q2_ = 0.189269;
        const double InverseCumulativeNormalDistribution::q3_ = 0.001308;
        
        double InverseCumulativeNormalDistribution::operator()(double x) const {
            QL_REQUIRE(x>0.0 && x<1.0, "InverseCumulativeNormalDistribution(" +
                DoubleFormatter::toString(x) + ") undefined: must be 0<x<1");
            
            if (x <= 0.5) {
                double kSquare = QL_LOG( 1 / (x*x) ) ;
                double k = QL_SQRT(kSquare);
                return ( ( p0_ + p1_ * k + p2_ * kSquare ) /
                         (   1 + q1_ * k + q2_ * kSquare + q3_ * kSquare * k ) - k );
            } else {
                return -(*this)(1.0-x);
            }
        }
    
    }

}
