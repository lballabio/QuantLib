
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
#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

    //! Base class for options on a single asset with striked payoff
    class OneAssetStrikedOption : public OneAssetOption {
      public:
        OneAssetStrikedOption(const Handle<StrikedTypePayoff>& payoff,
                              const RelinkableHandle<Quote>& underlying,
                              const RelinkableHandle<TermStructure>& dividendTS,
                              const RelinkableHandle<TermStructure>& riskFreeTS,
                              const Exercise& exercise,
                              const RelinkableHandle<BlackVolTermStructure>& volTS,
                              const Handle<PricingEngine>& engine =
                              Handle<PricingEngine>(),
                              const std::string& isinCode = "",
                              const std::string& description = "");
        //@{
        class arguments;
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
        Handle<StrikedTypePayoff> payoff_;
    };


    //! arguments for single asset option calculation
    class OneAssetStrikedOption::arguments : public OneAssetOption::arguments {
      public:
        void validate() const;
        Handle<Payoff> payoff_;
    };

    //! %results from single asset sriked option calculation
    class OneAssetStrikedOption::results : public OneAssetOption::results,
                                           public MoreGreeks {
      public:
        void reset() {

            
            #if defined(QL_PATCH_MICROSOFT)
            OneAssetOption::results copy = *this;
            copy.reset();
            #else
            OneAssetOption::results::reset();
            #endif
            
            MoreGreeks::reset();
        }
    };


}


#endif

