
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
/*! \file forwardvanillaoption.hpp
    \brief Forward version of a vanilla option

    \fullpath
    ql/Instruments/%forwardvanillaoption.hpp
*/

// $Id$

#ifndef quantlib_forward_vanilla_option_h
#define quantlib_forward_vanilla_option_h

#include <ql/PricingEngines/forwardengines.hpp>
#include <ql/Instruments/vanillaoption.hpp>

namespace QuantLib {

    namespace Instruments {

        //! Forward version of a vanilla option
        class ForwardVanillaOption : public VanillaOption {
          public:
            ForwardVanillaOption(
                Option::Type type,
                const RelinkableHandle<MarketElement>& underlying,
                const RelinkableHandle<TermStructure>& dividendTS,
                const RelinkableHandle<TermStructure>& riskFreeTS,
                const Exercise& exercise,
                const RelinkableHandle<BlackVolTermStructure>& volTS,
//                const Handle<PricingEngines::ForwardEngine>& engine,
                const Handle<PricingEngine>& engine,
                double moneyness,
                Date resetDate,
                const std::string& isinCode = "",
                const std::string& description = "");
          protected:
            void setupEngine() const;
            void performCalculations() const;
          private:
            // arguments
            double moneyness_;
            Date resetDate_;
        };

    }


}


#endif

