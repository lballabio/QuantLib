
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
                       "null risk free rate given");
            QL_REQUIRE(exchangeRateVolatility != Null<double>(),
                       "null exchange rate volatility given");
            QL_REQUIRE(exchangeRateVolatility >= 0.0,
                       "negative exchange rate volatility given");
            QL_REQUIRE(correlation != Null<double>(),
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
            void calculate() const;
        protected:
            Handle<GenericEngine<ArgumentsType, ResultsType> > originalEngine_;
            ArgumentsType* originalArgs_;
            const ResultsType* originalResults_;
        };

        template<class ArgumentsType, class ResultsType>
        QuantoEngine<ArgumentsType, ResultsType>::QuantoEngine(
            const Handle<GenericEngine<ArgumentsType, ResultsType> >& originalEngine)
        : originalEngine_(originalEngine) {
            QL_REQUIRE(!originalEngine_.isNull(), "null engine or wrong engine type");
            originalResults_ = dynamic_cast<const ResultsType*>(originalEngine_->results());
            originalArgs_ = dynamic_cast<ArgumentsType*>(originalEngine_->arguments());
        }


        //! Quanto vanilla engine base class
        class QuantoVanillaEngine : public
            QuantoEngine<VanillaOptionParameters,
                         VanillaOptionResults> {
        };

        //! Quanto vanilla engine base class
        class QuantoVanillaAnalyticEngine : public QuantoVanillaEngine {
        public:
            void calculate() const;
        };

    }
}

#endif

