/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003, 2004, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file analyticdigitalamericanengine.hpp
    \brief analytic digital American option engine
*/

#ifndef quantlib_analytic_digital_american_engine_hpp
#define quantlib_analytic_digital_american_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Analytic pricing engine for American vanilla options with digital payoff
    /*! \ingroup vanillaengines

        \todo add more greeks (as of now only delta and rho available)

        \test
        - the correctness of the returned value in case of
          cash-or-nothing at-hit digital payoff is tested by
          reproducing results available in literature.
        - the correctness of the returned value in case of
          asset-or-nothing at-hit digital payoff is tested by
          reproducing results available in literature.
        - the correctness of the returned value in case of
          cash-or-nothing at-expiry digital payoff is tested by
          reproducing results available in literature.
        - the correctness of the returned value in case of
          asset-or-nothing at-expiry digital payoff is tested by
          reproducing results available in literature.
        - the correctness of the returned greeks in case of
          cash-or-nothing at-hit digital payoff is tested by
          reproducing numerical derivatives.
    */
    class AnalyticDigitalAmericanEngine : public VanillaOption::engine {
      public:
        AnalyticDigitalAmericanEngine(std::shared_ptr<GeneralizedBlackScholesProcess>);
        void calculate() const override;
        virtual bool knock_in() const {
           return true;
        }
      private:
        std::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

    //! Analytic pricing engine for American Knock-out options with digital payoff
    /*! \ingroup vanillaengines

        \todo add more greeks (as of now only delta and rho available)

        \test
        - the correctness of the returned value in case of
          cash-or-nothing at-hit digital payoff is tested by
          reproducing results available in literature.
        - the correctness of the returned value in case of
          asset-or-nothing at-hit digital payoff is tested by
          reproducing results available in literature.
        - the correctness of the returned value in case of
          cash-or-nothing at-expiry digital payoff is tested by
          reproducing results available in literature.
        - the correctness of the returned value in case of
          asset-or-nothing at-expiry digital payoff is tested by
          reproducing results available in literature.
        - the correctness of the returned greeks in case of
          cash-or-nothing at-hit digital payoff is tested by
          reproducing numerical derivatives.
    */
    class AnalyticDigitalAmericanKOEngine : 
                              public AnalyticDigitalAmericanEngine {
      public:
        AnalyticDigitalAmericanKOEngine(
                    const std::shared_ptr<GeneralizedBlackScholesProcess> 
                                 &engine):
        AnalyticDigitalAmericanEngine(engine) {}
        bool knock_in() const override { return false; }
    };

}


#endif
