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
/*! \file twofactormodel.hpp
    \brief Abstract two-factor interest rate model class

    \fullpath
    ql/ShortRateModels/%twofactormodel.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_two_factor_model_h
#define quantlib_interest_rate_modelling_two_factor_model_h

#include <ql/diffusionprocess.hpp>
#include <ql/ShortRateModels/model.hpp>
#include <ql/Lattices/twodimensionaltree.hpp>

namespace QuantLib {

    namespace ShortRateModels {

        class TwoFactorModel : public Model {
          public:
            TwoFactorModel(Size nParams);

            class ShortRateDynamics;

            //! Returns the short-rate dynamics
            virtual Handle<ShortRateDynamics> dynamics() const = 0;

            virtual Handle<Lattices::Tree> tree(const TimeGrid& grid) const;
          protected:
            class ShortRateDiscounting;
        };

        class TwoFactorModel::ShortRateDynamics {
          public:
            ShortRateDynamics(const Handle<DiffusionProcess>& xProcess,
                              const Handle<DiffusionProcess>& yProcess,
                              double correlation)
            : xProcess_(xProcess), yProcess_(yProcess), 
              correlation_(correlation) {}
            virtual ~ShortRateDynamics() {}

            virtual Rate shortRate(Time t, double x, double y) const = 0;
            const Handle<DiffusionProcess>& xProcess() const {
                return xProcess_;
            }
            const Handle<DiffusionProcess>& yProcess() const {
                return yProcess_;
            }
            double correlation() const {
                return correlation_;
            }
          private:
            Handle<DiffusionProcess> xProcess_, yProcess_;
            double correlation_;
        };

        class TwoFactorModel::ShortRateDiscounting 
            : public Lattices::Discounting {
          public:
            ShortRateDiscounting(
                const Handle<ShortRateDynamics>& process,
                const Handle<Lattices::TwoDimensionalBranching>& branching,
                Time t, Time dt, double dx, double dy);

            double discount(Size index) const;
          private:
            Size modulo_;
            Time t_, dt_;
            double xMin_, dx_, yMin_, dy_;
            Handle<ShortRateDynamics> dynamics_;
        };

    }

}
#endif
