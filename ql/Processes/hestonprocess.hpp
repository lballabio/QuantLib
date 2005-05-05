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

/*! \file hestonprocess.hpp
    \brief Heston stochastic process
*/

#ifndef quantlib_heston_process_hpp
#define quantlib_heston_process_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/yieldtermstructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Square-root stochastic-volatility Heston process
    /*! This class describes the square root stochastic volatility
        process governed by
        \f[
        \begin{eqnarray*}
        dS(t, S) = \mu S dt + \sqrt(v_t) S dW1_t \\
        dv_t     = \kappa (\theta - v_t) dt + \sigma \sqrt{v_t} dW2_t \\
        dW1 dW2  = \rho dt
        \end{eqnarray*}
        \f]
    */
    class HestonProcess : public GenericStochasticProcess {
      public:
        HestonProcess(const Handle<YieldTermStructure>& riskFreeRate,
                      const Handle<YieldTermStructure>& dividendYield,
                      const Handle<Quote>& s0,
                      Real v0, Real kappa,
                      Real theta, Real sigma, Real rho);

        Size size() const;
        Disposable<Array> initialValues() const;
        Disposable<Array> drift(Time t, const Array& x) const;
        Disposable<Matrix> diffusion(Time t, const Array& x) const;
        Disposable<Array> evolve(const Array& change,
                                 const Array& currentValue) const;

        Real s0()    const;
        Real v0()    const;
        Real rho()   const;
        Real kappa() const;
        Real theta() const;
        Real sigma() const;

        const boost::shared_ptr<YieldTermStructure>& dividendYield() const;
        const boost::shared_ptr<YieldTermStructure>& riskFreeRate() const;

        Time time(const Date&) const;
      private:
        Handle<YieldTermStructure> riskFreeRate_, dividendYield_;
        Handle<Quote> s0_;
        Real v0_, kappa_, theta_, sigma_, rho_;
    };

}


#endif
