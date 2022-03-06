/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005 StatPro Italia srl

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

/*! \file merton76process.hpp
    \brief Merton-76 process
*/

#ifndef quantlib_merton_76_process_hpp
#define quantlib_merton_76_process_hpp

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/eulerdiscretization.hpp>

namespace QuantLib {

    //! Merton-76 jump-diffusion process
    /*! \ingroup processes */
    class Merton76Process : public StochasticProcess1D {
      public:
        Merton76Process(const Handle<Quote>& stateVariable,
                        const Handle<YieldTermStructure>& dividendTS,
                        const Handle<YieldTermStructure>& riskFreeTS,
                        const Handle<BlackVolTermStructure>& blackVolTS,
                        Handle<Quote> jumpInt,
                        Handle<Quote> logJMean,
                        Handle<Quote> logJVol,
                        const ext::shared_ptr<discretization>& d =
                            ext::shared_ptr<discretization>(new EulerDiscretization));
        //! \name StochasticProcess1D interface
        //@{
        Real x0() const override;
        Real drift(Time, Real) const override { QL_FAIL("not implemented"); }
        Real diffusion(Time, Real) const override { QL_FAIL("not implemented"); }
        Real apply(Real, Real) const override { QL_FAIL("not implemented"); }
        //@}
        Time time(const Date&) const override;
        //! \name Inspectors
        //@{
        const Handle<Quote>& stateVariable() const;
        const Handle<YieldTermStructure>& dividendYield() const;
        const Handle<YieldTermStructure>& riskFreeRate() const;
        const Handle<BlackVolTermStructure>& blackVolatility() const;
        const Handle<Quote>& jumpIntensity() const;
        const Handle<Quote>& logMeanJump() const;
        const Handle<Quote>& logJumpVolatility() const;
        //@}
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> blackProcess_;
        Handle<Quote> jumpIntensity_, logMeanJump_, logJumpVolatility_;
    };

}


#endif


#ifndef id_c693e2c86021ad629c73ad25bf1bb9c7
#define id_c693e2c86021ad629c73ad25bf1bb9c7
inline bool test_c693e2c86021ad629c73ad25bf1bb9c7(int* i) { return i != 0; }
#endif
