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
/*! \file extendedcoxingersollross.hpp
    \brief Extended Cox-Ingersoll-Ross model

    \fullpath
    ql/ShortRateModels/OneFactorModels/%extendedcoxingersollross.hpp
*/

// $Id$

#ifndef quantlib_one_factor_models_extended_cox_ingersoll_ross_h
#define quantlib_one_factor_models_extended_cox_ingersoll_ross_h

#include <ql/ShortRateModels/OneFactorModels/coxingersollross.hpp>

namespace QuantLib {

    namespace ShortRateModels {

        //! Extended Cox-Ingersoll-Ross model class.
        /*! This class implements the extended Cox-Ingersoll-Ross model
            defined by 
            \f[ 
                dr_t = (\theta(t) - \alpha r_t)dt + \sqrt{r_t}\sigma dW_t .
            \f]
            It is actually implemented as \f$ r_t = \varphi(t) + x_t \f$
            where x_t follows a standard Cox-Ingersoll-Ross process.
        */

        class ExtendedCoxIngersollRoss : public CoxIngersollRoss,
                                         public TermStructureConsistentModel {
          public:
            ExtendedCoxIngersollRoss(
                const RelinkableHandle<TermStructure>& termStructure,
                double theta = 0.1,
                double k = 0.1,
                double sigma = 0.1,
                double x0 = 0.05);

            Handle<Lattices::Tree> tree(const TimeGrid& grid) const;

            Handle<ShortRateDynamics> dynamics() const;

            double discountBond(Time T, Time s, Rate r) const;
            double discountBondOption(Option::Type type,
                                      double strike,
                                      Time maturity,
                                      Time bondMaturity) const;

          protected:
            void generateParameters();

          private:
            class FittingParameter;
            class Dynamics;

            double C(Time t, Time T) const;

            Parameter phi_;
        };

        class ExtendedCoxIngersollRoss::FittingParameter 
        : public TermStructureFittingParameter {
          public:
              class CIRImpl : public Parameter::ParameterImpl {
              public:
                CIRImpl(
                    const RelinkableHandle<TermStructure>& termStructure,
                    double theta, double k, double sigma, double x0) 
                : termStructure_(termStructure), 
                  theta_(theta), k_(k), sigma_(sigma), x0_(x0) {}
                virtual ~CIRImpl() {}

                double value(const Array& params, Time t) const {
                    double forwardRate = termStructure_->forward(t);
                    double h = QL_SQRT(k_*k_ + 2.0*sigma_*sigma_);
                    double expth = QL_EXP(t*h);
                    double temp = 2.0*h + (k_+h)*(expth-1.0);
                    double phi = forwardRate -
                                 2.0*k_*theta_*(expth - 1.0)/temp -
                                 x0_*4.0*h*h*expth/(temp*temp);
                    return phi;
                }
              private:
                RelinkableHandle<TermStructure> termStructure_;
                double theta_, k_, sigma_, x0_;
            };
            FittingParameter(
                const RelinkableHandle<TermStructure>& termStructure,
                double theta, double k, double sigma, double x0) 
            : TermStructureFittingParameter(Handle<ParameterImpl>(
                new CIRImpl(termStructure, theta, k, sigma, x0))) {}
        };

        class ExtendedCoxIngersollRoss::Dynamics
        : public CoxIngersollRoss::Dynamics {
          public:
            Dynamics(const Parameter& phi,
                     double theta,
                     double k,
                     double sigma,
                     double x0)
            : CoxIngersollRoss::Dynamics(theta, k, sigma, x0), phi_(phi) {}

            virtual double variable(Time t, Rate r) const {
                return QL_SQRT(r - phi_(t));
            }
            virtual double shortRate(Time t, double y) const {
                return y*y + phi_(t);
            }
          private:
            Parameter phi_;
        };

        inline Handle<OneFactorModel::ShortRateDynamics> 
        ExtendedCoxIngersollRoss::dynamics() const {
            return Handle<ShortRateDynamics>(
                new Dynamics(phi_, theta(), k() , sigma(), x0()));
        }

        inline void ExtendedCoxIngersollRoss::generateParameters() {
            phi_ = FittingParameter(termStructure(), theta(), k(), sigma(), 
                                    x0());
        }

    }

}

#endif
