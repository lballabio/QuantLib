
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
    class StochasticProcess : public Observer, public Observable {
      public:
        //! discretization of a stochastic process over a given time interval
        class discretization {
          public:
            virtual ~discretization() {}
            virtual Real expectation(const StochasticProcess&,
                                     Time t0, Real x0, Time dt) const = 0;
            virtual Real variance(const StochasticProcess&,
                                  Time t0, Real x0, Time dt) const = 0;
        };
        virtual ~StochasticProcess() {}
        //! returns the initial value of the state variable
        virtual Real x0() const = 0;
        //! returns the drift part of the equation, i.e. \f$ \mu(t, x_t) \f$
        virtual Real drift(Time t, Real x) const = 0;
        /*! \brief returns the diffusion part of the equation, i.e.
            \f$ \sigma(t, x_t) \f$
        */
        virtual Real diffusion(Time t, Real x) const = 0;
        /*! returns the expectation
            \f$ E(x_{t_0 + \Delta t} | x_{t_0} = x_0) \f$
            of the process after a time interval \f$ \Delta t \f$
            according to the given discretization. This method can be
            overridden in derived classes which want to hard-code a
            particular discretization.
        */
        virtual Real expectation(Time t0, Real x0, Time dt) const;
        /*! returns the variance
            \f$ V(x_{t_0 + \Delta t} | x_{t_0} = x_0) \f$
            of the process after a time interval \f$ \Delta t \f$
            according to the given discretization. This method can be
            overridden in derived classes which want to hard-code a
            particular discretization.
        */
        virtual Real variance(Time t0, Real x0, Time dt) const;
        /*! applies a change to the asset value. By default; it
            returns \f$ x + \Delta x \f$.
        */
        virtual Real evolve(Real change, Real currentValue) const;

        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        StochasticProcess();
        StochasticProcess(const boost::shared_ptr<discretization>&);
        boost::shared_ptr<discretization> discretization_;
    };


    //! Euler discretization for stochastic processes
    class EulerDiscretization : public StochasticProcess::discretization {
      public:
        /*! Returns an approximation of the expected value defined as
            \f$ x_0 + \mu(t_0, x_0) \Delta t \f$.
        */
        Real expectation(const StochasticProcess&,
                         Time t0, Real x0, Time dt) const;
        /*! Returns an approximation of the variance defined as
            \f$ \sigma(t_0, x_0)^2 \Delta t \f$.
        */
        Real variance(const StochasticProcess&,
                      Time t0, Real x0, Time dt) const;
    };

    //! Geometric brownian motion process
    /*! This class describes the stochastic process governed by
        \f[
            dS(t, S)= \mu S dt + \sigma S dW_t.
        \f]
    */
    class GeometricBrownianMotionProcess : public StochasticProcess {
      public:
        GeometricBrownianMotionProcess(double initialValue,
                                       double mue,
                                       double sigma);
        Real x0() const;
        Real drift(Time t, Real x) const;
        Real diffusion(Time t, Real x) const;
      protected:
        double initialValue_;
        double mue_;
        double sigma_;
    };

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


    //! Merton-76 jump-diffusion process
    class Merton76Process : public BlackScholesProcess {
      public:
        Merton76Process(
            const Handle<Quote>& stateVariable,
            const Handle<YieldTermStructure>& dividendTS,
            const Handle<YieldTermStructure>& riskFreeTS,
            const Handle<BlackVolTermStructure>& blackVolTS,
            const Handle<Quote>& jumpInt,
            const Handle<Quote>& logJMean,
            const Handle<Quote>& logJVol,
            const boost::shared_ptr<StochasticProcess::discretization>& d =
                    boost::shared_ptr<StochasticProcess::discretization>(
                                                    new EulerDiscretization));
        //! \name StochasticProcess interface
        //@{
        Real drift(Time, Real) const { QL_FAIL("not implemented"); }
        Real diffusion(Time, Real) const { QL_FAIL("not implemented"); }
        Real evolve(Real change, Real currentValue) const {
            QL_FAIL("not implemented");
        }
        //@}
        //! \name Inspectors
        //@{
        const boost::shared_ptr<Quote>& jumpIntensity() const;
        const boost::shared_ptr<Quote>& logMeanJump() const;
        const boost::shared_ptr<Quote>& logJumpVolatility() const;
        //@}
      private:
        Handle<Quote> jumpIntensity_, logMeanJump_, logJumpVolatility_;
    };


    //! Ornstein-Uhlenbeck process class
    /*! This class describes the Ornstein-Uhlenbeck process governed by
        \f[
            dx = -a x_t dt + \sigma dW_t.
        \f]
    */
    class OrnsteinUhlenbeckProcess : public StochasticProcess {
      public:
        OrnsteinUhlenbeckProcess(Real speed,
                                 Volatility vol,
                                 Real x0 = 0.0);
        //! \name StochasticProcess interface
        //@{
        Real x0() const;
        Real drift(Time t, Real x) const;
        Real diffusion(Time t, Real x) const;
        Real expectation(Time t0, Real x0, Time dt) const;
        Real variance(Time t0, Real x0, Time dt) const;
        //@}
      private:
        Real x0_, speed_;
        Volatility volatility_;
    };


    //! Square-root process class
    /*! This class describes a square-root process governed by
        \f[
            dx = a (b - x_t) dt + \sigma \sqrt{x_t} dW_t.
        \f]
    */
    class SquareRootProcess : public StochasticProcess {
      public:
        SquareRootProcess(
            Real b, Real a, Volatility sigma, Real x0 = 0.0,
            const boost::shared_ptr<StochasticProcess::discretization>& d =
                    boost::shared_ptr<StochasticProcess::discretization>(
                                                    new EulerDiscretization));
        //! \name StochasticProcess interface
        //@{
        Real x0() const;
        Real drift(Time t, Real x) const;
        Real diffusion(Time t, Real x) const;
        //@}
      private:
        Real x0_, mean_, speed_;
        Volatility volatility_;
    };

}


#endif
