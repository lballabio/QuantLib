
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file quantoforwardvanillaoption.hpp
    \brief Quanto version of a forward vanilla option
*/

#ifndef quantlib_quanto_forward_vanilla_option_h
#define quantlib_quanto_forward_vanilla_option_h

#include <ql/Instruments/quantovanillaoption.hpp>

namespace QuantLib {

    namespace Instruments {

        //! Quanto version of a forward vanilla option
        class QuantoForwardVanillaOption : public QuantoVanillaOption {
          public:
            QuantoForwardVanillaOption(
                Option::Type type,
                const RelinkableHandle<MarketElement>& underlying,
                double strike,
                const RelinkableHandle<TermStructure>& dividendTS,
                const RelinkableHandle<TermStructure>& riskFreeTS,
                const Exercise& exercise,
                const RelinkableHandle<BlackVolTermStructure>& volTS,
                const Handle<PricingEngine>& engine,
                const RelinkableHandle<TermStructure>& foreignRiskFreeTS,
                const RelinkableHandle<BlackVolTermStructure>& exchRateVolTS,
                const RelinkableHandle<MarketElement>& correlation,
                double moneyness,
                Date resetDate,
                const std::string& isinCode = "",
                const std::string& description = "");
            void setupArguments(Arguments*) const;
          private:
            // arguments
            double moneyness_;
            Date resetDate_;
        };

    }

}


#endif

