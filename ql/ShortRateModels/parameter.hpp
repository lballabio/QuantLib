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
/*! \file parameter.hpp
    \brief Model parameter classes

    \fullpath
    ql/ShortRateModels/%parameter.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_parameter_h
#define quantlib_interest_rate_modelling_parameter_h

#include <ql/qldefines.hpp>
#include <ql/array.hpp>
#include <ql/termstructure.hpp>
#include <ql/Optimization/constraint.hpp>

namespace QuantLib {

    namespace ShortRateModels {

        class Parameter {
          public:
            class ParameterImpl {
              public:
                virtual double value(const Array& params, Time t) const = 0;
            };

            Parameter() : constraint_(Optimization::NoConstraint()) {}
            Parameter(Size size, 
                      const Handle<ParameterImpl>& impl, 
                      const Optimization::Constraint& constraint)
            : params_(size), impl_(impl), constraint_(constraint) {}
            virtual ~Parameter() {}

            const Array& params() const { return params_; }
            void setParam(Size i, double x) { params_[i] = x; }
            bool testParams(const Array& params) const { 
                return constraint_.test(params);
            }
            virtual Size size() const { return params_.size(); }

            double operator()(Time t) const { return impl_->value(params_, t); }

            const Handle<ParameterImpl>& implementation() const {return impl_;}

          protected:
            Array params_;
            Handle<ParameterImpl> impl_;
            Optimization::Constraint constraint_;
        };

        class ConstantParameter : public Parameter {
          public:
            class ConstantParameterImpl : public Parameter::ParameterImpl {
              public:
                double value(const Array& params, Time t) const {
                    return params[0];
                }
            };

            ConstantParameter(
                const Optimization::Constraint& constraint) 
            : Parameter(1, 
                        Handle<ParameterImpl>(new ConstantParameterImpl), 
                        constraint) 
            {}

            ConstantParameter(
                double value,
                const Optimization::Constraint& constraint) 
            : Parameter(1, 
                        Handle<ParameterImpl>(new ConstantParameterImpl), 
                        constraint) {
                params_[0] = value;
                QL_REQUIRE(testParams(params_), 
                           "ConstantParameter: invalid value in constructor");
            }

        };

        class NullParameter : public Parameter {
          public:
            class NullParameterImpl : public Parameter::ParameterImpl {
              public:
                double value(const Array& params, Time t) const {
                    return 0.0;
                }
            };

            NullParameter() 
            : Parameter(0, 
                        Handle<ParameterImpl>(new NullParameterImpl), 
                        Optimization::NoConstraint()) 
            {}
        };

        class PiecewiseConstantParameter : public Parameter {
          public:
            class PiecewiseConstantParameterImpl : public Parameter::ParameterImpl {
              public:
                PiecewiseConstantParameterImpl(const std::vector<Time>& times)
                : times_(times) {}
                virtual ~PiecewiseConstantParameterImpl() {}

                double value(const Array& params, Time t) const {
                    Size size = times_.size();
                    for (Size i=0; i<size; i++) {
                        if (t<times_[i])
                            return params[i];
                    }
                    return params[size];
                }
              private:
                std::vector<Time> times_;
            };

            PiecewiseConstantParameter(const std::vector<Time>& times) 
            : Parameter(times.size()+1, 
                        Handle<ParameterImpl>(
                            new PiecewiseConstantParameterImpl(times)),
                        Optimization::NoConstraint()) 
            {}
        };

        class TermStructureFittingParameter : public Parameter {
          public:
            class NumericalImpl : public Parameter::ParameterImpl {
              public:
                NumericalImpl(
                    const RelinkableHandle<TermStructure>& termStructure)
                : times_(0), values_(0), termStructure_(termStructure) {}
                virtual ~NumericalImpl() {}

                void set(Time t, double x) {
                    times_.push_back(t);
                    values_.push_back(x);
                }
                void change(double x) {
                    values_.back() = x;
                }
                void reset() {
                    times_.clear();
                    values_.clear();
                }
                double value(const Array& params, Time t) const {
                    std::vector<Time>::const_iterator result = 
                        std::find(times_.begin(), times_.end(), t);
                    QL_REQUIRE(result!=times_.end(),"Fitting parameter unset!");
                    return values_[result - times_.begin()];
                }
                const RelinkableHandle<TermStructure>& termStructure() const {
                    return termStructure_;
                }
              private:
                std::vector<Time> times_;
                std::vector<double> values_;
                RelinkableHandle<TermStructure> termStructure_;
            };

            TermStructureFittingParameter(const Handle<ParameterImpl>& impl)
            : Parameter(0, impl, Optimization::NoConstraint()) {}

            TermStructureFittingParameter(
                const RelinkableHandle<TermStructure>& term)
            : Parameter(0, Handle<ParameterImpl>(new NumericalImpl(term)),
                        Optimization::NoConstraint())
            {} 
        };


    }

}


#endif
