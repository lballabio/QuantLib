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

/*! \file generalizedhullwhite.hpp
    \brief generalized Hull-White model
*/

#ifndef quantlib_generalized_hull_white_hpp
#define quantlib_generalized_hull_white_hpp

#include <ql/experimental/shortrate/generalizedornsteinuhlenbeckprocess.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/models/shortrate/onefactormodel.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>
#include <utility>

namespace QuantLib {

    //! Parameter that holds an interpolation object
    class InterpolationParameter : public Parameter {
    private:
        class Impl : public Parameter::Impl {
        public:
          Real value(const Array&, Time t) const override { return interpolator_(t); }
          void reset(const Interpolation& interp) { interpolator_ = interp; }
        private:
            Interpolation interpolator_;
        };
    public:
        explicit InterpolationParameter(
            Size count,
            const Constraint& constraint = NoConstraint())
        : Parameter(count,
            ext::shared_ptr<Parameter::Impl>(
                new InterpolationParameter::Impl()),
                constraint)
        { }
        void reset(const Interpolation &interp) {
            ext::shared_ptr<InterpolationParameter::Impl> impl =
                ext::dynamic_pointer_cast<InterpolationParameter::Impl>(impl_);
            if (impl != nullptr)
                impl->reset(interp);
        }
    };

    //! Generalized Hull-White model class.
    /*! This class implements the standard Black-Karasinski model defined by
        \f[
        d f(r_t) = (\theta(t) - \alpha f(r_t))dt + \sigma dW_t,
        \f]
        where \f$ alpha \f$ and \f$ sigma \f$ are piecewise linear functions.

        \ingroup shortrate
    */
    class GeneralizedHullWhite : public OneFactorAffineModel,
                                 public TermStructureConsistentModel {
      public:

        GeneralizedHullWhite(
            const Handle<YieldTermStructure>& yieldtermStructure,
            const std::vector<Date>& speedstructure,
            const std::vector<Date>& volstructure,
            const std::vector<Real>& speed,
            const std::vector<Real>& vol,
            const ext::function<Real(Real)>& f =
                                            ext::function<Real(Real)>(),
            const ext::function<Real(Real)>& fInverse =
                                            ext::function<Real(Real)>());

        template <class SpeedInterpolationTraits,class VolInterpolationTraits>
        GeneralizedHullWhite(
            const Handle<YieldTermStructure>& yieldtermStructure,
            const std::vector<Date>& speedstructure,
            const std::vector<Date>& volstructure,
            const std::vector<Real>& speed,
            const std::vector<Real>& vol,
            const SpeedInterpolationTraits &speedtraits,
            const VolInterpolationTraits &voltraits,
            const ext::function<Real(Real)>& f =
                                            ext::function<Real(Real)>(),
            const ext::function<Real(Real)>& fInverse =
                                            ext::function<Real(Real)>()) :
            OneFactorAffineModel(2), TermStructureConsistentModel(yieldtermStructure),
            speedstructure_(speedstructure), volstructure_(volstructure),
            a_(arguments_[0]), sigma_(arguments_[1]),
            f_(f), fInverse_(fInverse)
        {
            initialize(yieldtermStructure,speedstructure,volstructure,
                speed,vol,speedtraits,voltraits,f,fInverse);
        }

        ext::shared_ptr<ShortRateDynamics> dynamics() const override {
            QL_FAIL("no defined process for generalized Hull-White model, "
                    "use HWdynamics()");
        }

        ext::shared_ptr<Lattice> tree(const TimeGrid& grid) const override;

        //Analytical calibration of HW

        GeneralizedHullWhite(
                  const Handle<YieldTermStructure>& yieldtermStructure,
                  Real a = 0.1, Real sigma = 0.01);


        ext::shared_ptr<ShortRateDynamics> HWdynamics() const;

        //! Only valid under Hull-White model
        Real discountBondOption(Option::Type type,
                                Real strike,
                                Time maturity,
                                Time bondMaturity) const override;

        //! vector to pass to 'calibrate' to fit only volatility
        std::vector<bool> fixedReversion() const;

      protected:
        //Analytical calibration of HW
        Real a() const { return a_(0.0); }
        Real sigma() const { return sigma_(0.0); }
        void generateArguments() override;
        Real A(Time t, Time T) const override;
        Real B(Time t, Time T) const override;
        Real V(Time t, Time T) const;

      private:

        class Dynamics;
        class Helper;
        class FittingParameter;// for analytic HW fitting

        std::vector<Date> speedstructure_;
        std::vector<Date> volstructure_;
        std::vector<Time> speedperiods_;
        std::vector<Time> volperiods_;
        Interpolation speed_;
        Interpolation vol_;

        ext::function<Real (Time)> speed() const;
        ext::function<Real (Time)> vol() const;

        Parameter& a_;
        Parameter& sigma_;
        Parameter phi_;

        ext::function<Real(Real)> f_;
        ext::function<Real(Real)> fInverse_;

        static Real identity(Real x) {
            return x;
        }

        template <class SpeedInterpolationTraits,class VolInterpolationTraits>
        void initialize(const Handle<YieldTermStructure>& yieldtermStructure,
            const std::vector<Date>& speedstructure,
            const std::vector<Date>& volstructure,
            const std::vector<Real>& speed,
            const std::vector<Real>& vol,
            const SpeedInterpolationTraits &speedtraits,
            const VolInterpolationTraits &voltraits,
            const ext::function<Real(Real)>& f,
            const ext::function<Real(Real)>& fInverse)
        {
            QL_REQUIRE(speedstructure.size()==speed.size(),
                "mean reversion inputs inconsistent");
            QL_REQUIRE(volstructure.size()==vol.size(),
                "volatility inputs inconsistent");
            if (!f_)
                f_ = identity;
            if (!fInverse_)
                fInverse_ = identity;

            DayCounter dc = yieldtermStructure->dayCounter();
            Date ref = yieldtermStructure->referenceDate();
            for (auto i : speedstructure)
                speedperiods_.push_back(dc.yearFraction(ref, i));
            for (auto i : volstructure)
                volperiods_.push_back(dc.yearFraction(ref, i));

            // interpolator x points to *periods_ vector, y points to
            // the internal Array in the parameter
            InterpolationParameter atemp(speedperiods_.size(), NoConstraint());
            a_ = atemp;
            for (Size i=0; i<speedperiods_.size(); i++)
                a_.setParam(i, speed[i]);
            speed_ = speedtraits.interpolate(speedperiods_.begin(),
                speedperiods_.end(),a_.params().begin());
            speed_.enableExtrapolation();
            atemp.reset(speed_);

            InterpolationParameter sigmatemp(volperiods_.size(), PositiveConstraint());
            sigma_ = sigmatemp;
            for (Size i=0; i<volperiods_.size(); i++)
                sigma_.setParam(i, vol[i]);
            vol_ = voltraits.interpolate(volperiods_.begin(),
                volperiods_.end(),sigma_.params().begin());
            vol_.enableExtrapolation();
            sigmatemp.reset(vol_);

            generateArguments();
            registerWith(yieldtermStructure);
        }
    };

    //! Short-rate dynamics in the generalized Hull-White model
    /*! The short-rate is here

        f(r_t) = x_t + g(t)

        where g is the deterministic time-dependent
        parameter (which can't be determined analytically)
        used for initial term-structure fitting and  x_t is the state
        variable following an Ornstein-Uhlenbeck process.

        In this version, the function f may also be defined as a piece-wise linear
        function and can be calibrated to the away-from-the-money instruments.

    */
    class GeneralizedHullWhite::Dynamics
        : public GeneralizedHullWhite::ShortRateDynamics {
      public:
        Dynamics(Parameter fitting,
                 const ext::function<Real(Time)>& alpha,
                 const ext::function<Real(Time)>& sigma,
                 ext::function<Real(Real)> f,
                 ext::function<Real(Real)> fInverse)
        : ShortRateDynamics(ext::shared_ptr<StochasticProcess1D>(
              new GeneralizedOrnsteinUhlenbeckProcess(alpha, sigma))),
          fitting_(std::move(fitting)), _f_(std::move(f)), _fInverse_(std::move(fInverse)) {}

        //classical HW dynamics
        Dynamics(Parameter fitting, Real a, Real sigma)
        : GeneralizedHullWhite::ShortRateDynamics(
              ext::shared_ptr<StochasticProcess1D>(new OrnsteinUhlenbeckProcess(a, sigma))),
          fitting_(std::move(fitting)), _f_(identity()), _fInverse_(identity()) {}

        Real variable(Time t, Rate r) const override { return _f_(r) - fitting_(t); }

        Real shortRate(Time t, Real x) const override { return _fInverse_(x + fitting_(t)); }

      private:
        Parameter fitting_;
        ext::function<Real(Real)> _f_;
        ext::function<Real(Real)> _fInverse_;
        struct identity {
            Real operator()(Real x) const {return x;};
        };
    };

    //! Analytical term-structure fitting parameter \f$ \varphi(t) \f$.
    /*! \f$ \varphi(t) \f$ is analytically defined by
        \f[
            \varphi(t) = f(t) + \frac{1}{2}[\frac{\sigma(1-e^{-at})}{a}]^2,
        \f]
        where \f$ f(t) \f$ is the instantaneous forward rate at \f$ t \f$.
    */
    class GeneralizedHullWhite::FittingParameter
        : public TermStructureFittingParameter {
      private:
        class Impl : public Parameter::Impl {
          public:
            Impl(Handle<YieldTermStructure> termStructure, Real a, Real sigma)
            : termStructure_(std::move(termStructure)), a_(a), sigma_(sigma) {}

            Real value(const Array&, Time t) const override {
                Rate forwardRate =
                    termStructure_->forwardRate(t, t, Continuous, NoFrequency);
                Real temp = a_ < std::sqrt(QL_EPSILON) ?
                            Real(sigma_*t) :
                            Real(sigma_*(1.0 - std::exp(-a_*t))/a_);
                return (forwardRate + 0.5*temp*temp);
            }

          private:
            Handle<YieldTermStructure> termStructure_;
            Real a_, sigma_;
        };
      public:
        FittingParameter(const Handle<YieldTermStructure>& termStructure,
                         Real a, Real sigma)
        : TermStructureFittingParameter(ext::shared_ptr<Parameter::Impl>(
                      new FittingParameter::Impl(termStructure, a, sigma))) {}
    };

    // Analytic fitting dynamics
    inline ext::shared_ptr<OneFactorModel::ShortRateDynamics>
    GeneralizedHullWhite::HWdynamics() const {
        return ext::shared_ptr<ShortRateDynamics>(
          new Dynamics(phi_, a(), sigma()));
    }

    namespace detail {
        template <class I1, class I2>
        class LinearFlatInterpolationImpl;
    }

    //! %Linear interpolation between discrete points with flat extapolation
    /*! \ingroup interpolations */
    class LinearFlatInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        LinearFlatInterpolation(const I1& xBegin, const I1& xEnd,
                            const I2& yBegin) {
            impl_ = ext::shared_ptr<Interpolation::Impl>(new
                detail::LinearFlatInterpolationImpl<I1,I2>(xBegin, xEnd,
                                                       yBegin));
            impl_->update();
        }
    };

    //! %Linear-interpolation with flat-extrapolation factory and traits
    /*! \ingroup interpolations */
    class LinearFlat {
      public:
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return LinearFlatInterpolation(xBegin, xEnd, yBegin);
        }
        static const bool global = false;
        static const Size requiredPoints = 1;
    };

    namespace detail {
        template <class I1, class I2>
        class LinearFlatInterpolationImpl
            : public Interpolation::templateImpl<I1,I2> {
          public:
            LinearFlatInterpolationImpl(const I1& xBegin, const I1& xEnd,
                                    const I2& yBegin)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin,
                                    LinearFlat::requiredPoints),
              primitiveConst_(xEnd-xBegin), s_(xEnd-xBegin) {}
            void update() override {
                primitiveConst_[0] = 0.0;
                for (Size i=1; i<Size(this->xEnd_-this->xBegin_); ++i) {
                    Real dx = this->xBegin_[i]-this->xBegin_[i-1];
                    s_[i-1] = (this->yBegin_[i]-this->yBegin_[i-1])/dx;
                    primitiveConst_[i] = primitiveConst_[i-1]
                        + dx*(this->yBegin_[i-1] +0.5*dx*s_[i-1]);
                }
            }
            Real value(Real x) const override {
                if (x <= this->xMin())
                    return this->yBegin_[0];
                if (x >= this->xMax())
                    return *(this->yBegin_+(this->xEnd_-this->xBegin_)-1);
                Size i = this->locate(x);
                return this->yBegin_[i] + (x-this->xBegin_[i])*s_[i];
            }
            Real primitive(Real x) const override {
                Size i = this->locate(x);
                Real dx = x-this->xBegin_[i];
                return primitiveConst_[i] +
                    dx*(this->yBegin_[i] + 0.5*dx*s_[i]);
            }
            Real derivative(Real x) const override {
                if (!this->isInRange(x))
                    return 0;
                Size i = this->locate(x);
                return s_[i];
            }
            Real secondDerivative(Real) const override { return 0.0; }

          private:
            std::vector<Real> primitiveConst_, s_;
        };
    }

}


#endif
