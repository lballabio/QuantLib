
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
/*! \file quantoengines.hpp
    \brief Quanto (no discrete dividends, no barriers) option engines

    \fullpath
    ql/Instruments/%quantoengines.hpp
*/

// $Id$

#ifndef quantlib_quanto_engines_h
#define quantlib_quanto_engines_h

#include <ql/PricingEngines/genericengine.hpp>


namespace QuantLib {

    namespace PricingEngines {

        //! Quanto engine base class
        class QuantoEngine : public GenericEngine<QuantoOptionParameters,
                                                  QuantoOptionResults> {};

        //! Quanto European engine base class
        class QuantoEuropeanEngine : public QuantoEngine {};

        //! Pricing engine for Quanto European options using analytical formulas
        class QuantoEuropeanAnalyticalEngine : public QuantoEuropeanEngine {
        public:
            void calculate() const;
        };

        //! Pricing engine for Quanto European options using Finite Differences
        class QuantoEuropeanFDEngine : public QuantoEuropeanEngine {
        public:
            void calculate() const;
        };

        //! Pricing engine for Quanto European options using Monte Carlo simulation
        class QuantoEuropeanMCEngine : public QuantoEuropeanEngine {
        public:
            void calculate() const;
        };

        //! Quanto American engine base class
        class QuantoAmericanEngine : public QuantoEngine {};

        //! Pricing engine for Quanto American options using Finite Differences
        class QuantoAmericanFDEngine : public QuantoAmericanEngine {
        public:
            void calculate() const;
        };

    }
}

#endif

