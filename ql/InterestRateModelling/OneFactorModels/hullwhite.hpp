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
    ql/InterestRateModelling/OneFactorModels/%hullwhite.hpp
*/

// $Id$

#ifndef quantlib_one_factor_models_hull_white_h
#define quantlib_one_factor_models_hull_white_h

#include "ql/InterestRateModelling/onefactormodel.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        class GeneralHullWhite : public OneFactorModel {
          public:
            GeneralHullWhite(
                const Parameter& a,
                const Parameter& sigma,
                const RelinkableHandle<TermStructure>& termStructure) 
            : OneFactorModel(3, termStructure), 
              a_(parameters_[0]), sigma_(parameters_[1]), f_(parameters_[2]) {
                a_ = a;
                sigma_ = sigma;
            }
            virtual ~GeneralHullWhite() {}

            virtual Handle<Lattices::Tree> tree(const TimeGrid& grid) const {
                return Handle<Lattices::Tree>(
                    new OwnTrinomialTree(process(), f_.implementation(), grid));
            }

            virtual Handle<ShortRateProcess> process() const {
                return Handle<ShortRateProcess>(
                    new Process(f_, a_, sigma_));
            }
          protected:
            virtual void generateParameters() {
                f_ = TermStructureFittingParameter(termStructure());
            }

            Parameter& a_;
            Parameter& sigma_;
            Parameter& f_;
          private:
            class Process : public OrnsteinUhlenbeckProcess {
              public:
                Process(const Parameter& fitting,
                        const Parameter& speed,
                        const Parameter& volatility)
                : OrnsteinUhlenbeckProcess(speed, volatility),
                  fitting_(fitting) {}
                virtual double variable(Time t, Rate r) const {
                    return r - fitting_(t);
                }

                virtual Rate shortRate(Time t, double x) const {
                    return x + fitting_(t);
                }
              private:
                Parameter fitting_;
            };

        };

        class HullWhite : public GeneralHullWhite {
          public:
            HullWhite(const RelinkableHandle<TermStructure>& termStructure)
            : GeneralHullWhite(ConstantParameter(0.1), ConstantParameter(0.1), 
                               termStructure) {
                generateParameters();
            }
            virtual ~HullWhite() {}

            virtual Handle<Lattices::Tree> tree(const TimeGrid& grid) const {
                return Handle<Lattices::Tree>(
                    new OwnTrinomialTree(process(), grid));
            }

            virtual bool hasDiscountBondFormula() const { return true; }
            virtual double discountBond(Time T, Time s, Rate r) const;

            virtual bool hasDiscountBondOptionFormula() const { return true; }
            virtual double discountBondOption(Option::Type type,
                                              double strike,
                                              Time maturity,
                                              Time bondMaturity) const;

          protected:
            virtual void generateParameters() {
                f_ = HWFittingParameter(termStructure(), a_(0.0), sigma_(0.0));
            }

          private:
            class HWFittingParameter : public TermStructureFittingParameter {
              public:
                class HullWhiteImpl : public ParameterImpl {
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

                HWFittingParameter(
                    const RelinkableHandle<TermStructure>& termStructure,
                    double a, double sigma)
                : TermStructureFittingParameter(Handle<ParameterImpl>(
                    new HullWhiteImpl(termStructure, a, sigma))) {}
            };

            double a() const { return a_(0.0); }
            double sigma() const { return sigma_(0.0); }
            double B(Time t) const;
            double lnA(Time T, Time s) const;

        };

    }

}

#endif
