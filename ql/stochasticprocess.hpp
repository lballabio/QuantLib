
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 StatPro Italia srl

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

/*! \file stochasticprocess.hpp
    \brief stochastic processes
*/

#ifndef quantlib_stochastic_process_hpp
#define quantlib_stochastic_process_hpp

#include <ql/termstructure.hpp>
#include <ql/voltermstructure.hpp>

namespace QuantLib {

    //! Stochastic process class
    /*! This class describes a stochastic process governed by
        \f[
            dx_t = \mu(t, x_t)dt + \sigma(t, x_t)dW_t.
        \f]
    */
    class StochasticProcess {
      public:
        virtual ~StochasticProcess() {}
        //! returns the initial value of the state variable
        virtual double x0() const = 0;
        //! returns the drift part of the equation, i.e. \f$ \mu(t, x_t) \f$
        virtual double drift(Time t, double x) const = 0;
        /*! \brief returns the diffusion part of the equation, i.e.
            \f$ \sigma(t, x_t) \f$
        */
        virtual double diffusion(Time t, double x) const = 0;
        //! returns the expectation of the process after a time interval
        /*! returns \f$ E(x_{t_0 + \Delta t} | x_{t_0} = x_0) \f$.
            By default, it returns the Euler approximation defined by
            \f$ x_0 + \mu(t_0, x_0) \Delta t \f$.
        */
        virtual double expectation(Time t0, double x0, Time dt) const;
        //! returns the variance of the process after a time interval
        /*! returns \f$ Var(x_{t_0 + \Delta t} | x_{t_0} = x_0) \f$.
            By default, it returns the Euler approximation defined by
            \f$ \sigma(t_0, x_0)^2 \Delta t \f$.
        */
        virtual double variance(Time t0, double x0, Time dt) const;
    };


    //! Black-Scholes stochastic process
    /*! This class describes the stochastic process governed by
        \f[
            dS(t, S)=(r(t) - q(t) - \frac{\sigma(t, S)^2}{2}) dt + \sigma dW_t.
        \f]
    */
    class BlackScholesProcess : public StochasticProcess,
                                public Observer, public Observable {
      public:
        BlackScholesProcess(
            const RelinkableHandle<Quote>& x0,
            const RelinkableHandle<TermStructure>& dividendTS,
            const RelinkableHandle<TermStructure>& riskFreeTS,
            const RelinkableHandle<BlackVolTermStructure>& blackVolTS);
        //! \name StochasticProcess interface
        //@{
        double x0() const;
        /*! \todo revise extrapolation */
        double drift(Time t, double x) const ;
        /*! \todo revise extrapolation */
        double diffusion(Time t, double x) const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Inspectors
        //@{
        const boost::shared_ptr<Quote>& stateVariable() const;
        const boost::shared_ptr<TermStructure>& dividendYield() const;
        const boost::shared_ptr<TermStructure>& riskFreeRate() const;
        const boost::shared_ptr<BlackVolTermStructure>& 
                                                     blackVolatility() const;
        const boost::shared_ptr<LocalVolTermStructure>& 
                                                     localVolatility() const;
        //@}
      private:
        RelinkableHandle<Quote> x0_;
        RelinkableHandle<TermStructure> riskFreeRate_, dividendYield_;
        RelinkableHandle<BlackVolTermStructure> blackVolatility_;
        mutable RelinkableHandle<LocalVolTermStructure> localVolatility_;
        mutable bool updated_;
    };

#ifndef QL_DISABLE_DEPRECATED
    //! \deprecated Use BlackScholesProcess instead
    typedef BlackScholesProcess BlackScholesStochasticProcess;
#endif

    //! Merton-76 jump-diffusion process
    class Merton76Process : public BlackScholesProcess {
      public:
        Merton76Process(const RelinkableHandle<Quote>& stateVariable,
                        const RelinkableHandle<TermStructure>& dividendTS,
                        const RelinkableHandle<TermStructure>& riskFreeTS,
                        const RelinkableHandle<BlackVolTermStructure>& volTS,
                        const RelinkableHandle<Quote>& jumpInt,
                        const RelinkableHandle<Quote>& logJMean,
                        const RelinkableHandle<Quote>& logJVol);
        //! \name StochasticProcess interface
        //@{
        double drift(Time, double) const { QL_FAIL("not implemented"); }
        double diffusion(Time, double) const { QL_FAIL("not implemented"); }
        //@}
        //! \name Inspectors
        //@{
        const boost::shared_ptr<Quote>& jumpIntensity() const;
        const boost::shared_ptr<Quote>& logMeanJump() const;
        const boost::shared_ptr<Quote>& logJumpVolatility() const;
        //@}
      private:
        RelinkableHandle<Quote> jumpIntensity_, logMeanJump_, 
                                logJumpVolatility_;
    };

#ifndef QL_DISABLE_DEPRECATED
    //! \deprecated Use Merton76Process instead
    typedef Merton76Process Merton76StochasticProcess;
#endif


    //! Ornstein-Uhlenbeck process class
    /*! This class describes the Ornstein-Uhlenbeck process governed by
        \f[
            dx = -a x_t dt + \sigma dW_t.
        \f]
    */
    class OrnsteinUhlenbeckProcess : public StochasticProcess {
      public:
        OrnsteinUhlenbeckProcess(double speed,
                                 double vol,
                                 double x0 = 0.0);
        //! \name StochasticProcess interface
        //@{
        double x0() const;
        double drift(Time t, double x) const;
        double diffusion(Time t, double x) const;
        double expectation(Time t, double x0, Time dt) const;
        double variance(Time t, double x0, Time dt) const;
        //@}
      private:
        double x0_, speed_, volatility_;
    };


    //! Square-root process class
    /*! This class describes a square-root process governed by
        \f[
            dx = a (b - x_t) dt + \sigma \sqrt{x_t} dW_t.
        \f]
    */
    class SquareRootProcess : public StochasticProcess {
      public:
        SquareRootProcess(double b,
                          double a,
                          double sigma,
                          double x0 = 0.0);
        //! \name StochasticProcess interface
        //@{
        double x0() const;
        double drift(Time t, double x) const;
        double diffusion(Time t, double x) const;
        //@}
      private:
        double x0_, mean_, speed_, volatility_;
    };

}


#endif
