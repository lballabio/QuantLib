/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file diffusionprocess.hpp
    \brief Diffusion process

    \fullpath
    ql/%diffusionprocess.hpp
*/

// $Id$

#ifndef quantlib_diffusion_process_h
#define quantlib_diffusion_process_h

#include <ql/qldefines.hpp>
#include <ql/types.hpp>

namespace QuantLib {

    //! Diffusion process class
    /*! This class describes a stochastic process goverved by 
        \f[
            dx = \mu(t, x)dt + \sigma(t, x)dW_t.
        \f]
    */
    class DiffusionProcess {
      public:
        DiffusionProcess(double x0) : x0_(x0) {}
        virtual ~DiffusionProcess() {}

        double x0() const { return x0_; }

        virtual double drift(Time t, double x) const = 0;
        virtual double diffusion(Time t, double x) const = 0;
        //! By default, returns the Euler approximation of the expectation
        virtual double expectation(Time t0, double x0, Time dt) const {
            return x0 + drift(t0, x0)*dt;
        }
        //! By default, returns the Euler approximation of the variance
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
            dS = r dt + \sigma dW_t.
        \f]
    */
    class BlackScholesProcess : public DiffusionProcess {
      public:
        BlackScholesProcess(Rate rate, double volatility, double s0 = 0.0)
        : DiffusionProcess(s0), r_(rate), sigma_(volatility)  {}

        double drift(Time t, double x) const {
            return - r_*x;
        }
        double diffusion(Time t, double x) const {
            return sigma_;
        }
        double expectation(Time t0, double x0, Time dt) const {
            return x0 + r_*dt;
        }
        double variance(Time t0, double x0, Time dt) const {
            return sigma_*sigma_*dt;
        }
      private:
        double r_, sigma_;
    };

    //! Ornstein-Uhlenbeck process class
    /*! This class describes the Ornstein-Uhlenbeck process governed by 
        \f[
            dx = -a x_t dt + \sigma dW_t.
        \f]
    */
    class OrnsteinUhlenbeckProcess : public DiffusionProcess {
      public:
        OrnsteinUhlenbeckProcess(double speed, double vol, double x0 = 0.0)
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
        SquareRootProcess(double b, double a, double sigma, double x0 = 0)
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
