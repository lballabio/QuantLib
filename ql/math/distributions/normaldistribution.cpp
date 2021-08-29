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

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include <boost/math/distributions/normal.hpp>

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif

namespace QuantLib {

    namespace { // file scope

        namespace InverseCumulativeNormalPrivate {

            // Coefficients for the rational approximation.
            constexpr Real a1_ = -3.969683028665376e+01;
            constexpr Real a2_ =  2.209460984245205e+02;
            constexpr Real a3_ = -2.759285104469687e+02;
            constexpr Real a4_ =  1.383577518672690e+02;
            constexpr Real a5_ = -3.066479806614716e+01;
            constexpr Real a6_ =  2.506628277459239e+00;

            constexpr Real b1_ = -5.447609879822406e+01;
            constexpr Real b2_ =  1.615858368580409e+02;
            constexpr Real b3_ = -1.556989798598866e+02;
            constexpr Real b4_ =  6.680131188771972e+01;
            constexpr Real b5_ = -1.328068155288572e+01;

            constexpr Real c1_ = -7.784894002430293e-03;
            constexpr Real c2_ = -3.223964580411365e-01;
            constexpr Real c3_ = -2.400758277161838e+00;
            constexpr Real c4_ = -2.549732539343734e+00;
            constexpr Real c5_ =  4.374664141464968e+00;
            constexpr Real c6_ =  2.938163982698783e+00;

            constexpr Real d1_ =  7.784695709041462e-03;
            constexpr Real d2_ =  3.224671290700398e-01;
            constexpr Real d3_ =  2.445134137142996e+00;
            constexpr Real d4_ =  3.754408661907416e+00;

            // Limits of the approximation regions
            constexpr Real x_low_ = 0.02425;
            constexpr Real x_high_= 1.0 - x_low_;
        }

        namespace MoroInverseCumulativeNormalPrivate {

            constexpr Real a0_ =  2.50662823884;
            constexpr Real a1_ =-18.61500062529;
            constexpr Real a2_ = 41.39119773534;
            constexpr Real a3_ =-25.44106049637;

            constexpr Real b0_ = -8.47351093090;
            constexpr Real b1_ = 23.08336743743;
            constexpr Real b2_ =-21.06224101826;
            constexpr Real b3_ =  3.13082909833;

            constexpr Real c0_ = 0.3374754822726147;
            constexpr Real c1_ = 0.9761690190917186;
            constexpr Real c2_ = 0.1607979714918209;
            constexpr Real c3_ = 0.0276438810333863;
            constexpr Real c4_ = 0.0038405729373609;
            constexpr Real c5_ = 0.0003951896511919;
            constexpr Real c6_ = 0.0000321767881768;
            constexpr Real c7_ = 0.0000002888167364;
            constexpr Real c8_ = 0.0000003960315187;
        }

    } // namespace { // file scope

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

    Real InverseCumulativeNormal::standard_value(Real x) {
        using namespace InverseCumulativeNormalPrivate;

        Real z;
        if (x < x_low_ || x_high_ < x) {
            z = tail_value(x);
        } else {
            z = x - 0.5;
            Real r = z*z;
            z = (((((a1_*r+a2_)*r+a3_)*r+a4_)*r+a5_)*r+a6_)*z /
                (((((b1_*r+b2_)*r+b3_)*r+b4_)*r+b5_)*r+1.0);
        }

        // The relative error of the approximation has absolute value less
        // than 1.15e-9.  One iteration of Halley's rational method (third
        // order) gives full machine precision.
        // #define REFINE_TO_FULL_MACHINE_PRECISION_USING_HALLEYS_METHOD
        #ifdef REFINE_TO_FULL_MACHINE_PRECISION_USING_HALLEYS_METHOD
        // error (f_(z) - x) divided by the cumulative's derivative
        const Real r = (f_(z) - x) * M_SQRT2 * M_SQRTPI * exp(0.5 * z*z);
        //  Halley's method
        z -= r/(1+0.5*z*r);
        #endif

        return z;
    }

    Real InverseCumulativeNormal::tail_value(Real x) {
        using namespace InverseCumulativeNormalPrivate;

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

    Real MoroInverseCumulativeNormal::operator()(Real x) const {
        using namespace MoroInverseCumulativeNormalPrivate;

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

    MaddockCumulativeNormal::MaddockCumulativeNormal(
        Real average, Real sigma)
    : average_(average), sigma_(sigma) {}

    Real MaddockCumulativeNormal::operator()(Real x) const {
        return boost::math::cdf(
            boost::math::normal_distribution<Real>(average_, sigma_), x);
    }
}
