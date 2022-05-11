/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio

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

#include <ql/experimental/math/convolvedstudentt.hpp>
#include <ql/errors.hpp>
#include <ql/math/factorial.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/functional.hpp>

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/math/distributions/students_t.hpp>
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif

namespace QuantLib {

    CumulativeBehrensFisher::CumulativeBehrensFisher(const std::vector<Integer>& degreesFreedom,
                                                     const std::vector<Real>& factors)
    : degreesFreedom_(degreesFreedom), factors_(factors), polyConvolved_(std::vector<Real>(1, 1.))

    {
        QL_REQUIRE(degreesFreedom.size() == factors.size(),
            "Incompatible sizes in convolution.");
        for (int i : degreesFreedom) {
            QL_REQUIRE(i % 2 != 0, "Even degree of freedom not allowed");
            QL_REQUIRE(i >= 0, "Negative degree of freedom not allowed");
        }
        for(Size i=0; i<degreesFreedom_.size(); i++)
            polynCharFnc_.push_back(polynCharactT((degreesFreedom[i]-1)/2));
        // adjust the polynomial coefficients by the factors in the linear
        //   combination:
        for(Size i=0; i<degreesFreedom_.size(); i++) {
            Real multiplier = 1.;
            for(Size k=1; k<polynCharFnc_[i].size(); k++) {
                multiplier *= std::abs(factors_[i]);
                polynCharFnc_[i][k] *= multiplier;
            }
        }
        //convolution, here it is a product of polynomials and exponentials
        for (auto& i : polynCharFnc_)
            polyConvolved_ = convolveVectorPolynomials(polyConvolved_, i);
        // trim possible zeros that might have arised:
        auto it = polyConvolved_.rbegin();
        while (it != polyConvolved_.rend()) {
            if (*it == 0.) {
                polyConvolved_.pop_back();
                it = polyConvolved_.rbegin();
              }else{
                  break;
              }
          }
          // cache 'a' value (the exponent)
          for(Size i=0; i<degreesFreedom_.size(); i++)
              a_ += std::sqrt(static_cast<Real>(degreesFreedom_[i]))
                * std::abs(factors_[i]);
          a2_ = a_ * a_;
    }

    std::vector<Real> CumulativeBehrensFisher::polynCharactT(Natural n) const {
        Natural nu = 2 * n +1;
        std::vector<Real> low(1,1.), high(1,1.);
        high.push_back(std::sqrt(static_cast<Real>(nu)));
        if(n==0) return low;
        if(n==1) return high;

        for(Size k=1; k<n; k++) {
            std::vector<Real> recursionFactor(1,0.); // 0 coef
            recursionFactor.push_back(0.); // 1 coef
            recursionFactor.push_back(nu/((2.*k+1.)*(2.*k-1.))); // 2 coef
            std::vector<Real> lowUp =
                convolveVectorPolynomials(recursionFactor, low);
            //add them up:
            for(Size i=0; i<high.size(); i++)
                lowUp[i] += high[i];
            low = high;
            high = lowUp;
        }
        return high;
    }

    std::vector<Real> CumulativeBehrensFisher::convolveVectorPolynomials(
        const std::vector<Real>& v1,
        const std::vector<Real>& v2) const {
    #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(!v1.empty() && !v2.empty(),
            "Incorrect vectors in polynomial.");
    #endif

        const std::vector<Real>& shorter = v1.size() < v2.size() ? v1 : v2;
        const std::vector<Real>& longer = (v1 == shorter) ? v2 : v1;

        Size newDegree = v1.size()+v2.size()-2;
        std::vector<Real> resultB(newDegree+1, 0.);
        for(Size polyOrdr=0; polyOrdr<resultB.size(); polyOrdr++) {
            for(Size i=std::max<Integer>(0, polyOrdr-longer.size()+1);
                i<=std::min(polyOrdr, shorter.size()-1); i++)
                resultB[polyOrdr] += shorter[i]*longer[polyOrdr-i];
        }
        return resultB;
    }

    Probability CumulativeBehrensFisher::operator()(const Real x) const {
        // 1st & 0th terms with the table integration
        Real integral = polyConvolved_[0] * std::atan(x/a_);
        Real squared = a2_ + x*x;
        Real rootsqr = std::sqrt(squared);
        Real atan2xa = std::atan2(-x,a_);
        if(polyConvolved_.size()>1)
            integral += polyConvolved_[1] * x/squared;

        for(Size exponent = 2; exponent <polyConvolved_.size(); exponent++) {
            integral -= polyConvolved_[exponent] *
                Factorial::get(exponent-1) * std::sin((exponent)*atan2xa)
                    /std::pow(rootsqr, static_cast<Real>(exponent));
         }
        return .5 + integral / M_PI;
    }

    Probability
    CumulativeBehrensFisher::density(const Real x) const {
        Real squared = a2_ + x*x;
        Real integral = polyConvolved_[0] * a_ / squared;
        Real rootsqr = std::sqrt(squared);
        Real atan2xa = std::atan2(-x,a_);
        for(Size exponent=1; exponent <polyConvolved_.size(); exponent++) {
            integral += polyConvolved_[exponent] *
                Factorial::get(exponent) * std::cos((exponent+1)*atan2xa)
                    /std::pow(rootsqr, static_cast<Real>(exponent+1) );
        }
        return integral / M_PI;
    }



    InverseCumulativeBehrensFisher::InverseCumulativeBehrensFisher(
        const std::vector<Integer>& degreesFreedom,
        const std::vector<Real>& factors,
        Real accuracy)
    : normSqr_(std::inner_product(factors.begin(), factors.end(),
        factors.begin(), 0.)),
      accuracy_(accuracy), distrib_(degreesFreedom, factors) { }

    Real InverseCumulativeBehrensFisher::operator()(const Probability q) const {
        Probability effectiveq;
        Real sign;
        // since the distrib is symmetric solve only on the right side:
        if(q==0.5) {
            return 0.;
        }else if(q < 0.5) {
            sign = -1.;
            effectiveq = 1.-q;
        }else{
            sign = 1.;
            effectiveq = q;
        }
        Real xMin =
            InverseCumulativeNormal::standard_value(effectiveq) * normSqr_;
        // inversion will fail at the Brent's bounds-check if this is not enough
        // (q is very close to 1.), in a bad combination fails around 1.-1.e-7
        Real xMax = 1.e6;
        return sign *
            Brent().solve([&](Real x){ return distrib_(x) - effectiveq; },
                          accuracy_, (xMin+xMax)/2., xMin, xMax);
    }

}
