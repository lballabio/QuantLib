
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

/*! \file quantoengines.hpp
    \brief Quanto option engines
*/

#ifndef quantlib_quanto_engines_h
#define quantlib_quanto_engines_h

#include <ql/Instruments/quantovanillaoption.hpp>
#include <ql/PricingEngines/genericengine.hpp>
#include <ql/TermStructures/quantotermstructure.hpp>

namespace QuantLib {

    namespace PricingEngines {

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

            originalEngine_->reset();
            // determine strike from payoff
            Handle<StrikedTypePayoff> payoff(arguments_.payoff);
            double strike = payoff->strike();

            originalArguments_->payoff = arguments_.payoff;
            originalArguments_->underlying    = arguments_.underlying;
            originalArguments_->dividendTS    = 
                RelinkableHandle<TermStructure>(
                    Handle<TermStructure>(
                    new TermStructures::QuantoTermStructure(
                        arguments_.dividendTS,
                        arguments_.riskFreeTS, 
                        arguments_.foreignRiskFreeTS,
                        arguments_.volTS, strike,
                        arguments_.exchRateVolTS, exchangeRateATMlevel,
                        arguments_.correlation)));
            originalArguments_->riskFreeTS    = arguments_.riskFreeTS;
            originalArguments_->volTS         = arguments_.volTS;
            originalArguments_->maturity      = arguments_.maturity;
            originalArguments_->stoppingTimes = arguments_.stoppingTimes;
            originalArguments_->exerciseType  = arguments_.exerciseType;

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
                arguments_.maturity, exchangeRateATMlevel);
            results_.vega = originalResults_->vega +
                arguments_.correlation * exchangeRateFlatVol *
                originalResults_->dividendRho;


            double volatility = arguments_.volTS->blackVol(
                arguments_.maturity, arguments_.underlying);
            results_.qvega = + arguments_.correlation
                * arguments_.volTS->blackVol(arguments_.maturity,
                arguments_.underlying) *
                originalResults_->dividendRho;
            results_.qrho = - originalResults_->dividendRho;
            results_.qlambda = exchangeRateFlatVol *
                volatility * originalResults_->dividendRho;
        }

    }

}


#endif
