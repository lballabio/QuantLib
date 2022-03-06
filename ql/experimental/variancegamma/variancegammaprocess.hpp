/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2010 Adrian O' Neill

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

/*! \file variancegammaprocess.hpp
    \brief Variance Gamma stochastic process
*/

#ifndef quantlib_variance_gamma_process_hpp
#define quantlib_variance_gamma_process_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Variance gamma process

    /*! This class describes the stochastic volatility
    process.  With a Brownian motion given by
    \f[
        db = \theta dt + \sigma dW_t
    \f]
    then a Variance Gamma process X is defined by evaluating this
    Brownian motion at sample times driven by a Gamma process. If T is
    the value of a Gamma process with mean 1 and variance rate \f$ \nu
    \f$ then the Variance Gamma process is given by
    \f[
        X(t) = B(T)
    \f]

    \ingroup processes
    */
    class VarianceGammaProcess : public StochasticProcess1D {
    public:
      VarianceGammaProcess(Handle<Quote> s0,
                           Handle<YieldTermStructure> dividendYield,
                           Handle<YieldTermStructure> riskFreeRate,
                           Real sigma,
                           Real nu,
                           Real theta);

      Real x0() const override;
      Real drift(Time t, Real x) const override;
      Real diffusion(Time t, Real x) const override;

      Real sigma() const { return sigma_; }
      Real nu() const { return nu_; }
      Real theta() const { return theta_; }

      const Handle<Quote>& s0() const;
      const Handle<YieldTermStructure>& dividendYield() const;
      const Handle<YieldTermStructure>& riskFreeRate() const;

    private:
        Handle<Quote> s0_;
        Handle<YieldTermStructure> dividendYield_, riskFreeRate_;
        Real sigma_, nu_, theta_;
    };

}


#endif



#ifndef id_265d8426c48a026840b1bb20ef1193d5
#define id_265d8426c48a026840b1bb20ef1193d5
inline bool test_265d8426c48a026840b1bb20ef1193d5(int* i) { return i != 0; }
#endif
