/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Colin Alberts

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file roughhestonmodel.hpp
    \brief rough Heston model for the stochastic volatility of an asset
*/

#ifndef quantlib_rough_heston_model_hpp
#define quantlib_rough_heston_model_hpp

#include <ql/handle.hpp>
#include <ql/models/model.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! rough Heston model for the stochastic volatility of an asset
    /*! The variance follows a fractional (Volterra) square-root process
        \f[
        \begin{array}{rcl}
        dS_t &=& (r-q) S_t dt + S_t\sqrt{V_t}\,dW_t \\
        V_t  &=& V_0 + \frac{1}{\Gamma(\alpha)}\int_0^t (t-s)^{\alpha-1}
                 \kappa(\theta - V_s)\,ds
               + \frac{1}{\Gamma(\alpha)}\int_0^t (t-s)^{\alpha-1}
                 \sigma\sqrt{V_s}\,dB_s \\
        dW_t\,dB_t &=& \rho\,dt
        \end{array}
        \f]
        with \f$ \alpha = H + \frac{1}{2} \f$ and Hurst exponent
        \f$ H \in (0, \frac{1}{2}] \f$.  For \f$ H = \frac{1}{2} \f$ the
        kernel is constant and the model reduces exactly to the classical
        Heston model with the same parameters.  Small values of the Hurst
        exponent (empirically \f$ H \approx 0.1 \f$) generate the steep
        short-maturity at-the-money implied-volatility skew, exploding like
        \f$ T^{H-1/2} \f$, that classical stochastic-volatility models
        cannot reproduce.

        Since the variance process is neither Markovian nor a semimartingale,
        the model is not backed by a StochasticProcess instance; the yield
        term structures and the spot quote are held directly.

        References:

        O. El Euch and M. Rosenbaum, The characteristic function of rough
        Heston models, Mathematical Finance 29(1), 3-38 (2019).

        J. Gatheral, T. Jaisson and M. Rosenbaum, Volatility is rough,
        Quantitative Finance 18(6), 933-949 (2018).

        \test calibration is tested against known synthetic parameters.
    */
    class RoughHestonModel : public CalibratedModel {
      public:
        RoughHestonModel(Handle<YieldTermStructure> riskFreeRate,
                         Handle<YieldTermStructure> dividendYield,
                         Handle<Quote> s0,
                         Real v0,
                         Real kappa,
                         Real theta,
                         Real sigma,
                         Real rho,
                         Real hurst);

        // variance mean reversion level
        Real theta() const { return arguments_[0](0.0); }
        // variance mean reversion speed
        Real kappa() const { return arguments_[1](0.0); }
        // volatility of the volatility
        Real sigma() const { return arguments_[2](0.0); }
        // correlation
        Real rho()   const { return arguments_[3](0.0); }
        // spot variance
        Real v0()    const { return arguments_[4](0.0); }
        // Hurst exponent of the variance process
        Real hurst() const { return arguments_[5](0.0); }

        const Handle<YieldTermStructure>& riskFreeRate() const {
            return riskFreeRate_;
        }
        const Handle<YieldTermStructure>& dividendYield() const {
            return dividendYield_;
        }
        const Handle<Quote>& s0() const { return s0_; }

      private:
        const Handle<YieldTermStructure> riskFreeRate_, dividendYield_;
        const Handle<Quote> s0_;
    };
}

#endif
