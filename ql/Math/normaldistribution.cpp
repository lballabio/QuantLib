

/*
 Copyright (C) 2002 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file normaldistribution.cpp
    \brief normal, cumulative and inverse cumulative distributions

    \fullpath
    ql/Math/%normaldistribution.cpp
*/

// $Id$

#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    namespace Math {

        const double NormalDistribution::pi_ = 3.14159265358979323846;

        const double CumulativeNormalDistribution::a1_ =  0.319381530;
        const double CumulativeNormalDistribution::a2_ = -0.356563782;
        const double CumulativeNormalDistribution::a3_ =  1.781477937;
        const double CumulativeNormalDistribution::a4_ = -1.821255978;
        const double CumulativeNormalDistribution::a5_ =  1.330274429;

        const double CumulativeNormalDistribution::gamma_     = 0.2316419;
        const double CumulativeNormalDistribution::precision_ = 1e-6;

        double CumulativeNormalDistribution::operator()(double x) const {
            QL_REQUIRE(!(x >= average_ && 2.0*average_-x > average_),
                "CumulativeNormalDistribution: not a real number. ");
            if (x >= average_) {
                double xn = (x - average_) / sigma_;
                double k = 1.0/(1.0+gamma_*xn);
                double temp = gaussian_(xn) * k *
                                (a1_ + k*(a2_ + k*(a3_ + k*(a4_ + k*a5_))));
                if (temp < precision_) 
                    return 1.0;
                temp = 1.0 - temp;
                if (temp < precision_) 
                    return 0.0;
                return temp;
            } else {
                return 1.0-(*this)(2.0*average_-x);
            }
        }




        // Anyone able to identify the following algorithm?
        // It might be Hill and Davis (1973), or
	    // Odeh and Evans (1974)
        const double InvCumulativeNormalDistribution2::p0_ = 2.515517;
        const double InvCumulativeNormalDistribution2::p1_ = 0.802853;
        const double InvCumulativeNormalDistribution2::p2_ = 0.010328;
        const double InvCumulativeNormalDistribution2::q1_ = 1.432788;
        const double InvCumulativeNormalDistribution2::q2_ = 0.189269;
        const double InvCumulativeNormalDistribution2::q3_ = 0.001308;

        double InvCumulativeNormalDistribution2::operator()(double x) const {
            QL_REQUIRE(x>0.0 && x<1.0, "InvCumulativeNormalDistribution2(" +
                DoubleFormatter::toString(x) + ") undefined: must be 0<x<1");

            if (x <= 0.5) {
                double kSquare = QL_LOG( 1 / (x*x) ) ;
                double k = QL_SQRT(kSquare);
                double rn = ((p0_ + p1_*k + p2_*kSquare) /
                            (  1  + q1_*k + q2_*kSquare + q3_*kSquare*k) - k );
                return average_ + rn*sigma_;
            } else {
                return 2.0*average_-(*this)(1.0-x);
            }
        }



        const double InvCumulativeNormalDistribution::a0_ =  2.50662823884;
        const double InvCumulativeNormalDistribution::a1_ =-18.61500062529;
        const double InvCumulativeNormalDistribution::a2_ = 41.39119773534;
        const double InvCumulativeNormalDistribution::a3_ =-25.44106049637;

        const double InvCumulativeNormalDistribution::b0_ = -8.47351093090;
        const double InvCumulativeNormalDistribution::b1_ = 23.08336743743;
        const double InvCumulativeNormalDistribution::b2_ =-21.06224101826;
        const double InvCumulativeNormalDistribution::b3_ =  3.13082909833;

        const double InvCumulativeNormalDistribution::c0_ = 0.3374754822726147;
        const double InvCumulativeNormalDistribution::c1_ = 0.9761690190917186;
        const double InvCumulativeNormalDistribution::c2_ = 0.1607979714918209;
        const double InvCumulativeNormalDistribution::c3_ = 0.0276438810333863;
        const double InvCumulativeNormalDistribution::c4_ = 0.0038405729373609;
        const double InvCumulativeNormalDistribution::c5_ = 0.0003951896511919;
        const double InvCumulativeNormalDistribution::c6_ = 0.0000321767881768;
        const double InvCumulativeNormalDistribution::c7_ = 0.0000002888167364;
        const double InvCumulativeNormalDistribution::c8_ = 0.0000003960315187;

        double InvCumulativeNormalDistribution::operator()(double x) const {
            QL_REQUIRE(x>0.0 && x<1.0, "InvCumulativeNormalDistribution(" +
                DoubleFormatter::toString(x) + ") undefined: must be 0<x<1");

            double result;
            double temp=x-0.5;

            if (QL_FABS(temp) < 0.42) {
                // Beasley and Springer, 1977
                result=temp*temp;
                result=temp*
                    (((a3_*result+a2_)*result+a1_)*result+a0_) /
                    ((((b3_*result+b2_)*result+b1_)*result+b0_)*result+1.0);
            } else {
                // improved approximation for the tail (Moro 1995)
                if (x<0.5)
                    result = x;
                else
                    result=1.0-x;
                result = QL_LOG(-QL_LOG(result));
                result = c0_+result*(c1_+result*(c2_+result*(c3_+result*
                            (c4_+result*(c5_+result*(c6_+result*
                            (c7_+result*c8_)))))));
                if (x<0.5)
                    result=-result;
            }

            return average_ + result*sigma_;
        }


}

}
