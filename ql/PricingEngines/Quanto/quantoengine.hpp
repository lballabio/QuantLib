
/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano

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

/*! \file quantoengine.hpp
    \brief Quanto option engine
*/

#ifndef quantlib_quanto_engine_hpp
#define quantlib_quanto_engine_hpp

#include <ql/Instruments/quantovanillaoption.hpp>
#include <ql/TermStructures/quantotermstructure.hpp>

namespace QuantLib {

    //! Quanto engine base class
    template<class ArgumentsType, class ResultsType>
    class QuantoEngine
        : public GenericEngine<QuantoOptionArguments<ArgumentsType>,
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


    // template definitions

    template<class ArgumentsType, class ResultsType>
    QuantoEngine<ArgumentsType, ResultsType>::QuantoEngine(
        const Handle<GenericEngine<ArgumentsType, ResultsType> >&
            originalEngine)
    : originalEngine_(originalEngine) {
        QL_REQUIRE(!IsNull(originalEngine_),
                   "QuantoEngine::QuantoEngine : null engine");
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
        Handle<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "QuantoEngine: non-striked payoff given");
        double strike = payoff->strike();

        originalArguments_->payoff = arguments_.payoff;

//        originalArguments_->underlying    = arguments_.blackScholesProcess->stateVariable->value();
//        originalArguments_->riskFreeTS    = arguments_.riskFreeTS;
//        originalArguments_->volTS         = arguments_.blackScholesProcess->volTS;
        originalArguments_->blackScholesProcess=arguments_.blackScholesProcess;
        // dividendTS needs modification
        originalArguments_->blackScholesProcess->dividendTS =
            RelinkableHandle<TermStructure>(Handle<TermStructure>(new
                QuantoTermStructure(
                    arguments_.blackScholesProcess->dividendTS,
                    arguments_.blackScholesProcess->riskFreeTS,
                    arguments_.foreignRiskFreeTS,
                    arguments_.blackScholesProcess->volTS, strike,
                    arguments_.exchRateVolTS, exchangeRateATMlevel,
                    arguments_.correlation)));

        originalArguments_->exercise      = arguments_.exercise;

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
            arguments_.exercise->lastDate(),
            exchangeRateATMlevel);
        results_.vega = originalResults_->vega +
            arguments_.correlation * exchangeRateFlatVol *
            originalResults_->dividendRho;


        double volatility =
            arguments_.blackScholesProcess->volTS->blackVol(
                arguments_.exercise->lastDate(),
                arguments_.blackScholesProcess->stateVariable->value());
        results_.qvega = + arguments_.correlation
            * arguments_.blackScholesProcess->volTS
                ->blackVol(
                    arguments_.exercise->lastDate(),
                    arguments_.blackScholesProcess->stateVariable->value()) *
            originalResults_->dividendRho;
        results_.qrho = - originalResults_->dividendRho;
        results_.qlambda = exchangeRateFlatVol *
            volatility * originalResults_->dividendRho;
    }

}


#endif
