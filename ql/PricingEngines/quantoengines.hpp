
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
    \brief Quanto option engines

    \fullpath
    ql/PricingEngines/%quantoengines.hpp
*/

// $Id$

#ifndef quantlib_quanto_engines_h
#define quantlib_quanto_engines_h

#include <ql/PricingEngines/genericengine.hpp>
#include <ql/PricingEngines/vanillaengines.hpp>

namespace QuantLib {

    namespace PricingEngines {

        //! parameters for quanto option calculation
        template<class ArgumentsType>
        class QuantoOptionParameters : public ArgumentsType {
          public:
            QuantoOptionParameters() : foreignRiskFreeRate(Null<double>()),
                                       exchangeRateVolatility(Null<double>()),
                                       correlation(Null<double>()) {}
            void validate() const;
            Rate foreignRiskFreeRate;
            double exchangeRateVolatility;
            double correlation;
        };

        template<class ArgumentsType>
        void QuantoOptionParameters<ArgumentsType>::validate() const {
            ArgumentsType::validate();
            QL_REQUIRE(foreignRiskFreeRate != Null<double>(),
                       "QuantoOptionParameters::validate() : "
                       "null risk free rate given");
            QL_REQUIRE(exchangeRateVolatility != Null<double>(),
                       "QuantoOptionParameters::validate() : "
                       "null exchange rate volatility given");
            QL_REQUIRE(exchangeRateVolatility >= 0.0,
                       "QuantoOptionParameters::validate() : "
                       "negative exchange rate volatility given");
            QL_REQUIRE(correlation != Null<double>(),
                       "QuantoOptionParameters::validate() : "
                       "null correlation given");
        }

        //! %results from quanto option calculation
        template<class ResultsType>
        class QuantoOptionResults : public ResultsType {
          public:
            QuantoOptionResults() : qvega(Null<double>()),
                                    qrho(Null<double>()),
                                    qlambda(Null<double>()) {}
            double qvega;
            double qrho;
            double qlambda;
        };

        //! Quanto engine base class
        template<class ArgumentsType, class ResultsType>
        class QuantoEngine : public
            GenericEngine<QuantoOptionParameters<ArgumentsType>,
                          QuantoOptionResults<ResultsType> > {
        public:
            QuantoEngine(const Handle<GenericEngine<ArgumentsType,
                ResultsType> >&);
        protected:
            Handle<GenericEngine<ArgumentsType, ResultsType> > originalEngine_;
            ArgumentsType* originalParameters_;
            const ResultsType* originalResults_;
        };

        template<class ArgumentsType, class ResultsType>
        QuantoEngine<ArgumentsType, ResultsType>::QuantoEngine(
            const Handle<GenericEngine<ArgumentsType, ResultsType> >&
            originalEngine)
        : originalEngine_(originalEngine) {
            QL_REQUIRE(!originalEngine_.isNull(),
                "QuantoEngine::QuantoEngine : "
                "null engine or wrong engine type");
            originalResults_ = dynamic_cast<const ResultsType*>(
                originalEngine_->results());
            originalParameters_ = dynamic_cast<ArgumentsType*>(
                originalEngine_->parameters());
        }


        //! Quanto vanilla engine base class
        class QuantoVanillaEngine : public QuantoEngine<
                                        VanillaOptionParameters,
                                        VanillaOptionResults> {
        public:
            QuantoVanillaEngine(const Handle<VanillaEngine>& vanillaEngine);
        };

        inline QuantoVanillaEngine::QuantoVanillaEngine(const Handle<VanillaEngine>&
            vanillaEngine)
        : QuantoEngine<VanillaOptionParameters,
                        VanillaOptionResults>(vanillaEngine) {
            QL_REQUIRE(!vanillaEngine.isNull(),
                "QuantoVanillaEngine::QuantoVanillaEngine : "
                "null engine or wrong engine type");
        }


        //! Quanto vanilla engine base class
        class QuantoVanillaAnalyticEngine : public QuantoVanillaEngine {
        public:
            QuantoVanillaAnalyticEngine(const Handle<VanillaEngine>&
                vanillaEngine);
            void calculate() const;
        };

        inline QuantoVanillaAnalyticEngine::QuantoVanillaAnalyticEngine(const
            Handle<VanillaEngine>& vanillaEngine)
        : QuantoVanillaEngine(vanillaEngine) {
            QL_REQUIRE(!vanillaEngine.isNull(),
                "QuantoVanillaAnalyticEngine::QuantoVanillaAnalyticEngine : "
                "null engine or wrong engine type");
        }



    }
}

#endif

