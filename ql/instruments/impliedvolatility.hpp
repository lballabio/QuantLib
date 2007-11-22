/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file impliedvolatility.hpp
    \brief Utilities for implied-volatility calculation
*/

#ifndef quantlib_implied_volatility_hpp
#define quantlib_implied_volatility_hpp

#include <ql/instrument.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! helper class for one-asset implied-volatility calculation
    /*! The passed engine must be linked to the passed quote (see,
        e.g., VanillaOption to see how this can be achieved.)
    */
    class ImpliedVolatilityHelper {
      public:
        static Volatility calculate(const Instrument& instrument,
                                    const PricingEngine& engine,
                                    SimpleQuote& volQuote,
                                    Real targetValue,
                                    Real accuracy,
                                    Size maxEvaluations,
                                    Volatility minVol,
                                    Volatility maxVol);
        // utilities

        /*! The returned process is equal to the passed one, except
            for the volatility which is flat and whose value is driven
            by the passed quote.
        */
        static boost::shared_ptr<GeneralizedBlackScholesProcess> clone(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             const boost::shared_ptr<SimpleQuote>& volQuote);
    };

}


#endif

