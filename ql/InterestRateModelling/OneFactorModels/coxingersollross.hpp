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

        class GeneralCoxIngersollRoss : public OneFactorModel {
          public:
            GeneralCoxIngersollRoss(
                const Parameter& k,
                const Parameter& theta,
                const Parameter& sigma,
                const RelinkableHandle<TermStructure>& termStructure) 
            : OneFactorModel(4, termStructure), 
              k_(parameters_[0]), theta_(parameters_[1]), 
              sigma_(parameters_[2]), phi_(parameters_[3]) {
                k_ = k;
                theta_ = theta;
                sigma_ = sigma;
                x0_ = termStructure->forward(0.0);
            }

            virtual ~GeneralCoxIngersollRoss() {}

            virtual Handle<Lattices::Tree> tree(const TimeGrid& grid) const {
                return Handle<Lattices::Tree>(
                    new OwnTrinomialTree(process(),
                                         phi_.implementation(), grid));
            }

            virtual Handle<ShortRateProcess> process() const {
                return Handle<ShortRateProcess>(
                    new Process(phi_, theta_, k_ , sigma_, x0_));
            }

          protected:
            virtual void generateParameters() {
                phi_ = TermStructureFittingParameter(termStructure());
            }

            Parameter& k_;
            Parameter& theta_;
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
                : phi_(phi), theta_(theta), k_(k), sigma_(sigma), x0_(x0) {}

                virtual double variable(Time t, Rate r) const {
                    return QL_SQRT(r - phi_(t)) - QL_SQRT(x0_);
                }
                virtual double shortRate(Time t, double y) const {
                    double v = y + QL_SQRT(x0_);
                    return v*v + phi_(t);
                }
                virtual double drift(Time t, double y) const {
                    double v = y + QL_SQRT(x0_);
                    return (0.5*theta_(t)*k_(t) - 0.125*sigma_(t)*sigma_(t))/v 
                        - 0.5*k_(t)*v;
                }
                virtual double diffusion(Time t, double y) const {
                    return 0.5*sigma_(t);
                }
              private:
                Parameter phi_, theta_, k_, sigma_;
                double x0_;
            };

        };

        class ExtendedCoxIngersollRoss : public GeneralCoxIngersollRoss {
          public:
            ExtendedCoxIngersollRoss(
                const RelinkableHandle<TermStructure>& termStructure);

            virtual ~ExtendedCoxIngersollRoss() {}

            virtual bool hasDiscountBondFormula() const { return true; }
            virtual double discountBond(Time T, Time s, Rate r) const;
/*
            virtual bool hasDiscountBondOptionFormula() const { return true; }
            virtual double discountBondOption(Option::Type type, 
                                              double strike,
                                              Time T, 
                                              Time s) const;
*/

          protected:
            virtual void generateParameters() {
                phi_ = FittingParameter(termStructure(), theta(), k(), sigma(), 
                                        x0_);
            }

          private:
            class FittingParameter : public TermStructureFittingParameter {
              public:
                class CIRImpl : public ParameterImpl {
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
                    new CIRImpl(termStructure, k, theta, sigma, x0))) {}
            };

            double A(Time t, Time T) const;
            double B(Time t, Time T) const;
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
            : 

            virtual ~CoxIngersollRoss() {}

            virtual double discountBond(Time t, Time T, Rate r) const {
                double value =  A(t,T)*QL_EXP(-B(t,T)*r);
                return value;
            }

          protected:
            virtual void generateParameters() {
                phi_ = FittingParameter(termStructure(), theta(), k(), sigma(), 
                                        x0_);
            }

          private:
            double theta() const { return theta_(0.0); }
            double k() const { return k_(0.0); }
            double sigma() const { return sigma_(0.0); }

        };
*/
    }

}

#endif
