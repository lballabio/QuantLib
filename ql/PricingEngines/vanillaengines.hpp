
/*
 Copyright (C) 2002 Ferdinando Ametrano
 Copyright (C) 2002 Sad Rejeb
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file vanillaengines.hpp
    \brief Vanilla option engines

    \fullpath
    ql/PricingEngines/%vanillaengines.hpp
*/

// $Id$

#ifndef quantlib_vanilla_engines_h
#define quantlib_vanilla_engines_h

#include <ql/PricingEngines/genericengine.hpp>

namespace QuantLib {

    namespace PricingEngines {

        //! Vanilla engine base class
        class VanillaEngine : public GenericEngine<VanillaOptionParameters,
                                                   VanillaOptionResults> {};

        //! European engine base class
        class EuropeanEngine : public VanillaEngine {};

        //! Pricing engine for European options using analytical formulas
        class EuropeanAnalyticalEngine : public EuropeanEngine {
        public:
            void calculate() const;
        };

        //! Pricing engine for European options using finite differences
        class EuropeanFDEngine : public EuropeanEngine {
        public:
            void calculate() const;
        };

        //! Pricing engine for European options using Monte Carlo simulation
        class EuropeanMCEngine : public EuropeanEngine {
        public:
            void calculate() const;
        };

        //! Pricing engine for European options using binomial trees
        class EuropeanBinomialEngine : public EuropeanEngine {
          public:
            enum Type { CoxRossRubinstein, JarrowRudd, LeisenReimer };

            EuropeanBinomialEngine(Type type, Size steps)
            : type_(type), steps_(steps) {}
            void calculate() const;
          private:
            Type type_;
            Size steps_;
        };

        
        
        //! Pricing engine for American options
        class AmericanEngine : public VanillaEngine {};

        //! Pricing engine for American options using finite differences
        class AmericanFDEngine : public AmericanEngine {
        public:
            void calculate() const;
        };

    }

}

#endif
