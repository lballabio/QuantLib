
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
/*! \file forwardengines.hpp
    \brief Forward (strike-resetting) option engines

    \fullpath
    ql/PricingEngines/%forwardengines.hpp
*/

// $Id$

#ifndef quantlib_forward_engines_h
#define quantlib_forward_engines_h

#include <ql/PricingEngines/vanillaengines.hpp>

namespace QuantLib {

    namespace PricingEngines {

        //! arguments for forward (strike-resetting) option calculation
        template<class ArgumentsType>
        class ForwardOptionArguments : public ArgumentsType {
          public:
            ForwardOptionArguments() : moneyness(Null<double>()),
                                        resetTime(Null<double>()) {}
            void validate() const;
            double moneyness;
            Time resetTime;
        };

        template<class ArgumentsType>
        void ForwardOptionArguments<ArgumentsType>::validate() const {
            ArgumentsType::validate();
            QL_REQUIRE(moneyness != Null<double>(),
                       "ForwardOptionArguments::validate() : "
                       "null moneyness given");
            QL_REQUIRE(moneyness > 0.0,
                       "ForwardOptionArguments::validate() : "
                       "negative or zero moneyness given");
            QL_REQUIRE(resetTime != Null<double>(),
                       "ForwardOptionArguments::validate() : "
                       "null reset time given");
            QL_REQUIRE(resetTime >= 0.0,
                       "ForwardOptionArguments::validate() : "
                       "negative reset time given");
            QL_REQUIRE(residualTime >= resetTime,
                       "ForwardOptionArguments::validate() : "
                       "reset time greater than residual time");
        }

        //! Forward engine base class
        template<class ArgumentsType, class ResultsType>
        class ForwardEngine : public
            GenericEngine<ForwardOptionArguments<ArgumentsType>,
                          ResultsType> {
        public:
            ForwardEngine(const Handle<GenericEngine<ArgumentsType,
                ResultsType> >&);
        protected:
            Handle<GenericEngine<ArgumentsType, ResultsType> > originalEngine_;
            ArgumentsType* originalArguments_;
            const ResultsType* originalResults_;
        };

        template<class ArgumentsType, class ResultsType>
        ForwardEngine<ArgumentsType, ResultsType>::ForwardEngine(
            const Handle<GenericEngine<ArgumentsType, ResultsType> >&
            originalEngine)
        : originalEngine_(originalEngine) {
            QL_REQUIRE(!originalEngine_.isNull(),
                "ForwardEngine::ForwardEngine : "
                "null engine or wrong engine type");
            originalResults_ = dynamic_cast<const ResultsType*>(
                originalEngine_->results());
            originalArguments_ = dynamic_cast<ArgumentsType*>(
                originalEngine_->arguments());
        }

        //! Forward vanilla engine base class
        class ForwardVanillaEngine : public ForwardEngine<
            VanillaOptionArguments, VanillaOptionResults> {
        public:
            ForwardVanillaEngine(const Handle<VanillaEngine>& vanillaEngine);
        };

        inline ForwardVanillaEngine::ForwardVanillaEngine(const
            Handle<VanillaEngine>& vanillaEngine)
        : ForwardEngine<VanillaOptionArguments,
                        VanillaOptionResults    >(vanillaEngine) {
            QL_REQUIRE(!vanillaEngine.isNull(),
                "ForwardVanillaEngine::ForwardVanillaEngine : "
                "null engine or wrong engine type");
        }

        //! Forward vanilla engine class using analytic formulas
        class ForwardVanillaAnalyticEngine : public ForwardVanillaEngine {
        public:
            ForwardVanillaAnalyticEngine(const Handle<VanillaEngine>&
                vanillaEngine);
            void calculate() const;
        };

        inline ForwardVanillaAnalyticEngine::ForwardVanillaAnalyticEngine(const
            Handle<VanillaEngine>& vanillaEngine)
        : ForwardVanillaEngine(vanillaEngine) {
            QL_REQUIRE(!vanillaEngine.isNull(),
                "ForwardVanillaAnalyticEngine::ForwardVanillaAnalyticEngine : "
                "null engine or wrong engine type");
        }

        //! Forward Performance vanilla engine class using analytic formulas
        class ForwardPerformanceVanillaAnalyticEngine : public
            ForwardVanillaEngine {
        public:
            ForwardPerformanceVanillaAnalyticEngine(const
                Handle<VanillaEngine>& vanillaEngine);
            void calculate() const;
        };

       inline ForwardPerformanceVanillaAnalyticEngine::
           ForwardPerformanceVanillaAnalyticEngine(const Handle<VanillaEngine>&
           vanillaEngine)
        : ForwardVanillaEngine(vanillaEngine) {
            QL_REQUIRE(!vanillaEngine.isNull(),
                "ForwardPerformanceVanillaAnalyticEngine::"
                "ForwardPerformanceVanillaAnalyticEngine : "
                "null engine or wrong engine type");
        }

    }

}

#endif
