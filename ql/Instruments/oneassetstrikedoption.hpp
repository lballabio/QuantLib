
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file oneassetstrikedoption.hpp
    \brief Option on a single asset with striked payoff
*/

#ifndef quantlib_oneassetstriked_option_h
#define quantlib_oneassetstriked_option_h

#include <ql/Instruments/oneassetoption.hpp>

namespace QuantLib {

    //! Base class for options on a single asset with striked payoff
    class OneAssetStrikedOption : public OneAssetOption {
      public:
        OneAssetStrikedOption(Option::Type type,
                              const RelinkableHandle<Quote>& underlying,
                              double strike,
                              const RelinkableHandle<TermStructure>& dividendTS,
                              const RelinkableHandle<TermStructure>& riskFreeTS,
                              const Exercise& exercise,
                              const RelinkableHandle<BlackVolTermStructure>& volTS,
                              const Handle<PricingEngine>& engine =
                              Handle<PricingEngine>(),
                              const std::string& isinCode = "",
                              const std::string& description = "");
        //! \name Instrument interface
        //@{
        class results;
        //@}
        //! \name greeks
        //@{
        double strikeSensitivity() const;
        //@}
        void setupArguments(Arguments*) const;
      protected:
        void performCalculations() const;
        // results
        mutable double strikeSensitivity_;
        // arguments
        Handle<Payoff> payoff_;
    };


    //! %results from single asset sriked option calculation
    class OneAssetStrikedOption::results : public OneAssetOption::results,
                                           public MoreGreeks {
      public:
        void reset() {
            // works on Borland, not on Visual ???
//            OneAssetOption::results::reset();
            Value::reset();
            Greeks::reset();
            MoreGreeks::reset();
        }
    };


}


#endif

