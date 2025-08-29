/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Thema Consulting SA

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

/*! \file analyticbinarybarrierengine.hpp
    \brief analytic binary barrier (cash/asset or nothing plus in-the-money check) option engine
*/

#ifndef quantlib_binary_barrier_engine_hpp
#define quantlib_binary_barrier_engine_hpp

#include <ql/instruments/barrieroption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Analytic pricing engine for American binary barriers options
    /*! The formulas are taken from "The complete guide to option pricing formulas 2nd Ed",
         E.G. Haug, McGraw-Hill, p.176 and following. 

        \ingroup barrierengines

        \test
        - the correctness of the returned value in case of
          cash-or-nothing at-expiry binary payoff is tested by
          reproducing results available in literature.
        - the correctness of the returned value in case of
          asset-or-nothing at-expiry binary payoff is tested by
          reproducing results available in literature.
    */
    class AnalyticBinaryBarrierEngine : public BarrierOption::engine {
      public:
        AnalyticBinaryBarrierEngine(ext::shared_ptr<GeneralizedBlackScholesProcess>);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

}


#endif
