/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Thema Consulting SA

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

/*! \file analyticdoublebarrierbinaryengine.hpp
    \brief analytic binary double barrier (one-touch double barrier) option engine
*/

#ifndef quantlib_binary_double_barrier_engine_hpp
#define quantlib_binary_double_barrier_engine_hpp

#include <ql/experimental/barrieroption/doublebarrieroption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Analytic pricing engine for double barrier binary options
    /*! This engine implements C.H.Hui series ("One-Touch Double Barrier 
        Binary Option Values", Applied Financial Economics 6/1996), as
        described in "The complete guide to option pricing formulas 2nd Ed", 
        E.G. Haug, McGraw-Hill, p.180

        The Knock In part of KI+KO and KO+KI options pays at hit, while the
        Double Knock In pays at end. 
        This engine thus requires European esercise for Double Knock options, 
        and American exercise for KIKO/KOKI.

        \ingroup barrierengines

        greeks are calculated by simple numeric derivation

        \test
        - the correctness of the returned value is tested by reproducing 
          results available in literature.
    */
    class AnalyticDoubleBarrierBinaryEngine : public DoubleBarrierOption::engine {
      public:
        explicit AnalyticDoubleBarrierBinaryEngine(ext::shared_ptr<GeneralizedBlackScholesProcess>);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

}


#endif


#ifndef id_a2feac068c6de58fc9f804fd5dfdc09a
#define id_a2feac068c6de58fc9f804fd5dfdc09a
inline bool test_a2feac068c6de58fc9f804fd5dfdc09a(int* i) { return i != 0; }
#endif
