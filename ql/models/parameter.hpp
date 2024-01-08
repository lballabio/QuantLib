/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file parameter.hpp
    \brief Model parameter classes
*/

#ifndef quantlib_interest_rate_modelling_parameter_hpp
#define quantlib_interest_rate_modelling_parameter_hpp

#include <ql/handle.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/qldefines.hpp>
#include <utility>
#include <vector>

namespace QuantLib {

    class YieldTermStructure;

    //! Base class for model arguments
    class Parameter {
      protected:
        //! Base class for model parameter implementation
        class Impl {
          public:
            virtual ~Impl() = default;
            virtual Real value(const Array& params, Time t) const = 0;
        };
        ext::shared_ptr<Impl> impl_;
      public:
        Parameter()
        : constraint_(NoConstraint()) {}
        const Array& params() const { return params_; }
        void setParam(Size i, Real x) { params_[i] = x; }
        bool testParams(const Array& params) const {
            return constraint_.test(params);
        }
        Size size() const { return params_.size(); }
        Real operator()(Time t) const {
            return impl_->value(params_, t);
        }
        const ext::shared_ptr<Impl>& implementation() const {
            return impl_;
        }
        const Constraint& constraint() const { return constraint_; }
      protected:
        Parameter(Size size, ext::shared_ptr<Impl> impl, Constraint constraint)
        : impl_(std::move(impl)), params_(size), constraint_(std::move(constraint)) {}
        Array params_;
        Constraint constraint_;
    };

    //! Standard constant parameter \f$ a(t) = a \f$
    class ConstantParameter : public Parameter {
      private:
        class Impl final : public Parameter::Impl {
          public:
            Real value(const Array& params, Time) const override { return params[0]; }
        };
      public:
        ConstantParameter(const Constraint& constraint)
        : Parameter(
              1,
              ext::shared_ptr<Parameter::Impl>(new ConstantParameter::Impl),
              constraint)
        {}

        ConstantParameter(Real value,
                          const Constraint& constraint)
        : Parameter(
              1,
              ext::shared_ptr<Parameter::Impl>(new ConstantParameter::Impl),
              constraint) {
            params_[0] = value;
            QL_REQUIRE(testParams(params_),
                       value << ": invalid value");
        }

    };

    //! %Parameter which is always zero \f$ a(t) = 0 \f$
    class NullParameter : public Parameter {
      private:
        class Impl final : public Parameter::Impl {
          public:
            Real value(const Array&, Time) const override { return 0.0; }
        };
      public:
        NullParameter()
        : Parameter(
                  0,
                  ext::shared_ptr<Parameter::Impl>(new NullParameter::Impl),
                  NoConstraint())
        {}
    };

    //! Piecewise-constant parameter
    /*! \f$ a(t) = a_i if t_{i-1} \geq t < t_i \f$.
        This kind of parameter is usually used to enhance the fitting of a
        model
    */
    class PiecewiseConstantParameter : public Parameter {
      private:
        class Impl final : public Parameter::Impl {
          public:
            explicit Impl(std::vector<Time> times) : times_(std::move(times)) {}

            Real value(const Array& params, Time t) const override {
                Size i = std::upper_bound(times_.begin(), times_.end(), t) - times_.begin();
                return params[i];
            }

          private:
            std::vector<Time> times_;
        };
      public:
        PiecewiseConstantParameter(const std::vector<Time>& times,
                                   const Constraint& constraint =
                                                             NoConstraint())
        : Parameter(times.size()+1,
                    ext::shared_ptr<Parameter::Impl>(
                                 new PiecewiseConstantParameter::Impl(times)),
                    constraint)
        {}
    };

    //! Deterministic time-dependent parameter used for yield-curve fitting
    class TermStructureFittingParameter : public Parameter {
      public:
        class NumericalImpl : public Parameter::Impl {
          public:
            NumericalImpl(Handle<YieldTermStructure> termStructure)
            : times_(0), values_(0), termStructure_(std::move(termStructure)) {}

            void set(Time t, Real x) {
                times_.push_back(t);
                values_.push_back(x);
            }
            void change(Real x) {
                values_.back() = x;
            }
            void reset() {
                times_.clear();
                values_.clear();
            }
            Real value(const Array&, Time t) const override {
                auto result = std::find(times_.begin(), times_.end(), t);
                QL_REQUIRE(result!=times_.end(),
                           "fitting parameter not set!");
                return values_[result - times_.begin()];
            }
            const Handle<YieldTermStructure>& termStructure() const {
                return termStructure_;
            }
          private:
            std::vector<Time> times_;
            std::vector<Real> values_;
            Handle<YieldTermStructure> termStructure_;
        };

        TermStructureFittingParameter(
                               const ext::shared_ptr<Parameter::Impl>& impl)
        : Parameter(0, impl, NoConstraint()) {}

        TermStructureFittingParameter(const Handle<YieldTermStructure>& term)
        : Parameter(
                  0,
                  ext::shared_ptr<Parameter::Impl>(new NumericalImpl(term)),
                  NoConstraint())
        {}
    };

}


#endif
