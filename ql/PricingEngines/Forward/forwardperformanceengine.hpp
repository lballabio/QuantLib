
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

/*! \file forwardperformanceengine.hpp
    \brief Forward (strike-resetting) performance option engines
*/

#ifndef quantlib_forward_performance_engine_hpp
#define quantlib_forward_performance_engine_hpp

#include <ql/PricingEngines/Forward/forwardengine.hpp>

namespace QuantLib {

    //! Forward performance engine
    /*! \ingroup forwardengines

        \test a) the correctness of the returned value is tested by
                 reproducing results available in literature.

        \test b) the correctness of the returned greeks is tested by
                 reproducing numerical derivatives.
    */
    template<class ArgumentsType, class ResultsType>
    class ForwardPerformanceEngine
        : public ForwardEngine<ArgumentsType, ResultsType> {
      public:
        ForwardPerformanceEngine(
          const boost::shared_ptr<GenericEngine<ArgumentsType,ResultsType> >&);
        void calculate() const;
        void getOriginalResults() const;
    };


    // template definitions

    template<class ArgumentsType, class ResultsType>
    ForwardPerformanceEngine<ArgumentsType, ResultsType>::
    ForwardPerformanceEngine(
        const boost::shared_ptr<GenericEngine<ArgumentsType, ResultsType> >&
            originalEngine)
    : ForwardEngine<ArgumentsType, ResultsType>(originalEngine) {}

    template<class ArgumentsType, class ResultsType>
    void ForwardPerformanceEngine<ArgumentsType, ResultsType>::calculate()
                                                                       const {
        this->setOriginalArguments();
        this->originalEngine_->calculate();
        getOriginalResults();
    }

    template<class ArgumentsType, class ResultsType>
    void ForwardPerformanceEngine<ArgumentsType, ResultsType>::
    getOriginalResults() const {

        const boost::shared_ptr<BlackScholesProcess>& process =
            this->arguments_.blackScholesProcess;

        Time resetTime = process->riskFreeRate()->dayCounter().yearFraction(
                                    process->riskFreeRate()->referenceDate(),
                                    this->arguments_.resetDate);
        DiscountFactor discR = process->riskFreeRate()->discount(
                                    this->arguments_.resetDate);
        // it's a performance option
        discR /= process->stateVariable()->value();

        Real temp = this->originalResults_->value;
        this->results_.value = discR * temp;
        this->results_.delta = 0.0;
        this->results_.gamma = 0.0;
        this->results_.theta = process->riskFreeRate()->zeroYield(
            this->arguments_.resetDate) * this->results_.value;
        this->results_.vega = discR * this->originalResults_->vega;
        this->results_.rho = - resetTime * this->results_.value +
            discR * this->originalResults_->rho;
        this->results_.dividendRho = discR * this->originalResults_->dividendRho;
    }

}


#endif
