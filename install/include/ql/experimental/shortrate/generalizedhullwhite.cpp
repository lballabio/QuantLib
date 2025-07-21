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
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/methods/lattices/trinomialtree.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        // integral of mean reversion
        Real integrateMeanReversion(const Interpolation &a,Real t,Real T) {
            if ((T-t) < QL_EPSILON)
                return 0.0;
            SimpsonIntegral integrator(1e-5, 1000);
            Real mr = integrator(a,t,T);
            return mr;
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
        Helper(const Size i,
               const Real xMin,
               const Real dx,
               const Real discountBondPrice,
               const ext::shared_ptr<ShortRateTree>& tree,
               std::function<Real(Real)> fInv)
        : size_(tree->size(i)), dt_(tree->timeGrid().dt(i)), xMin_(xMin), dx_(dx),
          statePrices_(tree->statePrices(i)), discountBondPrice_(discountBondPrice),
          fInverse_(std::move(fInv)) {}

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
        std::function<Real(Real)> fInverse_;
    };

    GeneralizedHullWhite::GeneralizedHullWhite(
        const Handle<YieldTermStructure>& yieldtermStructure,
        const std::vector<Date>& speedstructure,
        const std::vector<Date>& volstructure,
        const std::vector<Real>& speed,
        const std::vector<Real>& vol,
        const std::function<Real(Real)>& f,
        const std::function<Real(Real)>& fInverse)
    : OneFactorAffineModel(2), TermStructureConsistentModel(yieldtermStructure),
      speedstructure_(speedstructure),
      volstructure_(volstructure),
      a_(arguments_[0]), sigma_(arguments_[1]),
      f_(f), fInverse_(fInverse) {

        LinearFlat traits;
        initialize(yieldtermStructure,speedstructure,volstructure,
          speed, vol, traits, traits, f, fInverse);
    }

    //classical HW
    GeneralizedHullWhite::GeneralizedHullWhite(
        const Handle<YieldTermStructure>& yieldtermStructure,
        Real a, Real sigma)
    : OneFactorAffineModel(2),
      TermStructureConsistentModel(yieldtermStructure),
      a_(arguments_[0]),
      sigma_(arguments_[1])
    {
        Date ref = yieldtermStructure->referenceDate();
        std::vector<Date> speedstructure,volstructure;
        std::vector<Real> _a, _sigma;
        _a.push_back(a);
        _sigma.push_back(sigma);
        speedstructure.push_back(ref);
        volstructure.push_back(ref);
        BackwardFlat traits;
        initialize(yieldtermStructure,speedstructure,volstructure,
            _a, _sigma, traits, traits, identity, identity);
    }

    void GeneralizedHullWhite::generateArguments() {
        speed_.update();
        vol_.update();
        phi_ = FittingParameter(termStructure(), a(), sigma());
    }

    Real GeneralizedHullWhite::B(Time t, Time T) const {
        // Gurrieri et al, equations (30) and (31)
        Real lnEt = integrateMeanReversion(speed_,0,t);
        Real Et = exp(lnEt);
        Real B = 0;
        Size N = std::min<Size>(Size((T-t)*365), 2000);
        if (N==0) N=1;
        Real dt = 0.5*(T-t)/N;
        Real a,b,c,_t,total=0;
        _t = t;
        c = speed_(_t);
        _t += dt;
        for (Size i=0; i<N; i++) {
            a = c;
            b = speed_(_t);
            c = speed_(_t+dt);
            total += (dt*(2.0/6.0))*(a+4*b+c);
            B += (2*dt) / exp(lnEt+total);
            _t += 2*dt;
        }
        B *= Et;
        return B;
    }

    Real GeneralizedHullWhite::V(Time t, Time T) const {
        // Gurrieri et al, equation (37)
        Real lnEt = integrateMeanReversion(speed_,0,t);
        Real V = 0,Eu;
        Size N = std::min<Size>(Size((T-t)*365), 2000);
        if (N==0) N=1;
        Real dt = 0.5*(T-t)/N;
        Real a,b,c,_t,lnE=lnEt;
        _t = t;
        Real vol = vol_(_t);
        Eu = exp(lnE);
        c = Eu*Eu*vol*vol;
        _t += dt;
        for (Size i=0; i<N; i++) {
            a = c;
            vol = vol_(_t);
            lnE += speed_(_t)*dt;
            Eu = exp(lnE);
            b = Eu*Eu*vol*vol;
            vol = vol_(_t+dt);
            lnE += speed_(_t+dt)*dt;
            Eu = exp(lnE);
            c = Eu*Eu*vol*vol;
            V += (dt*(2.0/6.0))*(a+4*b+c);
            _t += 2*dt;
        }
        return V / (Eu*Eu);
    }

    Real GeneralizedHullWhite::discountBondOption(Option::Type type, Real strike,
                                                  Time maturity,
                                                  Time bondMaturity) const
    {
        /*
        Hull-White bond option pricing with time varying sigma and mean reversion.
        Based on Gurrieri, Nakabayashi & Wong (2009) "Calibration Methods of
        Hull-White Model", https://ssrn.com/abstract=1514192
        */
        Real BtT = B(maturity,bondMaturity);
        Real Vr = V(0,maturity);
        Real Vp = Vr*BtT*BtT;
        Real vol = sqrt(Vp);
        Real f = termStructure()->discount(bondMaturity);
        Real k = termStructure()->discount(maturity)*strike;
        return blackFormula(type, k, f, vol);
    }

    Real GeneralizedHullWhite::A(Time t, Time T) const {
        // Gurrieri et al, equation (43)
        DiscountFactor discount1 = termStructure()->discount(t);
        DiscountFactor discount2 = termStructure()->discount(T);
        Rate forward = termStructure()->forwardRate(t, t, Continuous, NoFrequency);
        Real BtT = B(t,T);
        Real Vr = V(0,t);
        Real AtT = log(discount2/discount1) + BtT*forward - 0.5*BtT*BtT*Vr;
        return exp(AtT);
    }


    ext::shared_ptr<Lattice> GeneralizedHullWhite::tree(
                                                  const TimeGrid& grid) const{

        TermStructureFittingParameter phi(termStructure());
        ext::shared_ptr<ShortRateDynamics> numericDynamics(
            new Dynamics(phi, speed(), vol(), f_, fInverse_));
        ext::shared_ptr<TrinomialTree> trinomial(
            new TrinomialTree(numericDynamics->process(), grid));
        ext::shared_ptr<ShortRateTree> numericTree(
            new ShortRateTree(trinomial, numericDynamics, grid));
        typedef TermStructureFittingParameter::NumericalImpl NumericalImpl;
        ext::shared_ptr<NumericalImpl> impl =
            ext::dynamic_pointer_cast<NumericalImpl>(phi.implementation());

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

    std::function<Real (Time)> GeneralizedHullWhite::speed() const {
        return speed_;
    }

    std::function<Real (Time)> GeneralizedHullWhite::vol() const {
        return vol_;
    }

    //! vector to pass to 'calibrate' to fit only volatility
    std::vector<bool> GeneralizedHullWhite::fixedReversion() const {
        Size na = a_.params().size();
        Size nsigma = sigma_.params().size();
        std::vector<bool> fixr(na+nsigma,false);
        std::fill(fixr.begin(),fixr.begin()+na,true);
        return fixr;
    }

}
