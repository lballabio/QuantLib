
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005 StatPro Italia srl

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

/*! \file blackscholesprocess.hpp
    \brief Black-Scholes processes
*/

#ifndef quantlib_black_scholes_process_hpp
#define quantlib_black_scholes_process_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/yieldtermstructure.hpp>
#include <ql/voltermstructure.hpp>

namespace QuantLib {

    //! Black-Scholes stochastic process
    /*! This class describes the stochastic process governed by
        \f[
            dS(t, S)=(r(t) - q(t) - \frac{\sigma(t, S)^2}{2}) dt + \sigma dW_t.
        \f]
    */
    class BlackScholesProcess : public StochasticProcess {
      public:
        BlackScholesProcess(
            const Handle<Quote>& x0,
            const Handle<YieldTermStructure>& dividendTS,
            const Handle<YieldTermStructure>& riskFreeTS,
            const Handle<BlackVolTermStructure>& blackVolTS,
            const boost::shared_ptr<StochasticProcess::discretization>& d =
                    boost::shared_ptr<StochasticProcess::discretization>(
                                                    new EulerDiscretization));
        //! \name StochasticProcess interface
        //@{
        Real x0() const;
        /*! \todo revise extrapolation */
        Real drift(Time t, Real x) const ;
        /*! \todo revise extrapolation */
        Real diffusion(Time t, Real x) const;
        Real evolve(Real change, Real currentValue) const;
        //@}
        Time time(const Date&) const;
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Inspectors
        //@{
        const boost::shared_ptr<Quote>& stateVariable() const;
        const boost::shared_ptr<YieldTermStructure>& dividendYield() const;
        const boost::shared_ptr<YieldTermStructure>& riskFreeRate() const;
        const boost::shared_ptr<BlackVolTermStructure>&
                                                     blackVolatility() const;
        const boost::shared_ptr<LocalVolTermStructure>&
                                                     localVolatility() const;
        //@}
      private:
        Handle<Quote> x0_;
        Handle<YieldTermStructure> riskFreeRate_, dividendYield_;
        Handle<BlackVolTermStructure> blackVolatility_;
        mutable Handle<LocalVolTermStructure> localVolatility_;
        mutable bool updated_;
    };

}


#endif
