
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    double CumulativeNormalDistribution::operator()(double z) const {
        /// ???
        QL_REQUIRE(!(z >= average_ && 2.0*average_-z > average_),
                   "CumulativeNormalDistribution: not a real number. ");
        z = (z - average_) / sigma_;

        double result = 0.5 * ( 1 + errorFunction_( z*M_SQRT_2 ) );
        if (result<=QL_EPSILON) {
            // Asymptotic expansion for very negative z following (26.2.12)
            // on page 408 in M. Abramowitz and A. Stegun,
            // Pocketbook of Mathematical Functions, ISBN 3-87144818-4.
            double sum=1, zsqr=z*z, i=1, g=1, x, y, 
                a=QL_MAX_DOUBLE, lasta;
            do {
                lasta=a;
                x = (4*i-3)/zsqr;
                y = x*((4*i-1)/zsqr);
                a = g*(x-y);
                sum -= a;
                g *= y;
                ++i;
                a = QL_FABS(a);
            } while (lasta>a && a>=QL_FABS(sum*QL_EPSILON));
            result = -gaussian_(z)/z*sum;
        }
        return result;
    }

    #if !defined(QL_PATCH_SOLARIS)
    const CumulativeNormalDistribution InverseCumulativeNormal::f_;
    #endif

    // Coefficients for the rational approximation.
    const double InverseCumulativeNormal::a1_ = -3.969683028665376e+01;
    const double InverseCumulativeNormal::a2_ =  2.209460984245205e+02;
    const double InverseCumulativeNormal::a3_ = -2.759285104469687e+02;
    const double InverseCumulativeNormal::a4_ =  1.383577518672690e+02;
    const double InverseCumulativeNormal::a5_ = -3.066479806614716e+01;
    const double InverseCumulativeNormal::a6_ =  2.506628277459239e+00;

    const double InverseCumulativeNormal::b1_ = -5.447609879822406e+01;
    const double InverseCumulativeNormal::b2_ =  1.615858368580409e+02;
    const double InverseCumulativeNormal::b3_ = -1.556989798598866e+02;
    const double InverseCumulativeNormal::b4_ =  6.680131188771972e+01;
    const double InverseCumulativeNormal::b5_ = -1.328068155288572e+01;

    const double InverseCumulativeNormal::c1_ = -7.784894002430293e-03;
    const double InverseCumulativeNormal::c2_ = -3.223964580411365e-01;
    const double InverseCumulativeNormal::c3_ = -2.400758277161838e+00;
    const double InverseCumulativeNormal::c4_ = -2.549732539343734e+00;
    const double InverseCumulativeNormal::c5_ =  4.374664141464968e+00;
    const double InverseCumulativeNormal::c6_ =  2.938163982698783e+00;

    const double InverseCumulativeNormal::d1_ =  7.784695709041462e-03;
    const double InverseCumulativeNormal::d2_ =  3.224671290700398e-01;
    const double InverseCumulativeNormal::d3_ =  2.445134137142996e+00;
    const double InverseCumulativeNormal::d4_ =  3.754408661907416e+00;

    // Limits of the approximation regions
    const double InverseCumulativeNormal::x_low_ = 0.02425;
    const double InverseCumulativeNormal::x_high_= 1.0 - x_low_;

    double InverseCumulativeNormal::operator()(double x) const {
        QL_REQUIRE(x > 0.0 && x < 1.0, 
                   "InverseCumulativeNormal(" +
                   DoubleFormatter::toString(x) + 
                   ") undefined: must be 0 < x < 1");

        double z, r;

        if (x < x_low_) {
            // Rational approximation for the lower region 0<x<u_low
            z = QL_SQRT(-2.0*QL_LOG(x));
            z = (((((c1_*z+c2_)*z+c3_)*z+c4_)*z+c5_)*z+c6_) /
                ((((d1_*z+d2_)*z+d3_)*z+d4_)*z+1.0);
        } else if (x <= x_high_) {
            // Rational approximation for the central region u_low<=x<=u_high
            z = x - 0.5;
            r = z*z;
            z = (((((a1_*r+a2_)*r+a3_)*r+a4_)*r+a5_)*r+a6_)*z /
                (((((b1_*r+b2_)*r+b3_)*r+b4_)*r+b5_)*r+1.0);
        } else {
            // Rational approximation for the upper region u_high<x<1
            z = QL_SQRT(-2.0*QL_LOG(1.0-x));
            z = -(((((c1_*z+c2_)*z+c3_)*z+c4_)*z+c5_)*z+c6_) /
                ((((d1_*z+d2_)*z+d3_)*z+d4_)*z+1.0);
        }


        // The relative error of the approximation has absolute value less
        // than 1.15e-9.  One iteration of Halley's rational method (third
        // order) gives full machine precision.
#define REFINE_TO_FULL_MACHINE_PRECISION_USING_HALLEYS_METHOD
#ifdef  REFINE_TO_FULL_MACHINE_PRECISION_USING_HALLEYS_METHOD
        // error (f_(z) - x) divided by the cumulative's derivative
        r = (f_(z) - x) * M_SQRT2 * M_SQRTPI * exp(0.5 * z*z);
        //	Halley's method
        z -= r/(1+0.5*z*r);
#endif

        return average_ + z*sigma_;
    }


    const double MoroInverseCumulativeNormal::a0_ =  2.50662823884;
    const double MoroInverseCumulativeNormal::a1_ =-18.61500062529;
    const double MoroInverseCumulativeNormal::a2_ = 41.39119773534;
    const double MoroInverseCumulativeNormal::a3_ =-25.44106049637;

    const double MoroInverseCumulativeNormal::b0_ = -8.47351093090;
    const double MoroInverseCumulativeNormal::b1_ = 23.08336743743;
    const double MoroInverseCumulativeNormal::b2_ =-21.06224101826;
    const double MoroInverseCumulativeNormal::b3_ =  3.13082909833;

    const double MoroInverseCumulativeNormal::c0_ = 0.3374754822726147;
    const double MoroInverseCumulativeNormal::c1_ = 0.9761690190917186;
    const double MoroInverseCumulativeNormal::c2_ = 0.1607979714918209;
    const double MoroInverseCumulativeNormal::c3_ = 0.0276438810333863;
    const double MoroInverseCumulativeNormal::c4_ = 0.0038405729373609;
    const double MoroInverseCumulativeNormal::c5_ = 0.0003951896511919;
    const double MoroInverseCumulativeNormal::c6_ = 0.0000321767881768;
    const double MoroInverseCumulativeNormal::c7_ = 0.0000002888167364;
    const double MoroInverseCumulativeNormal::c8_ = 0.0000003960315187;

    double MoroInverseCumulativeNormal::operator()(double x) const {
        QL_REQUIRE(x > 0.0 && x < 1.0, 
                   "MoroInverseCumulativeNormal(" +
                   DoubleFormatter::toString(x) + 
                   ") undefined: must be 0<x<1");

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
