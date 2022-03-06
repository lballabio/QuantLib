/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file twofactormodel.hpp
    \brief Abstract two-factor interest rate model class
*/

#ifndef quantlib_two_factor_model_hpp
#define quantlib_two_factor_model_hpp

#include <ql/methods/lattices/lattice2d.hpp>
#include <ql/models/model.hpp>
#include <utility>

namespace QuantLib {
    class StochasticProcess1D;
    class StochasticProcess;
    //! Abstract base-class for two-factor models
    /*! \ingroup shortrate */
    class TwoFactorModel : public ShortRateModel {
      public:
        explicit TwoFactorModel(Size nParams);

        class ShortRateDynamics;
        class ShortRateTree;

        //! Returns the short-rate dynamics
        virtual ext::shared_ptr<ShortRateDynamics> dynamics() const = 0;

        //! Returns a two-dimensional trinomial tree
        ext::shared_ptr<Lattice> tree(const TimeGrid& grid) const override;
    };

    //! Class describing the dynamics of the two state variables
    /*! We assume here that the short-rate is a function of two state
        variables x and y.
        \f[
            r_t = f(t, x_t, y_t)
        \f]
        of two state variables \f$ x_t \f$ and \f$ y_t \f$. These stochastic
        processes satisfy
        \f[
            x_t = \mu_x(t, x_t)dt + \sigma_x(t, x_t) dW_t^x
        \f]
        and
        \f[
            y_t = \mu_y(t,y_t)dt + \sigma_y(t, y_t) dW_t^y
        \f]
        where \f$ W^x \f$ and \f$ W^y \f$ are two brownian motions
        satisfying
        \f[
            dW^x_t dW^y_t = \rho dt
        \f].
    */
    class TwoFactorModel::ShortRateDynamics {
      public:
        ShortRateDynamics(ext::shared_ptr<StochasticProcess1D> xProcess,
                          ext::shared_ptr<StochasticProcess1D> yProcess,
                          Real correlation)
        : xProcess_(std::move(xProcess)), yProcess_(std::move(yProcess)),
          correlation_(correlation) {}
        virtual ~ShortRateDynamics() = default;

        virtual Rate shortRate(Time t, Real x, Real y) const = 0;

        //! Risk-neutral dynamics of the first state variable x
        const ext::shared_ptr<StochasticProcess1D>& xProcess() const {
            return xProcess_;
        }

        //! Risk-neutral dynamics of the second state variable y
        const ext::shared_ptr<StochasticProcess1D>& yProcess() const {
            return yProcess_;
        }

        //! Correlation \f$ \rho \f$ between the two brownian motions.
        Real correlation() const {
            return correlation_;
        }

        //! Joint process of the two variables
        ext::shared_ptr<StochasticProcess> process() const;

      private:
        ext::shared_ptr<StochasticProcess1D> xProcess_, yProcess_;
        Real correlation_;
    };

    //! Recombining two-dimensional tree discretizing the state variable
    class TwoFactorModel::ShortRateTree
        : public TreeLattice2D<TwoFactorModel::ShortRateTree,TrinomialTree> {
      public:
        //! Plain tree build-up from short-rate dynamics
        ShortRateTree(const ext::shared_ptr<TrinomialTree>& tree1,
                      const ext::shared_ptr<TrinomialTree>& tree2,
                      const ext::shared_ptr<ShortRateDynamics>& dynamics);

        DiscountFactor discount(Size i, Size index) const {
            Size modulo = tree1_->size(i);
            Size index1 = index % modulo;
            Size index2 = index / modulo;

            Real x = tree1_->underlying(i, index1);
            Real y = tree2_->underlying(i, index2);

            Rate r = dynamics_->shortRate(timeGrid()[i], x, y);
            return std::exp(-r*timeGrid().dt(i));
        }
      private:
        ext::shared_ptr<ShortRateDynamics> dynamics_;
    };

}


#endif


#ifndef id_e1d53870410e5481b78b960d3c2348b4
#define id_e1d53870410e5481b78b960d3c2348b4
inline bool test_e1d53870410e5481b78b960d3c2348b4(const int* i) {
    return i != nullptr;
}
#endif
