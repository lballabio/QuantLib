/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file piecewisetimedependenthestonprocess.hpp
    \brief PTD Heston stochastic process
*/

#ifndef quantlib_piecewise_time_dependent_heston_process_hpp
#define quantlib_piecewise_time_dependent_heston_process_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/quote.hpp>
#include <ql/timegrid.hpp>
#include <ql/models/model.hpp>

namespace QuantLib {

    //! Square-root stochastic-volatility Heston process
    /*! This class describes the square root stochastic volatility
        process governed by
        \f[
        \begin{array}{rcl}
        dS(t, S)  &=& \mu S dt + \sqrt{v} S dW_1 \\
        dv(t, S)  &=& \kappa(t) (\theta(t) - v) dt + \sigma(t) \sqrt{v} dW_2 \\
        dW_1 dW_2 &=& \rho(t) dt
        \end{array}
        where \kappa, \theta, \sigma and \rho are all piecewise
        constant in time
        \f]

        \ingroup processes
    */
    class PiecewiseTimeDependentHestonProcess : public StochasticProcess {
      public:
        enum Discretization { PartialTruncation,
                              FullTruncation,
                              Reflection };

        PiecewiseTimeDependentHestonProcess(
                         Handle<YieldTermStructure> riskFreeRate,
                         Handle<YieldTermStructure> dividendYield,
                         Handle<Quote> s0,
                         Real v0,
                         const Parameter& kappa, // HestonProcess is kappa, theta... PTD Model is theta, kappa
                         const Parameter& theta,
                         const Parameter& sigma,
                         const Parameter& rho,
                         TimeGrid timeGrid,
                         Discretization d = PartialTruncation);

        Size size() const override;
        Size factors() const override;

        Disposable<Array> initialValues() const override;
        Disposable<Array> drift(Time t, const Array& x) const override;
        Disposable<Matrix> diffusion(Time t, const Array& x) const override;
        Disposable<Array> apply(const Array& x0, const Array& dx) const override;
        Disposable<Array> evolve(Time t0, const Array& x0, Time dt, const Array& dw) const override;

        Real v0()                const { return v0_; }
        const Parameter& rho()   const { return rho_; }
        const Parameter& kappa() const { return kappa_; }
        const Parameter& theta() const { return theta_; }
        const Parameter& sigma() const { return sigma_; }

        const Handle<Quote>& s0() const;
        const TimeGrid& timeGrid() const;
        const Handle<YieldTermStructure>& dividendYield() const;
        const Handle<YieldTermStructure>& riskFreeRate() const;

        Time time(const Date&) const override;

      private:
        Handle<YieldTermStructure> riskFreeRate_, dividendYield_;
        const TimeGrid timeGrid_;
        Handle<Quote> s0_;
        Real v0_;
        Parameter kappa_, theta_, sigma_, rho_;
        Discretization discretization_;
    };
}
#endif
