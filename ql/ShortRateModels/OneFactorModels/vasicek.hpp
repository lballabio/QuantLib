
/*
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

/*! \file vasicek.hpp
    \brief Vasicek model class
*/

#ifndef quantlib_one_factor_models_vasicek_h
#define quantlib_one_factor_models_vasicek_h

#include <ql/ShortRateModels/onefactormodel.hpp>

namespace QuantLib {

    //! %Vasicek model class
    /*! This class implements the Vasicek model defined by 
        \f[ 
            dr_t = a(b - r_t)dt + \sigma dW_t ,
        \f]
        where \f$ a \f$, \f$ b \f$ and \f$ \sigma \f$ are constants. 
    */
    class Vasicek : public OneFactorAffineModel {
      public:
        Vasicek(Rate r0 = 0.05, 
                double a = 0.1, double b = 0.05, double sigma = 0.01);

        virtual double discountBondOption(Option::Type type,
                                          double strike,
                                          Time maturity,
                                          Time bondMaturity) const;

        virtual Handle<ShortRateDynamics> dynamics() const;

      protected:
        virtual double A(Time t, Time T) const;
        virtual double B(Time t, Time T) const;

        double a() const { return a_(0.0); }
        double b() const { return b_(0.0); }
        double sigma() const { return sigma_(0.0); }

      private:
        class Dynamics;

        double r0_;
        Parameter& a_;
        Parameter& b_;
        Parameter& sigma_;
    };

    //! Short-rate dynamics in the %Vasicek model
    /*! The short-rate follows an Ornstein-Uhlenbeck process with mean
        \f$ b \f$.
    */
    class Vasicek::Dynamics : public ShortRateDynamics {
      public:
        Dynamics(double a,
                 double b,
                 double sigma,
                 double r0)
        : ShortRateDynamics(Handle<DiffusionProcess>(
                             new OrnsteinUhlenbeckProcess(a, sigma, r0 - b))),
          a_(a), b_(b), r0_(r0) {}

        virtual double variable(Time t, Rate r) const {
            return r - b_;
        }
        virtual double shortRate(Time t, double x) const {
            return x + b_;
        }
      private:
        double a_, b_, r0_;
    };


    // inline definitions

    inline Handle<OneFactorModel::ShortRateDynamics> 
    Vasicek::dynamics() const {
        return Handle<ShortRateDynamics>(
                                     new Dynamics(a(), b() , sigma(), r0_));
    }

}


#endif
