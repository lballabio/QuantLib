/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file analyticbarrierengine.hpp
    \brief Analytic barrier option engines
*/

#ifndef quantlib_analytic_barrier_engine_hpp
#define quantlib_analytic_barrier_engine_hpp

#include <ql/instruments/barrieroption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    //! Pricing engine for barrier options using analytical formulae
    /*! The formulas are taken from "Option pricing formulas",
         E.G. Haug, McGraw-Hill, p.69 and following.

        \ingroup barrierengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    class AnalyticBarrierEngine : public BarrierOption::engine {
      public:
        AnalyticBarrierEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        CumulativeNormalDistribution f_;
        // helper methods
        Real underlying() const;
        Real strike() const;
        Volatility volatility() const;
        Real barrier() const;
        Real rebate() const;
        Real stdDeviation() const;
        Rate riskFreeRate() const;
        DiscountFactor riskFreeDiscount() const;
        Rate dividendYield() const;
        DiscountFactor dividendDiscount() const;
        Rate mu() const;
        Real muSigma() const;
        Real A(Real phi) const;
        Real B(Real phi) const;
        Real C(Real eta, Real phi) const;
        Real D(Real eta, Real phi) const;
        Real E(Real eta) const;
        Real F(Real eta) const;
    };

}


#endif


#ifndef id_6000d8f507bab79363e656b7dd9a6df6
#define id_6000d8f507bab79363e656b7dd9a6df6
inline bool test_6000d8f507bab79363e656b7dd9a6df6(int* i) { return i != 0; }
#endif
