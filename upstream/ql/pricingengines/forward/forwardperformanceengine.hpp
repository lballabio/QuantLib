/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file forwardperformanceengine.hpp
    \brief Forward (strike-resetting) performance vanilla-option engine
*/

#ifndef quantlib_forward_performance_engine_hpp
#define quantlib_forward_performance_engine_hpp

#include <ql/pricingengines/forward/forwardengine.hpp>

namespace QuantLib {

    //! %Forward performance engine for vanilla options
    /*! \ingroup forwardengines

        \test
        - the correctness of the returned value is tested by
          reproducing results available in literature.
        - the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
    */
    template <class Engine>
    class ForwardPerformanceVanillaEngine
        : public ForwardVanillaEngine<Engine> {
      public:
        ForwardPerformanceVanillaEngine(
                    const ext::shared_ptr<GeneralizedBlackScholesProcess>&);
        void calculate() const override;

      protected:
        void getOriginalResults() const;
    };


    // template definitions

    template <class Engine>
    ForwardPerformanceVanillaEngine<Engine>::ForwardPerformanceVanillaEngine(
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : ForwardVanillaEngine<Engine>(process) {}

    template <class Engine>
    void ForwardPerformanceVanillaEngine<Engine>::calculate() const {
        this->setup();
        this->originalEngine_->calculate();
        getOriginalResults();
    }

    template <class Engine>
    void ForwardPerformanceVanillaEngine<Engine>::getOriginalResults() const {

        DayCounter rfdc = this->process_->riskFreeRate()->dayCounter();
        Time resetTime = rfdc.yearFraction(
            this->process_->riskFreeRate()->referenceDate(),
            this->arguments_.resetDate);
        DiscountFactor discR = this->process_->riskFreeRate()->discount(
                                                  this->arguments_.resetDate);
        // it's a performance option
        discR /= this->process_->stateVariable()->value();

        Real temp = this->originalResults_->value;
        this->results_.value = discR * temp;
        this->results_.delta = 0.0;
        this->results_.gamma = 0.0;
        this->results_.theta = this->process_->riskFreeRate()->
            zeroRate(this->arguments_.resetDate, rfdc, Continuous, NoFrequency)
            * this->results_.value;
        this->results_.vega = discR * this->originalResults_->vega;
        this->results_.rho = - resetTime * this->results_.value +
            discR * this->originalResults_->rho;
        this->results_.dividendRho = discR * this->originalResults_->dividendRho;
    }

}


#endif
