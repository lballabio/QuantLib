
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

/*! \file forwardengine.hpp
    \brief Forward (strike-resetting) option engine
*/

#ifndef quantlib_forward_engine_hpp
#define quantlib_forward_engine_hpp

#include <ql/Instruments/forwardvanillaoption.hpp>
#include <ql/Volatilities/impliedvoltermstructure.hpp>
#include <ql/TermStructures/impliedtermstructure.hpp>

namespace QuantLib {

    //! Forward engine base class
    template<class ArgumentsType, class ResultsType>
    class ForwardEngine
        : public GenericEngine<ForwardOptionArguments<ArgumentsType>,
                               ResultsType> {
      public:
        ForwardEngine(const boost::shared_ptr<GenericEngine<ArgumentsType,
                                                            ResultsType> >&);
        void setOriginalArguments() const;
        void calculate() const;
        void getOriginalResults() const;
      protected:
        boost::shared_ptr<GenericEngine<ArgumentsType, 
                                        ResultsType> > originalEngine_;
        ArgumentsType* originalArguments_;
        const ResultsType* originalResults_;
    };


    // template definitions

    template<class ArgumentsType, class ResultsType>
    ForwardEngine<ArgumentsType, ResultsType>::ForwardEngine(
        const boost::shared_ptr<GenericEngine<ArgumentsType, ResultsType> >&
            originalEngine)
    : originalEngine_(originalEngine) {
        QL_REQUIRE(originalEngine_, "null engine");
        originalResults_ = dynamic_cast<const ResultsType*>(
            originalEngine_->results());
        originalArguments_ = dynamic_cast<ArgumentsType*>(
            originalEngine_->arguments());
    }


    template<class ArgumentsType, class ResultsType>
    void ForwardEngine<ArgumentsType, ResultsType>::setOriginalArguments()
                                                                        const {

        boost::shared_ptr<StrikedTypePayoff> argumentsPayoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(argumentsPayoff, "wrong payoff given");

        argumentsPayoff->setStrike(arguments_.moneyness*
            arguments_.blackScholesProcess->stateVariable->value());
        originalArguments_->payoff = argumentsPayoff;


        // maybe the forward value is "better", in some fashion
        // the right level is needed in order to interpolate
        // the vol
        originalArguments_->blackScholesProcess->stateVariable = 
            arguments_.blackScholesProcess->stateVariable;
        originalArguments_->blackScholesProcess->dividendTS = 
            RelinkableHandle<TermStructure>(
                boost::shared_ptr<TermStructure>(
                    new ImpliedTermStructure(
                        arguments_.blackScholesProcess->dividendTS, 
                        arguments_.resetDate,
                        arguments_.resetDate)));
        originalArguments_->blackScholesProcess->riskFreeTS = 
            RelinkableHandle<TermStructure>(
                boost::shared_ptr<TermStructure>(
                    new ImpliedTermStructure(
                        arguments_.blackScholesProcess->riskFreeTS, 
                        arguments_.resetDate,
                        arguments_.resetDate)));

        // The following approach is ok if the vol is at most
        // time dependant. It is plain wrong if it is asset dependant.
        // In the latter case the right solution would be stochastic
        // volatility or at least local volatility (which unfortunately
        // implies an unrealistic time-decreasing smile)
        originalArguments_->blackScholesProcess->volTS =
            RelinkableHandle<BlackVolTermStructure>(
                boost::shared_ptr<BlackVolTermStructure>(
                    new ImpliedVolTermStructure(
                        arguments_.blackScholesProcess->volTS,
                        arguments_.resetDate)));

        originalArguments_->exercise = arguments_.exercise;

        originalArguments_->validate();
    }

    template<class ArgumentsType, class ResultsType>
    void ForwardEngine<ArgumentsType, ResultsType>::calculate() const {
        originalEngine_->reset();
        setOriginalArguments();
        originalEngine_->calculate();
        getOriginalResults();
    }

    template<class ArgumentsType, class ResultsType>
    void ForwardEngine<ArgumentsType, ResultsType>::getOriginalResults()
                                                                      const {

        Time resetTime = arguments_.blackScholesProcess->riskFreeTS
            ->dayCounter().yearFraction(
                arguments_.blackScholesProcess->riskFreeTS->referenceDate(), 
                arguments_.resetDate);
        double discQ = arguments_.blackScholesProcess->dividendTS
            ->discount(arguments_.resetDate);

        results_.value = discQ * originalResults_->value;
        // I need the strike derivative here ...
        results_.delta = discQ * (originalResults_->delta +
                  arguments_.moneyness * originalResults_->strikeSensitivity);
        results_.gamma = 0.0;
        results_.theta = arguments_.blackScholesProcess->dividendTS->zeroYield(
                                       arguments_.resetDate) * results_.value;
        results_.vega  = discQ * originalResults_->vega;
        results_.rho   = discQ *  originalResults_->rho;
        results_.dividendRho = - resetTime * results_.value
            + discQ * originalResults_->dividendRho;

    }

}


#endif
