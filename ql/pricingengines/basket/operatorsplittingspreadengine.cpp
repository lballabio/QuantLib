/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 klaus Spanderen

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

#include <ql/pricingengines/basket/operatorsplittingspreadengine.hpp>

#include <ql/math/functional.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

#include <iostream>

namespace QuantLib {

    OperatorSplittingSpreadEngine::OperatorSplittingSpreadEngine(
        ext::shared_ptr<BlackProcess> process1,
        ext::shared_ptr<BlackProcess> process2,
        Real correlation,
        Order order)
    : SpreadBlackScholesVanillaEngine(process1, process2, correlation),
      order_(order) {
    }

    Real OperatorSplittingSpreadEngine::calculate(
        Real k, Option::Type optionType,
        Real variance1, Real variance2, DiscountFactor df) const {

        const auto callPutParityPrice = [this, df, k, optionType](Real callPrice) -> Real {
            if (optionType == Option::Call)
                return callPrice;
            else
                return callPrice - df*(f1_-f2_-k);
        };

        const Real vol1 = std::sqrt(variance1);
        const Real vol2 = std::sqrt(variance2);
        const Real sig2 = vol2*f2_/(f2_+k);
        const Real sig_m = std::sqrt(variance1 +sig2*(sig2 - 2*rho_*vol1));

        const Real d1 = (std::log(f1_) - std::log(f2_ + k))/sig_m + 0.5*sig_m;
        const Real d2 = d1 - sig_m;

        const CumulativeNormalDistribution N;

        const Real kirkCallNPV = df*(f1_*N(d1) - (f2_ + k)*N(d2));

        const Real vv = (rho_*vol1 - sig2)*vol2/(sig_m*sig_m);
        const Real oPlt = -sig2*sig2 * k * df * NormalDistribution()(d2) * vv
                *( d2*(1 - rho_*vol1/sig2)
                   - 0.5*sig_m * vv * k / (f2_+k)
                     * ( d1*d2 + (1-rho_*rho_)*squared(vol1/(rho_*vol1-sig2))));

        if (order_ == First)
            return callPutParityPrice(kirkCallNPV + 0.5*oPlt);

        QL_REQUIRE(order_ == Second, "unknown approximation type");

        /*
           In the original paper the second order was calculated using numerical differentiation.
           The following Mathematica scripts calculates the approximation to the n'th order.

           vol2Hat[R2_] := vol2*(R2 - K)/R2
           volMinusHat[R2_] := Sqrt[vol1^2 - 2*rho*vol1*vol2Hat[R2] + vol2Hat[R2]^2]
           zeta1[R1_, R2_] := 1/(volMinusHat[R2]*Sqrt[t])*(Log[R1] + volMinusHat[R2]^2*t/2)
           zeta2[R1_, R2_] := zeta1[R1, R2] - volMinusHat[R2]*Sqrt[t]
           pLT[R1_, R2_] := Exp[-r*t]*R2*(R1*CDF[NormalDistribution[0, 1], zeta1[R1, R2]]
                                           - CDF[NormalDistribution[0, 1], zeta2[R1, R2]])
           opt[R1_, R2_] := (1/2*vol2Hat[R2]^2*R2^2*D[#, {R2, 2}] + (rho*vol1 - vol2Hat[R2])*vol2Hat[R2]*R1*R2*
                               D[#, R1, R2] - (rho*vol1 - vol2Hat[R2])*vol2Hat[R2]*R1*D[#, R1]) &

           pStrange1[R1_, R2_] := pLT[R1, R2] + (t/2)^1/Factorial[1]*opt[R1, R2][pLT[R1, R2]]
           pStrange2[R1_, R2_] := pStrange1[R1, R2] + (t/2)^2/Factorial[2]*opt[R1, R2][opt[R1, R2][pLT[R1, R2]]]
         */

        const Real R2 = f2_+k;
        const Real R1 = f1_/R2;
        const Real F2 = f2_;

        const Real F22 = F2*F2;
        const Real F23 = F22*F2;
        const Real F24 = F22*F22;

        const Real iR2 = 1.0/R2;
        const Real iR22 = iR2*iR2;
        const Real iR23 = iR22*iR2;
        const Real iR24 = iR22*iR22;
        const Real vol12 = vol1*vol1;
        const Real vol22 = vol2*vol2;
        const Real vol23 = vol22*vol2;
        const Real a = vol12 - 2*F2*iR2*rho_*vol1*vol2 + F22*iR22*vol22;
        const Real a2 = a*a;
        const Real b = a/2+std::log(R1);
        const Real b2 = b*b;
        const Real c = std::sqrt(a);
        const Real d = b/c;
        const Real e = rho_*vol1 - F2*iR2*vol2;
        const Real e2 = e*e;
        const Real f = d-c;
        const Real g = -2*iR2*rho_*vol1*vol2 + 2*F2*iR22*rho_*vol1*vol2 + 2*F2*iR22*vol22 - 2*F22*iR23*vol22;
        const Real h = rho_*rho_;
        const Real j = 1-h;
        const Real iat = 1/c;
        const Real l = b*iat - c;
        const Real m = f*(1 - (R2*rho_*vol1)/(F2*vol2)) - (e*iR2*k*(d*l + (j*vol12)/(e*e))*vol2)/(2.*c);
        const Real n = (iat*(1 - (R2*rho_*vol1)/(F2*vol2)))/R1 - (e*iR2*k*((f*iat)/R1 + b/(a*R1))*vol2)/(2.*c);
        const Real o = df*std::exp(-0.5*f*f);
        const Real p = d*l + (j*vol12)/(e*e);
        const Real q = (-2*j*vol12*(-(iR2*vol2) + F2*iR22*vol2))/(e*e*e);
        const Real s = q - (b2*g)/(2.*a2) - (b*f*g)/(2.*a*c) + (f*g)/(2.*c);
        const Real u = f*(-((rho_*vol1)/(F2*vol2)) + (R2*rho_*vol1)/(F22*vol2));
        const Real v = -0.5*(b*g*(1 - (R2*rho_*vol1)/(F2*vol2)))/(a*c);
        const Real w = (3*g*g)/(4.*a2*c) - (4*iR22*rho_*vol1*vol2 - 4*F2*iR23*rho_*vol1*vol2 +
                2*iR22*vol22 - 8*F2*iR23*vol22 + 6*F22*iR24*vol22)/(2.*a*c);
        const Real x = u + v + (e*g*iR2*k*p*vol2)/(4.*a*c) + (e*iR22*k*p*vol2)/(2.*c) -
                (e*iR2*k*s*vol2)/(2.*c) - (iR2*k*p*vol2*(-(iR2*vol2) + F2*iR22*vol2))/(2.*c);
        const Real y = (4*iR22 - 4*F2*iR23)*rho_*vol1*vol2 + (2*iR22 - 8*F2*iR23 + 6*F22*iR24)*vol22;
        const Real z = 4*iR22*rho_*vol1*vol2 - 4*F2*iR23*rho_*vol1*vol2 + 2*iR22*vol22 -
                8*F2*iR23*vol22 + 6*F22*iR24*vol22;

        const Real ooPlt = (k*o*vol23*(-2*c*b2*e2*e*(-1 + f*f)*F23*F24*g*g*iR22*m*vol23 +
                2*b2*e2*e2*F23*F24*g*g*iR2*iR22*k*vol22*vol22 + 2*a*b*e2*e*F23*F22*g*iR22*vol2*(-8*
                e2*F2*iR2*k*vol22 + 7*f*F22*g*m*vol22) - a*c*e2*e*F23*F22*g*iR22*vol2*(4*e*F2*
                vol2*(-2*b*(-1 + f*f)*m + e*f*iR2*k*vol2) + F22*g*(16*m + e*(2*f + 3*b*iat)*iR2*k*vol2)*vol22) -
                4*a2*a*c*e2*(e2*F22*vol2*(4*F22*iat*iR22*R2*rho_*vol1 + 8*F23*iR22*n*R1*vol2 -
                4*F24*3*iR23*n*R1*vol2 - F23*iR22*(4*iat*rho_*vol1 + F22*iR2*k*p*vol23*w))
                + 4*F23*F22*vol22*vol22*(iR22*(-2*F2*iR2 + 3*F22*iR22)*m + F22*(2*iR2 - 3*F2*iR22)*iR23*m
                + F22*iR22*(-iR2 + F2*iR22)*x) + 2*e*F22*(2*F24*F2*iR24*n*R1*vol23 +
                2*f*F2*F22*iR22*rho_*vol1*vol22 - 2*f*F22*iR22*R2*rho_*vol1*vol22 -
                b*F24*iR22*R2*rho_*vol1*vol22*w - 2*F24*vol2*(iR23*n*R1*vol22 + 4*iR23*m*vol22 - 2*iR22*vol22*x) +
                F23*(2*iR22*m*vol23 + 6*F22*iR24*m*vol23 + b*F22*iR22*vol23*w - 4*F22*iR23*vol23*x))) +
                2*a2*c*e2*F23*F22*vol2*(8*F22*g*iR22*(-iR2 + F2*iR22)*m*vol23 +
                e2*iR22*vol2*(8*F2*g*n*R1 + b*F22*iat*iR2*k*vol22*(y - z)) +
                4*e*vol22*(4*F2*g*iR22*m + F22*(-4*g*iR23*m + 2*g*iR22*x + iR22*m*z))) +
                2*a2*a*F22*(-4*e2*e2*e*f*F24*iat*iR24*k*vol23 + 8*e*F2*F24*iR23*(-iR22 + F2*iR23)*j*k*vol12*vol23*vol22 +
                12*F2*F24*iR23*squared(iR2 - F2*iR22)*j*k*vol12*vol23*vol23 +
                e2*e2*F2*vol22*(2*F24*iR22*k*vol22*(2*(iR23*p - iR22*s) + b2*iat*iR2*w) + f*(4*F22*iR22*(4*m +
                F22*iat*iR2*iR22*k*vol22) - 4*F23*(6*iR23*m + iat*iR24*k*vol22 - 2*iR22*x)
                + F24*iR23*k*vol22*(2*b*w + iat*y))) - 2*e2*e*F22*iR22*(4*f*F22*(iR2 - F2*iR22)*m*vol23 +
                F22*vol22*(F2*vol2*(2*k*(F2*iR24*p + F2*iR24*p + iR22*s - iR23*(2*p + F2*s))*vol22 + y - z)
                + R2*rho_*vol1*(-y + z)))) - 2*a2*e2*F23*(2*e2*e*F23*iR22*k*(2*b*iR22 + g*(-1 + f*iat)*iR2)*vol23 +
                4*b*f*F22*F22*g*iR22*(-iR2 + F2*iR22)*m*vol22*vol22 +
                2*e2*F22*iR22*vol2*(2*b*F2*iR2*(iR2 - F2*iR22)*k*vol23 + g*(2*R2*rho_*vol1 + 2*F2*(-1 + 3*f*m +
                b*f*n*R1)*vol2 + F22*k*(-(iR22*p) + iR2*s)*vol23)) + e*vol22*(F2*F22*g*iR22*(g*R2*rho_*vol1 + F2*g*(-1 + f*m)*vol2 +
                2*F2*iR2*(-iR2 + F2*iR22)*k*p*vol23) + 2*b*(2*F2*F22*g*iR22*rho_*vol1 - 2*F22*g*iR22*R2*rho_*vol1 +
                4*f*F23*g*iR22*m*vol2 + f*F24*vol2*(-4*g*iR23*m + 2*g*iR22*x +
                iR22*m*z))))))/(16.*a2*a2*c*e2*F23*M_SQRT2*M_SQRTPI*vol2);


        return callPutParityPrice(kirkCallNPV + 0.5*oPlt + 0.125*ooPlt);
    }
}


