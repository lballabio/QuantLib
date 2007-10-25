/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include <ql/legacy/libormarketmodels/lfmcovarparam.hpp>
#include <ql/math/integrals/kronrodintegral.hpp>

namespace QuantLib {

    class LfmCovarianceParameterization::Var_Helper {
      public:
        Var_Helper(const LfmCovarianceParameterization* param, Size i, Size j);

        Real operator()(Real t) const;
      private:
        const Size i_, j_;
        const LfmCovarianceParameterization* param_;
    };

    LfmCovarianceParameterization::Var_Helper::Var_Helper(
                                   const LfmCovarianceParameterization* param,
                                   Size i, Size j)
    : i_(i), j_(j), param_(param) {}

    Real LfmCovarianceParameterization::Var_Helper::operator()(Real t) const {
        const Matrix m = param_->diffusion(t);

        return std::inner_product(m.row_begin(i_), m.row_end(i_),
                                  m.row_begin(j_), 0.0);
    }

    Disposable<Matrix> LfmCovarianceParameterization::covariance(
                                               Time t, const Array& x) const {
        Matrix sigma = this->diffusion(t, x);
        Matrix result = sigma*transpose(sigma);
        return result;
    }

    Disposable<Matrix> LfmCovarianceParameterization::integratedCovariance(
                                               Time t, const Array& x) const {
        // this implementation is not intended for production.
        // because it is too slow and too inefficient.
        // This method is useful for testing and R&D.
        // Please overload the method within derived classes.
        QL_REQUIRE(x.empty(), "can not handle given x here");

        Matrix tmp(size_, size_,0.0);

        for (Size i=0; i<size_; ++i) {
            for (Size j=0; j<=i;++j) {
                Var_Helper helper(this, i, j);
                GaussKronrodAdaptive integrator(1e-10, 10000);
                for (Size k=0; k < 64; ++k) {
                    tmp[i][j]+=integrator(helper, k*t/64.,(k+1)*t/64.);
                }
                tmp[j][i]=tmp[i][j];
            }
        }

        return tmp;
    }

}

