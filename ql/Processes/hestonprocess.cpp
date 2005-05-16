/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Processes/hestonprocess.hpp>
#include <ql/Processes/eulerdiscretization.hpp>
#include <ql/Math/pseudosqrt.hpp>

namespace QuantLib {

    HestonProcess::HestonProcess(
                              const Handle<YieldTermStructure>& riskFreeRate,
                              const Handle<YieldTermStructure>& dividendYield,
                              const Handle<Quote>& s0,
                              Real v0, Real kappa, Real theta,
                              Real sigma, Real rho)
    : GenericStochasticProcess(boost::shared_ptr<discretization>(
                                                    new EulerDiscretization)),
      riskFreeRate_(riskFreeRate), dividendYield_(dividendYield), s0_(s0),
      v0_(v0), kappa_(kappa), theta_(theta), sigma_(sigma), rho_  (rho) {

        registerWith(riskFreeRate_);
        registerWith(dividendYield_);
        registerWith(s0_);
    }

    Size HestonProcess::size() const {
        return 2;
    }

    Disposable<Array> HestonProcess::initialValues() const {
        Array tmp(2);
        tmp[0] = s0_->value();
        tmp[1] = v0_;
        return tmp;
    }

    Disposable<Array> HestonProcess::drift(Time t, const Array& x) const {
        Array tmp(2);
        const Real vol = x[1] > 0 ? std::sqrt(x[1]) : 0.0;
        tmp[0] = riskFreeRate_->forwardRate(t, t, Continuous)
               - dividendYield_->forwardRate(t, t, Continuous)
               - 0.5 * vol * vol;
        tmp[1] = kappa_*(theta_ - x[1]);
        return tmp;
    }

    Disposable<Matrix> HestonProcess::diffusion(Time t, const Array& x) const {
        /* the correlation matrix is
           |  1   rho |
           | rho   1  |
           whose square root (which is used here) is
           |  1          0       |
           | rho   sqrt(1-rho^2) |
        */
        Matrix tmp(2,2);
        Real sigma1 = x[1] > 0 ? std::sqrt(x[1]) : 0.0;
        Real sigma2 = sigma_ * sigma1;
        tmp[0][0] = sigma1;       tmp[0][1] = 0.0;
        tmp[1][0] = rho_*sigma2;  tmp[1][1] = std::sqrt(1.0-rho_*rho_)*sigma2;
        return tmp;
    }

    #ifndef QL_DISABLE_DEPRECATED
    Disposable<Array> HestonProcess::evolve(const Array& change,
                                            const Array& currentValue) const {
        return apply(currentValue,change);
    }
    #endif

    Disposable<Array> HestonProcess::apply(const Array& x0,
                                           const Array& dx) const {
        Array tmp(2);
        tmp[0] = x0[0] * std::exp(dx[0]);
        tmp[1] = x0[1] + dx[1];
        return tmp;
    }

    Real HestonProcess::s0() const {
        return s0_->value();
    }

    Real HestonProcess::v0() const {
        return v0_;
    }

    Real HestonProcess::rho() const {
        return rho_;
    }

    Real HestonProcess::kappa() const {
        return kappa_;
    }

    Real HestonProcess::theta() const {
        return theta_;
    }

    Real HestonProcess::sigma() const {
        return sigma_;
    }

    const boost::shared_ptr<YieldTermStructure>&
    HestonProcess::dividendYield() const {
        return dividendYield_.currentLink();
    }

    const boost::shared_ptr<YieldTermStructure>&
    HestonProcess::riskFreeRate() const {
        return riskFreeRate_.currentLink();
    }

    Time HestonProcess::time(const Date& d) const {
        return riskFreeRate_->dayCounter().yearFraction(
                                           riskFreeRate_->referenceDate(), d);
    }

}
