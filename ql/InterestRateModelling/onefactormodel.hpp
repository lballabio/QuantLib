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
/*! \file onefactormodel.hpp
    \brief Abstract one-factor interest rate model class

    \fullpath
    ql/InterestRateModelling/%onefactormodel.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_one_factor_model_h
#define quantlib_interest_rate_modelling_one_factor_model_h

#include <ql/InterestRateModelling/model.hpp>
#include <ql/InterestRateModelling/shortrateprocess.hpp>
#include <ql/Lattices/trinomialtree.hpp>

namespace QuantLib {

    namespace InterestRateModelling {

        //! Single-factor short-rate model abstract class
        class OneFactorModel : public Model {
          public:
            OneFactorModel(
                Size nParameters,
                const RelinkableHandle<TermStructure>& termStructure);
            virtual ~OneFactorModel() {}

            //! returns the driving stochastic equation
            virtual Handle<ShortRateProcess> process() const = 0;

            //! Return a recombining tree
            virtual Handle<Lattices::Tree> tree(const TimeGrid& grid) const {
                return Handle<Lattices::Tree>(
                    new OwnTrinomialTree(process(), grid));
            }

          protected:

            class OwnTrinomialTree : public Lattices::TrinomialTree {
              public:
                OwnTrinomialTree(const Handle<ShortRateProcess>& process,
                                 const TimeGrid& timeGrid,
                                 bool isPositive = false) 
                : Lattices::TrinomialTree(process, timeGrid, isPositive), 
                  process_(process) {}

                OwnTrinomialTree(
                    const Handle<ShortRateProcess>& process,
                    const Handle<TermStructureFittingParameter::NumericalImpl>& 
                          theta,
                    const TimeGrid& timeGrid,
                    bool isPositive = false);

                virtual DiscountFactor discount(Size i, int j) const {
                     double x = process_->x0() + j*dx(i);
                     Rate r = process_->shortRate(t(i), x);
                     return QL_EXP(-r*dt(i));
                }
              protected:
                Handle<ShortRateProcess> process_;
              private:
                class Helper;
            };

          private:
            class StandardConstraint;
        };

        class OneFactorAffineModel : public AffineModel {
          public:
            virtual double discountBond(
                Time now,
                Time maturity,
                Rate rate) const = 0;
        };

        //! Term structure implied by a model
        class ModelTermStructure : public DiscountStructure {
          public:
            ModelTermStructure(
                const OneFactorAffineModel* model, Time t0, Rate r0) 
            : model_(model), t0_(t0), r0_(r0) {}

            virtual DiscountFactor discountImpl(Time t, bool extrapolate) {
                return model_->discountBond(t0_, t0_+t, r0_);
            }
          private:
            const OneFactorAffineModel* model_;
            Time t0_;
            Rate r0_;
        };
    }

}
#endif
