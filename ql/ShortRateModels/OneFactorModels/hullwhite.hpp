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
/*! \file hullwhite.hpp
    \brief Hull & White (HW) model

    \fullpath
    ql/ShortRateModels/OneFactorModels/%hullwhite.hpp
*/

// $Id$

#ifndef quantlib_one_factor_models_hull_white_h
#define quantlib_one_factor_models_hull_white_h

#include <ql/ShortRateModels/OneFactorModels/vasicek.hpp>

namespace QuantLib {

    namespace ShortRateModels {

        //! Single-factor Hull-White model class.
        /*! This class implements the standard single-factor Hull-White model
            defined by 
            \f[ 
                dr_t = (\theta(t) - \alpha r_t)dt + \sigma dW_t
            \f]
            where \f$ \alpha \f$ and \f$ \sigma \f$ are constants. 
            \f$ \varphi(t) \f$ is analytically defined by
            \f[
                \varphi(t) = f(t) + \frac{1}{2}[\frac{\sigma(1-e^{-at})}{a}]^2.
            \f]
        */
        class HullWhite : public Vasicek, public TermStructureConsistentModel {
          public:
            HullWhite(const RelinkableHandle<TermStructure>& termStructure, 
                      double a = 0.1, double sigma = 0.01);
           
            Handle<Lattices::Tree> tree(const TimeGrid& timeGrid) const;

            Handle<ShortRateDynamics> dynamics() const;

            /*! Analytical formula for discount bonds:
                \f[
                    P(t, T, r_t) = A(t,T)e^{-B(t,T)r_t}.
                \f]
            */
            double discountBond(Time t, Time T, Rate r) const;
            double discountBondOption(Option::Type type,
                                      double strike,
                                      Time maturity,
                                      Time bondMaturity) const;

          protected:
            void generateParameters();

            double A(Time t, Time T) const;
            // remove the warning
            double A(Time t) const { return Vasicek::A(t); } 
            double B(Time t) const;

          private:
            class FittingParameter;
            class Dynamics;

            Parameter phi_;

        };

        class HullWhite::FittingParameter 
            : public TermStructureFittingParameter {
          public:
            class HullWhiteImpl : public Parameter::ParameterImpl {
              public:
                HullWhiteImpl(
                    const RelinkableHandle<TermStructure>& termStructure,
                   double a, double sigma) 
                : termStructure_(termStructure), a_(a), sigma_(sigma) {}
                virtual ~HullWhiteImpl() {}

                double value(const Array& params, Time t) const {
                    double forwardRate = termStructure_->forward(t);
                    double temp = sigma_*(1.0 - QL_EXP(-a_*t))/a_;
                    return (forwardRate + 0.5*temp*temp);
                }
              private:
                RelinkableHandle<TermStructure> termStructure_;
                double a_, sigma_;
            };

            FittingParameter(
                const RelinkableHandle<TermStructure>& termStructure,
                double a, double sigma)
            : TermStructureFittingParameter(Handle<ParameterImpl>(
                new HullWhiteImpl(termStructure, a, sigma))) {}
        };

        class HullWhite::Dynamics : public ShortRateDynamics {
          public:
            Dynamics(const Parameter& fitting,
                     double a,
                     double sigma)
            : ShortRateDynamics(Handle<DiffusionProcess>(
                  new OrnsteinUhlenbeckProcess(a, sigma))),
              fitting_(fitting) {}

            double variable(Time t, Rate r)    const { return r - fitting_(t); }
            double shortRate(Time t, double x) const { return x + fitting_(t); }
          private:
            Parameter fitting_;
        };

        inline Handle<OneFactorModel::ShortRateDynamics> 
        HullWhite::dynamics() const {
            return Handle<ShortRateDynamics>(new Dynamics(phi_, a(), sigma()));
        }

    }

}

#endif
