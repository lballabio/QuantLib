
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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
                const RelinkableHandle<TermStructure>& dividendTS,
                const RelinkableHandle<TermStructure>& riskFreeTS,
                const Exercise& exercise,
                const RelinkableHandle<BlackVolTermStructure>& volTS,
                const Handle<PricingEngine>& engine,
                const RelinkableHandle<TermStructure>& foreignRiskFreeTS,
                const RelinkableHandle<BlackVolTermStructure>& exchRateVolTS,
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
            // arguments
            RelinkableHandle<TermStructure> foreignRiskFreeTS_;
            RelinkableHandle<BlackVolTermStructure> exchRateVolTS_;
            RelinkableHandle<MarketElement> correlation_;
            // results
            mutable double qvega_, qrho_, qlambda_;
        };

    }


}


#endif

