
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
    /*! \ingroup forwardengines */
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

        setOriginalArguments();
        originalEngine_->calculate();
        getOriginalResults();
    }

    template<class ArgumentsType, class ResultsType>
    void ForwardPerformanceEngine<ArgumentsType, ResultsType>::
    getOriginalResults() const {

        const boost::shared_ptr<BlackScholesProcess>& process =
            arguments_.blackScholesProcess;

        Time resetTime = process->riskFreeRate()->dayCounter().yearFraction(
                                    process->riskFreeRate()->referenceDate(), 
                                    arguments_.resetDate);
        DiscountFactor discR = 
            process->riskFreeRate()->discount(arguments_.resetDate);
        // it's a performance option
        discR /= process->stateVariable()->value();

        Real temp = originalResults_->value;
        results_.value = discR * temp;
        results_.delta = 0.0;
        results_.gamma = 0.0;
        results_.theta = process->riskFreeRate()->zeroYield(
                                                        arguments_.resetDate) 
            * results_.value;
        results_.vega = discR * originalResults_->vega;
        results_.rho = - resetTime * results_.value +
            discR * originalResults_->rho;
        results_.dividendRho = discR * originalResults_->dividendRho;
    }

}


#endif
