
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sad Rejeb
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

/*! \file vanillaengines.hpp
    \brief Vanilla option engines
*/

#ifndef quantlib_vanilla_engines_h
#define quantlib_vanilla_engines_h

#include <ql/Instruments/vanillaoption.hpp>
#include <ql/PricingEngines/genericengine.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    namespace PricingEngines {

        //! Vanilla engine base class
        class VanillaEngine 
        : public GenericEngine<VanillaOption::arguments,
                               VanillaOption::results> {};

        //! Pricing engine for European options using analytical formulae
        class AnalyticEuropeanEngine : public VanillaEngine {
          public:
            void calculate() const;
          private:
            #if defined(QL_PATCH_SOLARIS)
            CumulativeNormalDistribution f_;
            #else
            static const CumulativeNormalDistribution f_;
            #endif
        };


        //! Pricing engine for Vanilla options using integral approach
        class IntegralEngine : public VanillaEngine {
          public:
            void calculate() const;
        };


        //! Pricing engine for Vanilla options using binomial trees
        class BinomialVanillaEngine : public VanillaEngine {
          public:
            enum Type {CoxRossRubinstein, JarrowRudd, EQP, Trigeorgis, Tian};

            BinomialVanillaEngine(Type type, Size timeSteps)
            : type_(type), timeSteps_(timeSteps) {}
            void calculate() const;
          private:
            Type type_;
            Size timeSteps_;
        };

    }

}


#endif
