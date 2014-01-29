/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 SunTrust Bank
 Copyright (C) 2010, 2014 Cavit Hafizoglu

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
#include <ql/pricingengines/blackformula.hpp>

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
                return interpolation_(t,true);
            }
        };

        class PiecewiseConstantParameter2 : public Parameter {
          private:
            class Impl : public Parameter::Impl {
              public:
                Impl(const std::vector<Time>& times)
                : times_(times) {}

                Real value(const Array& params, Time t) const {
                    Size size = times_.size();
                    for (Size i=0; i<size; i++) {
                        if (t<times_[i])
                            return params[i];
                    }
                    return params[size-1];
                }
              private:
                std::vector<Time> times_;
            };
          public:
            PiecewiseConstantParameter2(const std::vector<Time>& times,
                                        const Constraint& constraint =
                                                             NoConstraint())
            : Parameter(times.size(),
                        boost::shared_ptr<Parameter::Impl>(
                                new PiecewiseConstantParameter2::Impl(times)),
                        constraint)
            {}
        };

        Real identity(Real x) {
            return x;
        }

    }

    /* Private function used by solver to determine time-dependent parameter
       df(r) = [theta(t) - a(t) f(r)]dt + sigma(t) dz
       dg = [theta(t) - a(t) g(t)] dt
       dx = -a(t) x dt + sigma(t) dz
       x = f(r) - g(t)
    */
    class GeneralizedHullWhite::Helper {
      public:
        Helper(const Size i, const Real xMin, const Real dx,
               const Real discountBondPrice,
               const boost::shared_ptr<ShortRateTree>& tree,
               const boost::function<Real(Real)>& fInv)
        : size_(tree->size(i)),
          dt_(tree->timeGrid().dt(i)),
          xMin_(xMin), dx_(dx),
          statePrices_(tree->statePrices(i)),
          discountBondPrice_(discountBondPrice), fInverse_(fInv) {}

        Real operator()(const Real theta) const {
            Real value = discountBondPrice_;
            Real x = xMin_;
            for (Size j=0; j<size_; j++) {
                Real discount = std::exp(- fInverse_(theta+x)*dt_);
                value -= statePrices_[j]*discount;
                x += dx_;
            }

            return value;
        };

      private:
        Size size_;
        Time dt_;
        Real xMin_, dx_;
        const Array& statePrices_;
        Real discountBondPrice_;
        boost::function<Real(Real)> fInverse_;
    };


    GeneralizedHullWhite::GeneralizedHullWhite(
        const Handle<YieldTermStructure>& yieldtermStructure,
        const std::vector<Date>& speedstructure,
        const std::vector<Date>& volstructure,
        const boost::function<Real(Real)>& f,
        const boost::function<Real(Real)>& fInverse)
    : OneFactorAffineModel(2), TermStructureConsistentModel(yieldtermStructure),
      speedstructure_(speedstructure), volstructure_(volstructure),
      a_(arguments_[0]), sigma_(arguments_[1]),
      f_(f), fInverse_(fInverse) {

        if (f_.empty())
            f_ = identity;
        if (fInverse_.empty())
            fInverse_ = identity;

        DayCounter dc = yieldtermStructure->dayCounter();

        speedperiods_.push_back(0.0);
        for (Size i=0;i<speedstructure.size()-1;i++)
            speedperiods_.push_back(dc.yearFraction(speedstructure[0],
                                                    speedstructure[i+1]));

        a_ = PiecewiseConstantParameter2(speedperiods_, PositiveConstraint());

        volperiods_.push_back(0.0);
        for (Size i=0;i<volstructure.size()-1;i++)
            volperiods_.push_back(dc.yearFraction(volstructure[0],
                                                  volstructure[i+1]));

        sigma_ = PiecewiseConstantParameter2(volperiods_, PositiveConstraint());

        a_.setParam(0,0.001);
        sigma_.setParam(0,0.001);

        for (Size i=1; i< a_.size();i++){
            a_.setParam(i,0.01*i+0.07);
        }

        for (Size i=1; i< sigma_.size();i++){
            sigma_.setParam(i,0.01*i+0.01);
        }

        registerWith(yieldtermStructure);
    }

    GeneralizedHullWhite::GeneralizedHullWhite(
        const Handle<YieldTermStructure>& yieldtermStructure,
        const std::vector<Date>& speedstructure,
        const std::vector<Date>& volstructure,
        const std::vector<Real>& speed,
        const std::vector<Real>& vol,
        const boost::function<Real(Real)>& f,
        const boost::function<Real(Real)>& fInverse)
    : OneFactorAffineModel(2), TermStructureConsistentModel(yieldtermStructure),
      speedstructure_(speedstructure),
      volstructure_(volstructure),
      a_(arguments_[0]), sigma_(arguments_[1]),
      f_(f), fInverse_(fInverse) {

        if (f_.empty())
            f_ = identity;
        if (fInverse_.empty())
            fInverse_ = identity;

        DayCounter dc = yieldtermStructure->dayCounter();
        Date ref = yieldtermStructure->referenceDate();

        for (Size i=0;i<speedstructure.size();i++)
            speedperiods_.push_back(dc.yearFraction(ref,
                                                    speedstructure[i]));

        a_ = PiecewiseConstantParameter2(speedperiods_, PositiveConstraint());

        for (Size i=0;i<volstructure.size();i++)
            volperiods_.push_back(dc.yearFraction(ref,
                                                  volstructure[i]));

        sigma_ = PiecewiseConstantParameter2(volperiods_, PositiveConstraint());

        for (Size i=0; i< sigma_.size();i++) {
            sigma_.setParam(i,vol[i]);
        }
        for (Size i=0; i< a_.size();i++) {
            a_.setParam(i,speed[i]);
        }

        registerWith(yieldtermStructure);
    }

    //classical HW
    GeneralizedHullWhite::GeneralizedHullWhite(
        const Handle<YieldTermStructure>& yieldtermStructure,
        Real a, Real sigma)
    : OneFactorAffineModel(2),
      TermStructureConsistentModel(yieldtermStructure),
      a_(arguments_[0]),
      sigma_(arguments_[1]) {

        a_ = ConstantParameter(a, PositiveConstraint());
        sigma_ = ConstantParameter(sigma, PositiveConstraint());
        a_.setParam(0,a);
        sigma_.setParam(0,sigma);

        f_ = identity;
        fInverse_ = identity;

        Date ref = yieldtermStructure->referenceDate();

        speedperiods_.push_back(0.0);
        volperiods_.push_back(0.0);
        speedstructure_.push_back(ref);
        volstructure_.push_back(ref);

        generateArguments();

        registerWith(yieldtermStructure);
    }

    Real GeneralizedHullWhite::B(Time t, Time T) const {
        Real _a = a();
        if (_a < std::sqrt(QL_EPSILON))
            return (T - t);
        else
            return (1.0 - std::exp(-_a*(T - t)))/_a;
    }

    void GeneralizedHullWhite::generateArguments() {
        phi_ = FittingParameter(termStructure(), a(), sigma());
    }

    Real GeneralizedHullWhite::discountBondOption(Option::Type type, Real strike,
                                                  Time maturity,
                                                  Time bondMaturity) const {

        Real _a = a();
        Real v;
        if (_a < std::sqrt(QL_EPSILON)) {
            v = sigma()*B(maturity, bondMaturity)* std::sqrt(maturity);
        } else {
            v = sigma()*B(maturity, bondMaturity)*
                std::sqrt(0.5*(1.0 - std::exp(-2.0*_a*maturity))/_a);
        }
        Real f = termStructure()->discount(bondMaturity);
        Real k = termStructure()->discount(maturity)*strike;

        return blackFormula(type, k, f, v);
    }

    Real GeneralizedHullWhite::A(Time t, Time T) const {
        DiscountFactor discount1 = termStructure()->discount(t);
        DiscountFactor discount2 = termStructure()->discount(T);
        Rate forward = termStructure()->forwardRate(t, t,
                                                    Continuous, NoFrequency);
        Real temp = sigma()*B(t,T);
        Real value = B(t,T)*forward - 0.25*temp*temp*B(0.0,2.0*t);
        return std::exp(value)*discount2/discount1;
    }


    boost::shared_ptr<Lattice> GeneralizedHullWhite::tree(
                                                  const TimeGrid& grid) const{

        TermStructureFittingParameter phi(termStructure());
        boost::shared_ptr<ShortRateDynamics> numericDynamics(
            new Dynamics(phi, speed(), vol(), f_, fInverse_));
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
            Helper finder(i, xMin, dx, discountBond, numericTree, fInverse_);
            Brent s1d;
            s1d.setMaxEvaluations(2000);
            value =s1d.solve(finder, 1e-8, value, vMin, vMax);
            impl->set(grid[i], value);
        }

        return numericTree;
    }

    boost::function<Real (Time)> GeneralizedHullWhite::speed() const {

        std::vector<Real> speedvals;
        speedvals.push_back(a_(0.001));

        if (a_.size()==1) {
            std::vector<Time> speedper;
            speedper.push_back(0.0);
            speedper.push_back(200); // 200 years to match the
                                     // constant a and sigma case
            speedvals.push_back(a_(0.001));
            return PiecewiseLinearCurve(speedper, speedvals);
        }

        for (Size i=0;i<a_.size()-1;i++)
            speedvals.push_back(
            a_(
            (speedstructure_[i+1]- Settings::instance().evaluationDate() )/365.0
            - 0.001));

        return PiecewiseLinearCurve(speedperiods_, speedvals);
    }

    boost::function<Real (Time)> GeneralizedHullWhite::vol() const {

        std::vector<Real> volvals;
        volvals.push_back(sigma_(0.001));

        if (sigma_.size()==1) {
            std::vector<Time> volper;
            volper.push_back(0.0);
            volper.push_back(200); //200 years to approximate the
                                   //constant coefficient model
            volvals.push_back(sigma_(0.001));
            return PiecewiseLinearCurve(volper, volvals);
        }

        for (Size i=0;i<sigma_.size()-1;i++)
            volvals.push_back(
            sigma_(
            (speedstructure_[i+1]-Settings::instance().evaluationDate() )/365.0
            - 0.001));

        return PiecewiseLinearCurve(volperiods_, volvals);
    }

}
