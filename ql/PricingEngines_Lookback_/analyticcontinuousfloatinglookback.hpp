/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou

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

/*! \file analyticcontinuousfloatinglookback.hpp
    \brief Analytic engine for continuous floating-strike lookback
*/

#ifndef quantlib_analytic_continuous_floating_lookback_engine_hpp
#define quantlib_analytic_continuous_floating_lookback_engine_hpp

#include <ql/Instruments/lookbackoption.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    //! Pricing engine for European continuous floating-strike lookback
    /*! Formula from "Option Pricing Formulas",
        E.G. Haug, McGraw-Hill, 1998, p.61-62

        \ingroup lookbackengines

        \test returned values verified against results from literature
    */
    class AnalyticContinuousFloatingLookbackEngine
        : public ContinuousFloatingLookbackOption::engine {
      public:
        void calculate() const;
      private:
        CumulativeNormalDistribution f_;
        // helper methods
        Real underlying() const;
        Time residualTime() const;
        Volatility volatility() const;
        Real minmax() const;
        Real stdDeviation() const;
        Rate riskFreeRate() const;
        DiscountFactor riskFreeDiscount() const;
        Rate dividendYield() const;
        DiscountFactor dividendDiscount() const;
        Real A(Real eta) const;
    };

}


#endif
