/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
/*! \file vasicek.cpp
    \brief Vasicek model class

    \fullpath
    ql/ShortRateModels/OneFactorModels/%vasicek.cpp
*/

// $Id$

#include "ql/ShortRateModels/OneFactorModels/vasicek.hpp"
#include "ql/blackmodel.hpp"

namespace QuantLib {

    namespace ShortRateModels {

        using Optimization::NoConstraint;
        using Optimization::PositiveConstraint;

        Vasicek::Vasicek(Rate r0, double a, double b, double sigma)
        : OneFactorModel(3), r0_(r0),
          a_(parameters_[0]), b_(parameters_[1]), sigma_(parameters_[2]) {
            a_ = ConstantParameter(a, PositiveConstraint());
            b_ = ConstantParameter(b, NoConstraint());
            sigma_ = ConstantParameter(sigma, PositiveConstraint());
        }

        double Vasicek::A(Time t) const {
            double sigma2 = sigma()*sigma();
            double bt = B(t);
            return QL_EXP((b() - 0.5*sigma2/(a()*a()))*(bt - t) -
                          0.25*sigma2*bt*bt/a());
        }

        double Vasicek::B(Time t) const {
                return (1.0 - QL_EXP(-a()*t))/a();
        }

        double Vasicek::discountBond(Time t, Time T, Rate r) const {
            return A(T-t)*QL_EXP(- B(T-t)*r);
        }

        double Vasicek::discountBondOption(Option::Type type,
            double strike, Time maturity, Time bondMaturity) const {

            double v;
            if (QL_FABS(maturity) < QL_EPSILON) {
                v = 0.0;
            } else {
                v = sigma()*B(bondMaturity - maturity)*
                    QL_SQRT(0.5*(1.0 - QL_EXP(-2.0*a()*maturity))/a());
            }
            double f = discountBond(0.0, bondMaturity, r0_);
            double k = discountBond(0.0, maturity, r0_)*strike;

            double w = (type==Option::Call)? 1.0 : -1.0;

            return BlackModel::formula(f, k, v, w);
        }

    }

}
