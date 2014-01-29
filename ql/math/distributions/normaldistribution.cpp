/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2008 StatPro Italia srl
 Copyright (C) 2010 Kakhkhor Abdijalilov

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/comparison.hpp>
#include <boost/math/distributions/normal.hpp>

namespace QuantLib {

    Real CumulativeNormalDistribution::operator()(Real z) const {
        //QL_REQUIRE(!(z >= average_ && 2.0*average_-z > average_),
        //           "not a real number. ");
        z = (z - average_) / sigma_;

        Real result = 0.5 * ( 1.0 + errorFunction_( z*M_SQRT_2 ) );
        if (result<=1e-8) { //todo: investigate the threshold level
            // Asymptotic expansion for very negative z following (26.2.12)
            // on page 408 in M. Abramowitz and A. Stegun,
            // Pocketbook of Mathematical Functions, ISBN 3-87144818-4.
            Real sum=1.0, zsqr=z*z, i=1.0, g=1.0, x, y,
                 a=QL_MAX_REAL, lasta;
            do {
                lasta=a;
                x = (4.0*i-3.0)/zsqr;
                y = x*((4.0*i-1)/zsqr);
                a = g*(x-y);
                sum -= a;
                g *= y;
                ++i;
                a = std::fabs(a);
            } while (lasta>a && a>=std::fabs(sum*QL_EPSILON));
            result = -gaussian_(z)/z*sum;
        }
        return result;
    }

    #if !defined(QL_PATCH_SOLARIS)
    const CumulativeNormalDistribution InverseCumulativeNormal::f_;
    #endif

    // Coefficients for the rational approximation.
    const Real InverseCumulativeNormal::a1_ = -3.969683028665376e+01;
    const Real InverseCumulativeNormal::a2_ =  2.209460984245205e+02;
    const Real InverseCumulativeNormal::a3_ = -2.759285104469687e+02;
    const Real InverseCumulativeNormal::a4_ =  1.383577518672690e+02;
    const Real InverseCumulativeNormal::a5_ = -3.066479806614716e+01;
    const Real InverseCumulativeNormal::a6_ =  2.506628277459239e+00;

    const Real InverseCumulativeNormal::b1_ = -5.447609879822406e+01;
    const Real InverseCumulativeNormal::b2_ =  1.615858368580409e+02;
    const Real InverseCumulativeNormal::b3_ = -1.556989798598866e+02;
    const Real InverseCumulativeNormal::b4_ =  6.680131188771972e+01;
    const Real InverseCumulativeNormal::b5_ = -1.328068155288572e+01;

    const Real InverseCumulativeNormal::c1_ = -7.784894002430293e-03;
    const Real InverseCumulativeNormal::c2_ = -3.223964580411365e-01;
    const Real InverseCumulativeNormal::c3_ = -2.400758277161838e+00;
    const Real InverseCumulativeNormal::c4_ = -2.549732539343734e+00;
    const Real InverseCumulativeNormal::c5_ =  4.374664141464968e+00;
    const Real InverseCumulativeNormal::c6_ =  2.938163982698783e+00;

    const Real InverseCumulativeNormal::d1_ =  7.784695709041462e-03;
    const Real InverseCumulativeNormal::d2_ =  3.224671290700398e-01;
    const Real InverseCumulativeNormal::d3_ =  2.445134137142996e+00;
    const Real InverseCumulativeNormal::d4_ =  3.754408661907416e+00;

    // Limits of the approximation regions
    const Real InverseCumulativeNormal::x_low_ = 0.02425;
    const Real InverseCumulativeNormal::x_high_= 1.0 - x_low_;

    Real InverseCumulativeNormal::tail_value(Real x) {
        if (x <= 0.0 || x >= 1.0) {
            // try to recover if due to numerical error
            if (close_enough(x, 1.0)) {
                return QL_MAX_REAL; // largest value available
            } else if (std::fabs(x) < QL_EPSILON) {
                return QL_MIN_REAL; // largest negative value available
            } else {
                QL_FAIL("InverseCumulativeNormal(" << x
                        << ") undefined: must be 0 < x < 1");
            }
        }

        Real z;
        if (x < x_low_) {
            // Rational approximation for the lower region 0<x<u_low
            z = std::sqrt(-2.0*std::log(x));
            z = (((((c1_*z+c2_)*z+c3_)*z+c4_)*z+c5_)*z+c6_) /
                ((((d1_*z+d2_)*z+d3_)*z+d4_)*z+1.0);
        } else {
            // Rational approximation for the upper region u_high<x<1
            z = std::sqrt(-2.0*std::log(1.0-x));
            z = -(((((c1_*z+c2_)*z+c3_)*z+c4_)*z+c5_)*z+c6_) /
                ((((d1_*z+d2_)*z+d3_)*z+d4_)*z+1.0);
        }

        return z;
    }

    const Real MoroInverseCumulativeNormal::a0_ =  2.50662823884;
    const Real MoroInverseCumulativeNormal::a1_ =-18.61500062529;
    const Real MoroInverseCumulativeNormal::a2_ = 41.39119773534;
    const Real MoroInverseCumulativeNormal::a3_ =-25.44106049637;

    const Real MoroInverseCumulativeNormal::b0_ = -8.47351093090;
    const Real MoroInverseCumulativeNormal::b1_ = 23.08336743743;
    const Real MoroInverseCumulativeNormal::b2_ =-21.06224101826;
    const Real MoroInverseCumulativeNormal::b3_ =  3.13082909833;

    const Real MoroInverseCumulativeNormal::c0_ = 0.3374754822726147;
    const Real MoroInverseCumulativeNormal::c1_ = 0.9761690190917186;
    const Real MoroInverseCumulativeNormal::c2_ = 0.1607979714918209;
    const Real MoroInverseCumulativeNormal::c3_ = 0.0276438810333863;
    const Real MoroInverseCumulativeNormal::c4_ = 0.0038405729373609;
    const Real MoroInverseCumulativeNormal::c5_ = 0.0003951896511919;
    const Real MoroInverseCumulativeNormal::c6_ = 0.0000321767881768;
    const Real MoroInverseCumulativeNormal::c7_ = 0.0000002888167364;
    const Real MoroInverseCumulativeNormal::c8_ = 0.0000003960315187;

    Real MoroInverseCumulativeNormal::operator()(Real x) const {
        QL_REQUIRE(x > 0.0 && x < 1.0,
                   "MoroInverseCumulativeNormal(" << x
                   << ") undefined: must be 0<x<1");

        Real result;
        Real temp=x-0.5;

        if (std::fabs(temp) < 0.42) {
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
            result = std::log(-std::log(result));
            result = c0_+result*(c1_+result*(c2_+result*(c3_+result*
                                   (c4_+result*(c5_+result*(c6_+result*
                                                       (c7_+result*c8_)))))));
            if (x<0.5)
                result=-result;
        }

        return average_ + result*sigma_;
    }

    MaddockInverseCumulativeNormal::MaddockInverseCumulativeNormal(
        Real average, Real sigma)
    : average_(average), sigma_(sigma) {}

	Real MaddockInverseCumulativeNormal::operator()(Real x) const {
		return boost::math::quantile(
            boost::math::normal_distribution<Real>(average_, sigma_), x);
	}

}
