/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Yue Tian

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

/*! \file wulinyongdoublebarrierengine.hpp
    \brief Wulin Suo, Yong Wang double-barrier option engine
*/

#ifndef wulin_yong_double_barrier_engine_hpp
#define wulin_yong_double_barrier_engine_hpp

#include <ql/experimental/barrieroption/doublebarrieroption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    //! Pricing engine for barrier options using analytical formulae
    /*! The formulas are taken from "Barrier Option Pricing",
         Wulin Suo, Yong Wang.

        \ingroup barrierengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    class WulinYongDoubleBarrierEngine : public DoubleBarrierOption::engine {
      public:
        WulinYongDoubleBarrierEngine(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
            int series = 5);
        void calculate() const;
      private:
        boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
        const int series_;
        CumulativeNormalDistribution f_;
        // helper methods
        Real underlying() const;
        Real strike() const;
        Time residualTime() const;
        Volatility volatility() const;
        Real barrier() const;
        Real rebate() const;
        Real stdDeviation() const;
        Rate riskFreeRate() const;
        DiscountFactor riskFreeDiscount() const;
        Rate dividendYield() const;
        DiscountFactor dividendDiscount() const;
        Real D(Real X, Real lambda, Real sigma, Real T) const;

    };

}


#endif
