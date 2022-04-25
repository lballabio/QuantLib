/*
 Copyright (C) 2021 Skandinaviska Enskilda Banken AB (publ)

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

/*! \file ql/experimental/asian/analytic_discr_arith_av_price.hpp
    \brief Turnbull Wakeman moment-matching Asian option Engine
    \ingroup asianengines
*/

#ifndef quantlib_analytic_discrete_arithmetic_average_price_asian_engine_hpp
#define quantlib_analytic_discrete_arithmetic_average_price_asian_engine_hpp

#include <ql/instruments/asianoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    /*! Turnbull Wakeman two moment-matching Asian option Engine
        Analytical pricing based on the two-moment Turnbull-Wakeman
        approximation.
        References: "Commodity Option Pricing", Iain Clark, Wiley, section 2.7.4.
                    "Option Pricing Formulas, Second Edition", E.G. Haug, 2006, pp. 192-202.

        \test
        - the correctness of the returned value is tested by reproducing
          results in literature with flat as well as upward and downward
          sloping volatility term structures.

        \todo Add support for Asian options on futures
    */
    class AnalyticDiscreteArithmeticAveragePriceAsianEngine
    : public DiscreteAveragingAsianOption::engine {
      public:
        AnalyticDiscreteArithmeticAveragePriceAsianEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process)
        : process_(process) {
            registerWith(process_);
        }

        void calculate() const;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

}

#endif
