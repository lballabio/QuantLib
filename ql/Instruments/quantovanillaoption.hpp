
/*
 Copyright (C) 2002 Ferdinando Ametrano

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
/*! \file quantovanillaoption.hpp
    \brief Quanto version of a vanilla option

    \fullpath
    ql/Instruments/%quantovanillaoption.hpp
*/

// $Id$

#ifndef quantlib_quanto_vanilla_option_h
#define quantlib_quanto_vanilla_option_h

#include <ql/option.hpp>
#include <ql/termstructure.hpp>
#include <ql/solver1d.hpp>
#include <ql/PricingEngines/quantoengines.hpp>
#include <ql/Instruments/vanillaoption.hpp>

namespace QuantLib {

    namespace Instruments {

        //! Quanto version of a vanilla option
        class QuantoVanillaOption : public VanillaOption {
          public:
            QuantoVanillaOption(
                Option::Type type,
                const RelinkableHandle<MarketElement>& underlying,
                double strike,
                const RelinkableHandle<TermStructure>& dividendYield,
                const RelinkableHandle<TermStructure>& riskFreeRate,
                const Date& exerciseDate,
                const RelinkableHandle<MarketElement>& volatility,
//                const Handle<PricingEngines::QuantoVanillaEngine>& engine,
                const Handle<PricingEngine>& engine,
                const RelinkableHandle<TermStructure>& foreignRiskFreeRate,
                const RelinkableHandle<MarketElement>& exchangeRateVolatility,
                const RelinkableHandle<MarketElement>& correlation,
                const std::string& isinCode = "",
                const std::string& description = "");
            //! \name greeks
            //@{
            double qvega() const;
            double qrho() const;
            double qlambda() const;
            //@}
          protected:
            void setupEngine() const;
            void performCalculations() const;
          private:
            // arguments
            RelinkableHandle<TermStructure> foreignRiskFreeRate_;
            RelinkableHandle<MarketElement> exchangeRateVolatility_;
            RelinkableHandle<MarketElement> correlation_;
            // results
            mutable double qvega_, qrho_, qlambda_;
        };

    }


}


#endif

