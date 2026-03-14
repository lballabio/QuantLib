/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file analytictwoassetbarrierengine.hpp
    \brief Analytic engine for barrier option on two assets
*/

#ifndef quantlib_analytic_two_asset_barrier_engine_hpp
#define quantlib_analytic_two_asset_barrier_engine_hpp

#include <ql/instruments/twoassetbarrieroption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Analytic engine for %barrier %option on two assets
    /*! The formulas by Heynen and Kat are taken from Haug, "Option
        pricing formulas".

        \ingroup barrierengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
     */
    class AnalyticTwoAssetBarrierEngine
        : public TwoAssetBarrierOption::engine {
      public:
        AnalyticTwoAssetBarrierEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
                                      ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
                                      Handle<Quote> rho);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process1_;
        ext::shared_ptr<GeneralizedBlackScholesProcess> process2_;
        Handle<Quote> rho_;

        // helper methods
        Real underlying1() const;
        Real underlying2() const;

        Real strike() const;
        Time residualTime() const;

        Volatility volatility1() const;
        Volatility volatility2() const;

        Real barrier() const;
        Real rho() const;

        Rate riskFreeRate() const;

        Rate dividendYield1() const;
        Rate dividendYield2() const;

        Rate costOfCarry1() const;
        Rate costOfCarry2() const;

        Real mu(Real b, Real vol) const;

        Real d1() const;
        Real d2() const;
        Real d3() const;
        Real d4() const;

        Real e1() const;
        Real e2() const;
        Real e3() const;
        Real e4() const;

        Real call() const;
        Real put() const;

        Real A(Real eta, Real phi) const;
        Real B(Real eta, Real phi) const;

        Real M(Real m_a, Real m_b,Real rho) const;
    };

}


#endif
