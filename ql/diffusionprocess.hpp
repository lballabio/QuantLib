
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file diffusionprocess.hpp
    \brief Diffusion process
*/

#ifndef quantlib_diffusion_process_h
#define quantlib_diffusion_process_h

#include <ql/termstructure.hpp>
#include <ql/voltermstructure.hpp>

namespace QuantLib {

    //! Diffusion process class
    /*! This class describes a stochastic process governed by
        \f[
            dx_t = \mu(t, x_t)dt + \sigma(t, x_t)dW_t.
        \f]
    */
    class DiffusionProcess {
      public:
        DiffusionProcess(double x0)
        : x0_(x0) {}
        virtual ~DiffusionProcess() {}

        double x0() const { return x0_; }

        //! returns the drift part of the equation, i.e. \f$ \mu(t, x_t) \f$
        virtual double drift(Time t, double x) const = 0;

        /*! returns the diffusion part of the equation, i.e.
            \f$\sigma(t, x_t)\f$
        */
        virtual double diffusion(Time t, double x) const = 0;

        //! returns the expectation of the process after a time interval
        /*! returns \f$ E(x_{t_0 + \Delta t} | x_{t_0} = x_0) \f$.
            By default, it returns the Euler approximation defined by
            \f$ x_0 + \mu(t_0, x_0) \Delta t \f$.
        */
        virtual double expectation(Time t0, double x0, Time dt) const {
            return x0 + drift(t0, x0)*dt;
        }

        //! returns the variance of the process after a time interval
        /*! returns \f$ Var(x_{t_0 + \Delta t} | x_{t_0} = x_0) \f$.
            By default, it returns the Euler approximation defined by
            \f$ \sigma(t_0, x_0)^2 \Delta t \f$.
        */
        virtual double variance(Time t0, double x0, Time dt) const {
            double sigma = diffusion(t0, x0);
            return sigma*sigma*dt;
        }
      private:
        double x0_;
    };

    //! Black-Scholes diffusion process class
    /*! This class describes the stochastic process governed by
        \f[
            dS(t, S)= (r(t) - q(t) - \frac{\sigma(t, S)^2}{2}) dt + \sigma dW_t.
        \f]

        \todo revise extrapolation
    */
    class BlackScholesProcess : public DiffusionProcess {
      public:
        BlackScholesProcess(
            const RelinkableHandle<TermStructure>& riskFreeTS,
            const RelinkableHandle<TermStructure>& dividendTS,
            const RelinkableHandle<BlackVolTermStructure>& blackVolTS,
            double s0);
        double drift(Time t, double x) const ;
        double diffusion(Time t, double x) const {
            // this is a quick and dirty patch
            // rethink how to handle extrapolation
            return localVolTS_->localVol(t, x, true);
        }
      private:
        RelinkableHandle<TermStructure> riskFreeTS_, dividendTS_;
        RelinkableHandle<LocalVolTermStructure> localVolTS_;
    };

    //! Ornstein-Uhlenbeck process class
    /*! This class describes the Ornstein-Uhlenbeck process governed by
        \f[
            dx = -a x_t dt + \sigma dW_t.
        \f]
    */
    class OrnsteinUhlenbeckProcess : public DiffusionProcess {
      public:
        OrnsteinUhlenbeckProcess(double speed,
                                 double vol,
                                 double x0 = 0.0)
        : DiffusionProcess(x0), speed_(speed), volatility_(vol)  {}

        double drift(Time t, double x) const {
            return - speed_*x;
        }
        double diffusion(Time t, double x) const {
            return volatility_;
        }
        double expectation(Time t0, double x0, Time dt) const {
            return x0*QL_EXP(-speed_*dt);
        }
        double variance(Time t0, double x0, Time dt) const {
            return 0.5*volatility_*volatility_/speed_*
                   (1.0 - QL_EXP(-2.0*speed_*dt));
        }
      private:
        double speed_, volatility_;
    };

    //! Square-root process class
    /*! This class describes a square-root process governed by
        \f[
            dx = a (b - x_t) dt + \sigma \sqrt{x_t} dW_t.
        \f]
    */
    class SquareRootProcess : public DiffusionProcess {
      public:
        SquareRootProcess(double b,
                          double a,
                          double sigma,
                          double x0 = 0)
        : DiffusionProcess(x0), mean_(b), speed_(a), volatility_(sigma)  {}

        double drift(Time t, double x) const {
            return speed_*(mean_ - x);
        }
        double diffusion(Time t, double x) const {
            return volatility_*QL_SQRT(x);
        }
      private:
        double mean_, speed_, volatility_;
    };

}


#endif
