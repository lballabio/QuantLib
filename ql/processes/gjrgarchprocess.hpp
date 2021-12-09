/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Yee Man Chan

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

/*! \file gjrgarchprocess.hpp
    \brief GJR-GARCH(1,1) stochastic process
*/

#ifndef quantlib_gjrgarch_process_hpp
#define quantlib_gjrgarch_process_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Stochastic-volatility GJR-GARCH(1,1) process
    // parameters supplied should be daily constants
    // they are annualized by setting the parameter daysPerYear
    /*! This class describes the stochastic volatility
        process governed by
        \f[
        \begin{array}{rcl}
        dS(t, S)  &=& \mu S dt + \sqrt{v} S dW_1 \\
        dv(t, S)  &=& (\omega + (\beta + \alpha * q_{2} 
        + \gamma * q_{3} - 1) v) dt + (\alpha \sigma_{12} 
        + \gamma \sigma_{13}) v dW_1 
        + \sqrt{\alpha^{2} (\sigma^{2}_{2} - \sigma^{2}_{12}) 
        + \gamma^{2} (\sigma^{2}_{3} - \sigma^{2}_{13}) 
        + 2 \alpha \gamma (\sigma_{23} - \sigma_{12} \sigma_{13})} v dW_2 \ \
        N = normalCDF(\lambda) \\
        n &=& \exp{-\lambda^{2}/2} / \sqrt{2 \pi} \\
        q_{2} &=& 1 + \lambda^{2} \\
        q_{3} &=& \lambda n + N + \lambda^2 N \\
        \sigma^{2}_{2} = 2 + 4 \lambda^{4} \\
        \sigma^{2}_{3} = \lambda^{3} n + 5 \lambda n + 3N 
        + \lambda^{4} N + 6 \lambda^{2} N -\\lambda^{2} n^{2} - N^{2} 
        - \lambda^{4} N^{2} - 2 \lambda n N - 2 \lambda^{3} nN 
        - 2 \lambda^{2} N^{2} \                 \
        \sigma_{12} = -2 \lambda \\
        \sigma_{13} = -2 n - 2 \lambda N \\
        \sigma_{23} = 2N + \sigma_{12} \sigma_{13} \\
        \end{array}
        \f]

        \ingroup processes
    */
    class GJRGARCHProcess : public StochasticProcess {
      public:
        enum Discretization { PartialTruncation, FullTruncation,
                              Reflection};

        GJRGARCHProcess(Handle<YieldTermStructure> riskFreeRate,
                        Handle<YieldTermStructure> dividendYield,
                        Handle<Quote> s0,
                        Real v0,
                        Real omega,
                        Real alpha,
                        Real beta,
                        Real gamma,
                        Real lambda,
                        Real daysPerYear = 252.0,
                        Discretization d = FullTruncation);

        Size size() const override;
        Disposable<Array> initialValues() const override;
        Disposable<Array> drift(Time t, const Array& x) const override;
        Disposable<Matrix> diffusion(Time t, const Array& x) const override;
        Disposable<Array> apply(const Array& x0, const Array& dx) const override;
        Disposable<Array> evolve(Time t0, const Array& x0, Time dt, const Array& dw) const override;

        Real v0()     const { return v0_; }
        Real lambda() const { return lambda_; }
        Real omega()  const { return omega_; }
        Real alpha()  const { return alpha_; }
        Real beta()   const { return beta_; }
        Real gamma()  const { return gamma_; }
        Real daysPerYear()  const { return daysPerYear_; }

        const Handle<Quote>& s0() const;
        const Handle<YieldTermStructure>& dividendYield() const;
        const Handle<YieldTermStructure>& riskFreeRate() const;

        Time time(const Date&) const override;

      private:
        Handle<YieldTermStructure> riskFreeRate_, dividendYield_;
        Handle<Quote> s0_;
        Real v0_, omega_, alpha_, beta_, gamma_, lambda_, daysPerYear_;
        Discretization discretization_;
    };

}


#endif
