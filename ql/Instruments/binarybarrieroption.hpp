
/*
 Copyright (C) 2003 Neil Firth
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

/*! \file binarybarrieroption.hpp
    \brief Binary barrier option on a single asset
*/

#ifndef quantlib_binarybarrier_option_h
#define quantlib_binarybarrier_option_h

#include <ql/Instruments/oneassetoption.hpp>

namespace QuantLib {

    //! placeholder for enumerated binary barrier types
    struct BinaryBarrier {
        enum Type { CashAtHit, CashAtExpiry };
    };

    //! Binary barrier option on a single asset.
    /*! Depending on the exercise type, either the European or American
        analytic pricing engine will be used if none is given.
    */
    class BinaryBarrierOption : public OneAssetOption {
      public:
        class arguments;
        class results;
        BinaryBarrierOption(BinaryBarrier::Type binaryBarrierType,
                     double barrier,
                     double cashPayoff,
                     Option::Type type,
                     const RelinkableHandle<Quote>& underlying,
                     const RelinkableHandle<TermStructure>& dividendTS,
                     const RelinkableHandle<TermStructure>& riskFreeTS,
                     const Exercise& exercise,
                     const RelinkableHandle<BlackVolTermStructure>& volTS,
                     const Handle<PricingEngine>& engine =
                     Handle<PricingEngine>(),
                     const std::string& isinCode = "",
                     const std::string& description = "");
        bool isExpired() const;
        void setupArguments(Arguments*) const;
      protected:
        void performCalculations() const;
        // results
        // arguments
        BinaryBarrier::Type binaryBarrierType_;
        double barrier_;
        double cashPayoff_;
        Option::Type type_;
    };

    //! %arguments for binary barrier option calculation
    class BinaryBarrierOption::arguments : public OneAssetOption::arguments {
      public:
        arguments() : barrier(Null<double>()), 
                      cashPayoff(Null<double>()) {}
        BinaryBarrier::Type binaryBarrierType;
        double barrier;
        double cashPayoff;
        void validate() const;
    };

    //! %results from binary barrier option calculation
    class BinaryBarrierOption::results : public virtual OneAssetOption::results {};

}


#endif

