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
/*! \file onefactormodel.cpp
    \brief Abstract one-factor interest rate model class

    \fullpath
    ql/InterestRateModelling/%onefactormodel.cpp
*/

// $Id$

#include "ql/InterestRateModelling/onefactormodel.hpp"
#include "ql/Lattices/trinomialtree.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        using Lattices::TimeGrid;
        using Lattices::Tree;
        using Lattices::TrinomialTree;
        using Optimization::Constraint;

        class OneFactorModel::StandardConstraint : public Constraint {
            virtual bool test(const Array& params) const {
                if (params[params.size()-1] > 0.0)
                    return true;
                else
                    return false;
            }
            virtual void correct(Array& params) const {
                if (!test(params))
                    params[params.size()-1] = 0.000001;
            }
        };

        class OneFactorModel::PrivateTree : public TrinomialTree {
          public:
            PrivateTree(const OneFactorModel* model, const TimeGrid& timeGrid) 
            : TrinomialTree(model->process(), timeGrid), model_(model) {}

            virtual DiscountFactor discount(Size i, int j) const {
                 Rate r = model_->process()->shortRate(t(i), j*dx(i));
                 return QL_EXP(-r*dt(i));
            }
          private:
            const OneFactorModel* model_;

        };

        OneFactorModel::OneFactorModel(
            Size nParams,
            const RelinkableHandle<TermStructure>& termStructure)
        : Model(nParams, OneFactor, termStructure) {
            constraint_ = Handle<Constraint>(new StandardConstraint());
        }

        Handle<Tree> OneFactorModel::tree(const TimeGrid& timeGrid) const {
            return Handle<Tree>(new PrivateTree(this, timeGrid));
        }

    }

}
