
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

#include <ql/Volatilities/impliedvoltermstructure.hpp>
#include <ql/TermStructures/impliedtermstructure.hpp>

namespace QuantLib {

    //! %Arguments for forward (strike-resetting) option calculation
    template <class ArgumentsType>
    class ForwardOptionArguments : public ArgumentsType {
      public:
        ForwardOptionArguments() : moneyness(Null<Real>()),
                                   resetDate(Null<Date>()) {}
        void validate() const;
        Real moneyness;
        Date resetDate;
    };


    //! Forward engine base class
    /*! \ingroup forwardengines

        \test a) the correctness of the returned value is tested by
                 reproducing results available in literature.

        \test b) the correctness of the returned greeks is tested by
                 reproducing numerical derivatives.
    */
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

    template <class ArgumentsType>
    void ForwardOptionArguments<ArgumentsType>::validate() const {
        ArgumentsType::validate();

        QL_REQUIRE(moneyness != Null<Real>(), "null moneyness given");
        QL_REQUIRE(moneyness > 0.0, "negative or zero moneyness given");

        QL_REQUIRE(resetDate != Null<Date>(), "null reset date given");
        QL_REQUIRE(resetDate >= this->blackScholesProcess->riskFreeRate()
                                                            ->referenceDate(),
                   "reset date later than settlement");
        QL_REQUIRE(this->exercise->lastDate() > resetDate,
                   "reset date later or equal to maturity");
    }


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
            boost::dynamic_pointer_cast<StrikedTypePayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(argumentsPayoff, "wrong payoff given");

        const boost::shared_ptr<BlackScholesProcess>& process =
            this->arguments_.blackScholesProcess;

        boost::shared_ptr<StrikedTypePayoff> payoff(
                   new PlainVanillaPayoff(argumentsPayoff->optionType(),
                                          this->arguments_.moneyness *
                                          process->stateVariable()->value()));
        originalArguments_->payoff = payoff;

        // maybe the forward value is "better", in some fashion
        // the right level is needed in order to interpolate
        // the vol
        Handle<Quote> spot(process->stateVariable());
        Handle<YieldTermStructure> dividendYield(
            boost::shared_ptr<YieldTermStructure>(
               new ImpliedTermStructure(
                         Handle<YieldTermStructure>(process->dividendYield()),
                         this->arguments_.resetDate)));
        Handle<YieldTermStructure> riskFreeRate(
            boost::shared_ptr<YieldTermStructure>(
               new ImpliedTermStructure(
                          Handle<YieldTermStructure>(process->riskFreeRate()),
                          this->arguments_.resetDate)));
        // The following approach is ok if the vol is at most
        // time dependant. It is plain wrong if it is asset dependant.
        // In the latter case the right solution would be stochastic
        // volatility or at least local volatility (which unfortunately
        // implies an unrealistic time-decreasing smile)
        Handle<BlackVolTermStructure> blackVolatility(
            boost::shared_ptr<BlackVolTermStructure>(
                new ImpliedVolTermStructure(
                    Handle<BlackVolTermStructure>(process->blackVolatility()),
                    this->arguments_.resetDate)));

        originalArguments_->blackScholesProcess =
            boost::shared_ptr<BlackScholesProcess>(
                      new BlackScholesProcess(spot, dividendYield,
                                              riskFreeRate, blackVolatility));

        originalArguments_->exercise = this->arguments_.exercise;

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

        const boost::shared_ptr<BlackScholesProcess>& process =
            this->arguments_.blackScholesProcess;

        Time resetTime = process->riskFreeRate()->dayCounter().yearFraction(
                                    process->riskFreeRate()->referenceDate(),
                                    this->arguments_.resetDate);
        DiscountFactor discQ = process->dividendYield()->discount(
                                    this->arguments_.resetDate);

        this->results_.value = discQ * originalResults_->value;
        // I need the strike derivative here ...
        this->results_.delta = discQ * (originalResults_->delta +
            this->arguments_.moneyness * originalResults_->strikeSensitivity);
        this->results_.gamma = 0.0;
        this->results_.theta = process->dividendYield()->zeroYield(
            this->arguments_.resetDate) * this->results_.value;
        this->results_.vega  = discQ * originalResults_->vega;
        this->results_.rho   = discQ *  originalResults_->rho;
        this->results_.dividendRho = - resetTime * this->results_.value
            + discQ * originalResults_->dividendRho;
    }

}


#endif
