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
/*! \file coxingersollross.hpp
    \brief Cox-Ingersoll-Ross model

    \fullpath
    ql/InterestRateModelling/OneFactorModels/%coxingersollross.hpp
*/

// $Id$

#ifndef quantlib_one_factor_models_cox_ingersoll_ross_h
#define quantlib_one_factor_models_cox_ingersoll_ross_h

#include "ql/InterestRateModelling/onefactormodel.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        //! General single-factor extended Cox-Ingersoll-Ross model class.
        /*! This class implements the extended Cox-Ingersoll-Ross model
            defined by 
            \f[ 
                dr_t = (\theta(t) - \alpha(t)r_t)dt + \sqrt{r_t}\sigma(t)dW_t .
            \f]
            It is actually implemented as \f$ r_t = \varphi(t) + y_t^2 \f$
            where y_t is defined by 
            \f[
                dy_t=\left[ 
                        (\frac{k\theta }{2}+\frac{\sigma ^2}{8})\frac{1}{y_t}-
                        \frac{k}{2}y_t \right] d_t+ \frac{\sigma }{2}dW_{t}
            \f]
        */
        class GeneralCoxIngersollRoss : public OneFactorModel {
          public:
            GeneralCoxIngersollRoss(
                const Parameter& theta,
                const Parameter& k,
                const Parameter& sigma,
                const RelinkableHandle<TermStructure>& termStructure) 
            : OneFactorModel(4, termStructure), 
              theta_(parameters_[0]), k_(parameters_[1]), 
              sigma_(parameters_[2]), phi_(parameters_[3]) {
                theta_ = theta;
                k_ = k;
                sigma_ = sigma;
                x0_ = termStructure->forward(0.0);
            }

            virtual ~GeneralCoxIngersollRoss() {}

            virtual Handle<Lattices::Tree> tree(const TimeGrid& grid) const {
                return Handle<Lattices::Tree>(
                    new OwnTrinomialTree(process(),
                                         phi_.implementation(), grid, true));
            }

            virtual Handle<ShortRateProcess> process() const {
                return Handle<ShortRateProcess>(
                    new Process(phi_, theta_, k_ , sigma_, x0_));
            }

          protected:
            virtual void generateParameters() {
                phi_ = TermStructureFittingParameter(termStructure());
            }

            Parameter& theta_;
            Parameter& k_;
            Parameter& sigma_;
            Parameter& phi_;
            double x0_;

          private:
            class Process : public ShortRateProcess {
              public:
                Process(const Parameter& phi,
                        const Parameter& theta,
                        const Parameter& k,
                        const Parameter& sigma,
                        double x0)
                : ShortRateProcess(QL_SQRT(x0)),
                  phi_(phi), theta_(theta), k_(k), sigma_(sigma) {}

                virtual double variable(Time t, Rate r) const {
                    return QL_SQRT(r - phi_(t));
                }
                virtual double shortRate(Time t, double y) const {
                    return y*y + phi_(t);
                }
                virtual double drift(Time t, double y) const {
                    return (0.5*theta_(t)*k_(t) - 0.125*sigma_(t)*sigma_(t))/y 
                           - 0.5*k_(t)*y;
                }
                virtual double diffusion(Time t, double y) const {
                    return 0.5*sigma_(t);
                }
              private:
                Parameter phi_, theta_, k_, sigma_;
            };

        };

        class ExtendedCoxIngersollRoss 
        : public GeneralCoxIngersollRoss, public OneFactorAffineModel {
          public:
            ExtendedCoxIngersollRoss(
                const RelinkableHandle<TermStructure>& termStructure);

            virtual ~ExtendedCoxIngersollRoss() {}

            double discountBond(Time T, Time s, Rate r) const;
            double discountBondOption(Option::Type type,
                                      double strike,
                                      Time maturity,
                                      Time bondMaturity) const;

            virtual Handle<Lattices::Tree> tree(const TimeGrid& grid) const {
                return Handle<Lattices::Tree>(
                    new OwnTrinomialTree(process(), grid, true));
            }

          protected:
            virtual void generateParameters() {
                phi_ = FittingParameter(termStructure(), theta(), k(), sigma(), 
                                        x0_);
            }

          private:
            class FittingParameter : public TermStructureFittingParameter {
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

            double A(Time t) const;
            double B(Time t) const;
            double C(Time t, Time T) const;
            double theta() const { return theta_(0.0); }
            double k() const { return k_(0.0); }
            double sigma() const { return sigma_(0.0); }

            class OwnConstraint;
        };
/*
        class CoxIngersollRoss : public ExtendedCoxIngersollRoss {
          public:
            CoxIngersollRoss(Rate r0) 
            : ExtendedCoxIngersollRoss(RelinkableHandle<TermStructure>(
                                       new ModelTermStructure(this, 0.0, r0))) {
                phi_ = NullParameter();
            }

            virtual ~CoxIngersollRoss() {}

            virtual double discountBond(Time t, Time T, Rate r) const {
                double value =  A(t,T)*QL_EXP(-B(t,T)*r);
                return value;
            }

          protected:
            virtual void generateParameters() {}
        };
*/
    }

}

#endif
