/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou
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

/*! \file analyticcontinuousfixedlookback.hpp
    \brief Analytic engine for continuous fixed-strike lookback
*/

#ifndef quantlib_analytic_continuous_fixed_lookback_engine_hpp
#define quantlib_analytic_continuous_fixed_lookback_engine_hpp

#include <ql/instruments/lookbackoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    //! Pricing engine for European continuous fixed-strike lookback
    /*! Formula from "Option Pricing Formulas",
        E.G. Haug, McGraw-Hill, 1998, p.63-64

        \ingroup lookbackengines

        \test returned values are verified against results from literature
    */
    class AnalyticContinuousFixedLookbackEngine
        : public ContinuousFixedLookbackOption::engine {
      public:
        AnalyticContinuousFixedLookbackEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        CumulativeNormalDistribution f_;
        // helper methods
        Real underlying() const;
        Real strike() const;
        Time residualTime() const;
        Volatility volatility() const;
        Real minmax() const;
        Real stdDeviation() const;
        Rate riskFreeRate() const;
        DiscountFactor riskFreeDiscount() const;
        Rate dividendYield() const;
        DiscountFactor dividendDiscount() const;
        Real A(Real eta) const;
        Real B(Real eta) const;
        Real C(Real eta) const;
    };

}


#endif


#ifndef id_f3ebcb998626a2c9961f9de929715a89
#define id_f3ebcb998626a2c9961f9de929715a89
inline bool test_f3ebcb998626a2c9961f9de929715a89(const int* i) {
    return i != nullptr;
}
#endif
