
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file binaryoption.hpp
    \brief Binary option on a single asset
*/

#ifndef quantlib_binary_option_h
#define quantlib_binary_option_h

#include <ql/option.hpp>
#include <ql/PricingEngines/vanillaengines.hpp>

namespace QuantLib {

    //! placeholder for enumerated binary types
    struct Binary {
        enum Type { CashAtHit, CashAtExpiry };
    };

    namespace Instruments {

        //! Binary option on a single asset

        class BinaryOption : public Option {
          public:            
              BinaryOption(Binary::Type binaryType,
                          double barrier,
                          double cashPayoff,
                          Option::Type type,
                          const RelinkableHandle<MarketElement>& underlying,
                          const RelinkableHandle<TermStructure>& dividendTS,
                          const RelinkableHandle<TermStructure>& riskFreeTS,
                          const Exercise& exercise,
                          const RelinkableHandle<BlackVolTermStructure>& volTS,
                          const Handle<PricingEngine>& engine =
                                                   Handle<PricingEngine>(),
                          const std::string& isinCode = "",
                          const std::string& description = "");
            //! \name greeks
            //@{
            double delta() const;
            double gamma() const;
            double theta() const;
            double vega() const;
            double rho() const;
            double dividendRho() const;
            double strikeSensitivity() const;
            //@}
            bool isExpired() const;       
            void setupArguments(Arguments*) const;
          protected:
            void setupExpired() const;            
            void performCalculations() const;
            // results
            mutable double delta_, gamma_, theta_, 
                           vega_, rho_, dividendRho_, strikeSensitivity_;
            // arguments
            Binary::Type binaryType_;
            double barrier_;
            double cashPayoff_;
            Option::Type type_;
            RelinkableHandle<MarketElement> underlying_;            
            Exercise exercise_;
            RelinkableHandle<TermStructure> riskFreeTS_, dividendTS_;
            RelinkableHandle<BlackVolTermStructure> volTS_;
          private:
            
        };

        //! arguments for barrier option calculation
        class BinaryOptionArguments : public PricingEngines::VanillaOptionArguments {
          public:
              Binary::Type binaryType;
              double barrier;
              double cashPayoff;
              void validate() const;
        };

        inline void BinaryOptionArguments::validate() const {
            PricingEngines::VanillaOptionArguments::validate();
            // when should enums be checked?
            //QL_REQUIRE(binaryType != -1,
            //           "BinaryOptionArguments::validate() : "
            //           "null binaryType given");            
        }

        //! %results from barrier option calculation
        class BinaryOptionResults 
            : public virtual PricingEngines::VanillaOptionResults {};


    }


}


#endif

