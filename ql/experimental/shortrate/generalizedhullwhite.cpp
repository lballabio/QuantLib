/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 SunTrust Bank
 Copyright (C) 2010 Cavit Hafizoglu

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

#include <ql/experimental/shortrate/generalizedhullwhite.hpp>
#include <ql/termstructures/interpolatedcurve.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/methods/lattices/trinomialtree.hpp>
#include <ql/math/solvers1d/brent.hpp>

namespace QuantLib {

    namespace {

        class PiecewiseLinearCurve : public InterpolatedCurve<Linear> {
          public:
            PiecewiseLinearCurve(const std::vector<Time>& times,
                                 const std::vector<Real>& data)
            : InterpolatedCurve<Linear>(times, data) {
                setupInterpolation();
            }

            Real operator()(Time t) {
                return interpolation_(t);
            }
        };

    }


    /* Private function used by solver to determine time-dependent parameter
       df(r) = [theta(t) - a(t) f(r)]dt + sigma(t) dz
       dg = [theta(t) - a(t) g(t)] dt
       dx = -a(t) x dt + sigma(t) dz
       x = f(r) - g(t)
    */
    // Change the overloaded operator to change the model by changing
    // the function below

    Real fInverse_(Real x) {
        return std::exp(x);
    }

    class GeneralizedHullWhite::Helper {
      public:
        Helper(const Size i, const Real xMin, const Real dx,
               const Real discountBondPrice,
               const boost::shared_ptr<ShortRateTree>& tree)
        : size_(tree->size(i)),
          dt_(tree->timeGrid().dt(i)),
          xMin_(xMin), dx_(dx),
          statePrices_(tree->statePrices(i)),
          discountBondPrice_(discountBondPrice){}

        Real operator()(const Real theta) const {
            Real value = discountBondPrice_;
            Real x = xMin_;
            for (Size j=0; j<size_; j++) {
                Real discount = std::exp(- fInverse_(theta+x)*dt_);
                value -= statePrices_[j]*discount;
                x += dx_;
            }

            return value;
        }

      private:
        Size size_;
        Time dt_;
        Real xMin_, dx_;
        const Array& statePrices_;
        Real discountBondPrice_;
    };

    GeneralizedHullWhite::GeneralizedHullWhite(
        const Handle<YieldTermStructure>& yieldtermStructure,
        const std::vector<Date>& speedstructure,
        const std::vector<Date>& volstructure)
    : OneFactorModel(2), TermStructureConsistentModel(yieldtermStructure),
      speedstructure_(speedstructure), volstructure_(volstructure),
      a_(arguments_[0]), sigma_(arguments_[1]) {

        DayCounter dc = yieldtermStructure->dayCounter();

        speedperiods_.push_back(0.0);
        for (Size i=0;i<speedstructure.size()-1;i++)
            speedperiods_.push_back(dc.yearFraction(speedstructure[0],
                                                    speedstructure[i+1]));

        a_ = PiecewiseConstantParameter(speedperiods_, PositiveConstraint());

        volperiods_.push_back(0.0);
        for (Size i=0;i<volstructure.size()-1;i++)
            volperiods_.push_back(dc.yearFraction(volstructure[0],
                                                  volstructure[i+1]));

        sigma_ = PiecewiseConstantParameter(volperiods_, PositiveConstraint());

        a_.setParam(0,0.1);
        sigma_.setParam(0,0.1);

        for (Size i=1; i< a_.size();i++){
            a_.setParam(i,0.1*i+0.01);
        }

        for (Size i=1; i< sigma_.size();i++){
            sigma_.setParam(i,0.1*i+0.01);
        }

        registerWith(yieldtermStructure);
    }

    GeneralizedHullWhite::GeneralizedHullWhite(
        const Handle<YieldTermStructure>& yieldtermStructure,
        const std::vector<Date>& speedstructure,
        const std::vector<Date>& volstructure,
        const std::vector<Real>& speed,
        const std::vector<Real>& vol)
  : OneFactorModel(2), TermStructureConsistentModel(yieldtermStructure),
    speedstructure_(speedstructure),
    volstructure_(volstructure),
    a_(arguments_[0]), sigma_(arguments_[1]) {

        DayCounter dc = yieldtermStructure->dayCounter();

        speedperiods_.push_back(0.0);
        for (Size i=0;i<speedstructure.size()-1;i++)
            speedperiods_.push_back(dc.yearFraction(speedstructure[0],
                                                    speedstructure[i+1]));

        a_ = PiecewiseConstantParameter(speedperiods_, PositiveConstraint());

        volperiods_.push_back(0.0);
        for (Size i=0;i<volstructure.size()-1;i++)
            volperiods_.push_back(dc.yearFraction(volstructure[0],
                                                  volstructure[i+1]));

        sigma_ = PiecewiseConstantParameter(volperiods_, PositiveConstraint());

        a_.setParam(0,speed[0]);
        sigma_.setParam(0,vol[0]);

        for (Size i=1; i< sigma_.size();i++) {
            sigma_.setParam(i,vol[i-1]);
        }
        for (Size i=1; i< a_.size();i++) {
            a_.setParam(i,speed[i-1]);
        }

        registerWith(yieldtermStructure);
    }

    boost::shared_ptr<Lattice> GeneralizedHullWhite::tree(
                                                  const TimeGrid& grid) const{

        TermStructureFittingParameter phi(termStructure());
        boost::shared_ptr<ShortRateDynamics> numericDynamics(
            new Dynamics(phi, speed(), vol()));
        boost::shared_ptr<TrinomialTree> trinomial(
            new TrinomialTree(numericDynamics->process(), grid));
        boost::shared_ptr<ShortRateTree> numericTree(
            new ShortRateTree(trinomial, numericDynamics, grid));
        typedef TermStructureFittingParameter::NumericalImpl NumericalImpl;
        boost::shared_ptr<NumericalImpl> impl =
            boost::dynamic_pointer_cast<NumericalImpl>(phi.implementation());

        impl->reset();
        Real value = 1.0;
        Real vMin = -50.0;
        Real vMax = 50.0;

        for (Size i=0; i<(grid.size() - 1); i++) {
            Real discountBond = termStructure()->discount(grid[i+1]);
            Real xMin = trinomial->underlying(i, 0);
            Real dx = trinomial->dx(i);
            Helper finder(i, xMin, dx, discountBond, numericTree);
            Brent s1d;
            s1d.setMaxEvaluations(1000);
            value = s1d.solve(finder, 1e-7, value, vMin, vMax);
            impl->set(grid[i], value);
        }

        return numericTree;
    }

    boost::function<Real (Time)> GeneralizedHullWhite::speed() const {

        std::vector<Real> speedvals;
        speedvals.push_back(a_(0.0001));
        for (Size i=0;i<a_.size()-1;i++)
            speedvals.push_back(
            a_(
            (speedstructure_[i+1]-speedstructure_[0])/365.0
            - 0.00001));

        return PiecewiseLinearCurve(speedperiods_, speedvals);
    }

    boost::function<Real (Time)> GeneralizedHullWhite::vol() const {

        std::vector<Real> volvals;
        volvals.push_back(sigma_(0.0001));
        for (Size i=0;i<sigma_.size()-1;i++)
            volvals.push_back(
            sigma_(
            (speedstructure_[i+1]-speedstructure_[0])/365.0
            - 0.00001));

        return PiecewiseLinearCurve(volperiods_, volvals);
    }

}
