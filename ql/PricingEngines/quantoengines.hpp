
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
/*! \file quantoengines.hpp
    \brief Quanto option engines

    \fullpath
    ql/PricingEngines/%quantoengines.hpp
*/

// $Id$

#ifndef quantlib_quanto_engines_h
#define quantlib_quanto_engines_h

#include <ql/PricingEngines/vanillaengines.hpp>
#include <ql/TermStructures/quantotermstructure.hpp>

namespace QuantLib {

    namespace PricingEngines {

        //! arguments for quanto option calculation
        template<class ArgumentsType>
        class QuantoOptionArguments : public ArgumentsType {
          public:
            QuantoOptionArguments() : correlation(Null<double>()) {}
            void validate() const;
            double correlation;
            RelinkableHandle<TermStructure> foreignRiskFreeTS;
            RelinkableHandle<BlackVolTermStructure> exchRateVolTS;
        };

        template<class ArgumentsType>
        void QuantoOptionArguments<ArgumentsType>::validate() const {
            ArgumentsType::validate();
            QL_REQUIRE(!foreignRiskFreeTS.isNull(),
                       "QuantoOptionArguments::validate() : "
                       "null foreign risk free term structure");
            QL_REQUIRE(!exchRateVolTS.isNull(),
                       "QuantoOptionArguments::validate() : "
                       "null exchange rate vol term structure");
            QL_REQUIRE(correlation != Null<double>(),
                       "QuantoOptionArguments::validate() : "
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
            GenericEngine<QuantoOptionArguments<ArgumentsType>,
                          QuantoOptionResults<ResultsType> > {
        public:
            QuantoEngine(const Handle<GenericEngine<ArgumentsType,
                ResultsType> >&);
            void calculate() const;
        protected:
            Handle<GenericEngine<ArgumentsType, ResultsType> > originalEngine_;
            ArgumentsType* originalArguments_;
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
            originalArguments_ = dynamic_cast<ArgumentsType*>(
                originalEngine_->arguments());
        }

        template<class ArgumentsType, class ResultsType>
        void QuantoEngine<ArgumentsType, ResultsType>::calculate() const {

            // ATM exchangeRate level needed here
            double exchangeRateATMlevel = 1.0;

            originalArguments_->type = arguments_.type;
            originalArguments_->underlying = arguments_.underlying;
            originalArguments_->strike = arguments_.strike;
            originalArguments_->dividendTS = 
                RelinkableHandle<TermStructure>(
                    Handle<TermStructure>(
                    new TermStructures::QuantoTermStructure(
                        arguments_.dividendTS,
                        arguments_.riskFreeTS, 
                        arguments_.foreignRiskFreeTS,
                        arguments_.volTS, arguments_.strike,
                        arguments_.exchRateVolTS, exchangeRateATMlevel,
                        arguments_.correlation)));
            originalArguments_->riskFreeTS = arguments_.riskFreeTS;
            originalArguments_->volTS = arguments_.volTS;
            originalArguments_->exercise = arguments_.exercise;

            originalArguments_->validate();
            originalEngine_->calculate();

            results_.value = originalResults_->value;
            results_.delta = originalResults_->delta;
            results_.gamma = originalResults_->gamma;
            results_.theta = originalResults_->theta;
            results_.rho = originalResults_->rho +
                originalResults_->dividendRho;
            results_.dividendRho = originalResults_->dividendRho;
            double exchangeRateFlatVol = arguments_.exchRateVolTS->blackVol(
                arguments_.exercise.lastDate(), exchangeRateATMlevel);
            results_.vega = originalResults_->vega +
                arguments_.correlation * exchangeRateFlatVol *
                originalResults_->dividendRho;


            double volatility = arguments_.volTS->blackVol(
                arguments_.exercise.lastDate(), arguments_.underlying);
            results_.qvega = + arguments_.correlation
                * arguments_.volTS->blackVol(arguments_.exercise.lastDate(),
                arguments_.underlying) *
                originalResults_->dividendRho;
            results_.qrho = - originalResults_->dividendRho;
            results_.qlambda = exchangeRateFlatVol *
                volatility * originalResults_->dividendRho;
        }

    }

}

#endif

