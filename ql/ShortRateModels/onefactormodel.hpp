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
    ql/ShortRateModels/%onefactormodel.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_one_factor_model_h
#define quantlib_interest_rate_modelling_one_factor_model_h

#include <ql/ShortRateModels/model.hpp>
#include <ql/Lattices/trinomialtree.hpp>

namespace QuantLib {

    namespace ShortRateModels {

        //! Single-factor short-rate model abstract class
        class OneFactorModel : public Model {
          public:
            OneFactorModel(Size nParameters);
            virtual ~OneFactorModel() {}

            class ShortRateDynamics;

            //! returns the short-rate dynamics
            virtual Handle<ShortRateDynamics> dynamics() const = 0;

            //! Return by default a trinomial recombining tree
            virtual Handle<Lattices::Tree> tree(const TimeGrid& grid) const;

          protected:
            class ShortRateTree;
            class ShortRateDiscounting;
        };

        //! Base class describing the short-rate dynamics
        class OneFactorModel::ShortRateDynamics {
          public:
            ShortRateDynamics(const Handle<DiffusionProcess>& process)
            : process_(process) {}
            virtual ~ShortRateDynamics() {};

            virtual double variable(Time t, Rate r) const = 0;
            virtual Rate shortRate(Time t, double variable) const = 0;

            //! Returns the risk-neutral dynamics of the underlying process
            const Handle<DiffusionProcess>& process() { return process_; }
          private:
            Handle<DiffusionProcess> process_;
        };

        class OneFactorModel::ShortRateTree : public Lattices::TrinomialTree {
          public:
            ShortRateTree(
                const Handle<ShortRateDynamics>& dynamics,
                const TimeGrid& timeGrid,
                bool isPositive = false);
            ShortRateTree(
                const Handle<ShortRateDynamics>& dynamics,
                const Handle<TermStructureFittingParameter::NumericalImpl>& phi,
                const TimeGrid& timeGrid,
                bool isPositive = false);
          private:
            class Helper;
        };

        class OneFactorModel::ShortRateDiscounting 
            : public Lattices::Discounting {
          public:
            ShortRateDiscounting(
                const Handle<ShortRateDynamics>& dynamics,
                const Handle<Lattices::TrinomialBranching>& branching,
                Time t, Time dt, double dx);

            double discount(Size index) const;
          private:
            Time t_, dt_;
            double xMin_, dx_;
            Handle<ShortRateDynamics> dynamics_;
        };

        class OneFactorAffineModel : public AffineModel {
          public:
            virtual double discountBond(Time now, 
                                        Time maturity, 
                                        Rate rate) const = 0;
        };

    }

}
#endif
