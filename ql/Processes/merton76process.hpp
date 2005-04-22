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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file merton76process.hpp
    \brief Merton-76 process
*/

#ifndef quantlib_merton_76_process_hpp
#define quantlib_merton_76_process_hpp

#include <ql/Processes/blackscholesprocess.hpp>
#include <ql/Processes/eulerdiscretization.hpp>

namespace QuantLib {

    //! Merton-76 jump-diffusion process
    class Merton76Process : public StochasticProcess1D {
      public:
        Merton76Process(
            const Handle<Quote>& stateVariable,
            const Handle<YieldTermStructure>& dividendTS,
            const Handle<YieldTermStructure>& riskFreeTS,
            const Handle<BlackVolTermStructure>& blackVolTS,
            const Handle<Quote>& jumpInt,
            const Handle<Quote>& logJMean,
            const Handle<Quote>& logJVol,
            const boost::shared_ptr<discretization>& d =
                  boost::shared_ptr<discretization>(new EulerDiscretization));
        //! \name StochasticProcess1D interface
        //@{
        Real x0() const;
        Real drift(Time, Real) const { QL_FAIL("not implemented"); }
        Real diffusion(Time, Real) const { QL_FAIL("not implemented"); }
        Real evolve(Real change, Real currentValue) const {
            QL_FAIL("not implemented");
        }
        //@}
        Time time(const Date&) const;
        //! \name Inspectors
        //@{
        const boost::shared_ptr<Quote>& stateVariable() const;
        const boost::shared_ptr<YieldTermStructure>& dividendYield() const;
        const boost::shared_ptr<YieldTermStructure>& riskFreeRate() const;
        const boost::shared_ptr<BlackVolTermStructure>&
                                                     blackVolatility() const;
        const boost::shared_ptr<Quote>& jumpIntensity() const;
        const boost::shared_ptr<Quote>& logMeanJump() const;
        const boost::shared_ptr<Quote>& logJumpVolatility() const;
        //@}
      private:
        boost::shared_ptr<BlackScholesProcess> blackProcess_;
        Handle<Quote> jumpIntensity_, logMeanJump_, logJumpVolatility_;
    };

}


#endif
