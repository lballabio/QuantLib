/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

/*! \file fddividendeuropeanengine.hpp
    \brief finite-differences engine for European option with dividends
*/

#ifndef quantlib_fd_dividend_european_engine_hpp
#define quantlib_fd_dividend_european_engine_hpp

#include <ql/PricingEngines/Vanilla/fddividendengine.hpp>

namespace QuantLib {

    //! Finite-differences pricing engine for dividend European options
    /*! \ingroup vanillaengines

        \test
        - the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
        - the invariance of the results upon addition of null
          dividends is tested.

        \bug results are not overly reliable.
    */
    class FDDividendEuropeanEngine :  public DividendVanillaOption::engine,
        public FDDividendEngine {
      public:
        FDDividendEuropeanEngine(Size timeSteps = 100,
                                 Size gridPoints = 100,
                                 bool timeDependent = false)
        : FDDividendEngine(timeSteps, gridPoints, timeDependent) {}
        void calculate() const {
            setupArguments(&arguments_,
                           arguments_.getEventList());
            FDDividendEngine::calculate(&results_);
        }
    };

}


#endif
