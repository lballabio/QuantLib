/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005, 2007 StatPro Italia srl
 Copyright (C) 2013, 2015 Peter Caspers

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

/*! \file model.hpp
    \brief Abstract interest rate model class
*/

#ifndef quantlib_interest_rate_model_hpp
#define quantlib_interest_rate_model_hpp

#include <ql/math/optimization/endcriteria.hpp>
#include <ql/methods/lattices/lattice.hpp>
#include <ql/models/calibrationhelper.hpp>
#include <ql/models/parameter.hpp>
#include <ql/option.hpp>
#include <utility>

namespace QuantLib {

    class OptimizationMethod;

    //! Affine model class
    /*! Base class for analytically tractable models.

        \ingroup shortrate
    */
    class AffineModel : public virtual Observable {
      public:
        //! Implied discount curve
        virtual DiscountFactor discount(Time t) const = 0;

        virtual Real discountBond(Time now,
                                  Time maturity,
                                  Array factors) const = 0;

        virtual Real discountBondOption(Option::Type type,
                                        Real strike,
                                        Time maturity,
                                        Time bondMaturity) const = 0;

        virtual Real discountBondOption(Option::Type type,
                                        Real strike,
                                        Time maturity,
                                        Time bondStart,
                                        Time bondMaturity) const;
    };


    //! Term-structure consistent model class
    /*! This is a base class for models that can reprice exactly
        any discount bond.

        \ingroup shortrate
    */
    class TermStructureConsistentModel : public virtual Observable {
      public:
        TermStructureConsistentModel(Handle<YieldTermStructure> termStructure)
        : termStructure_(std::move(termStructure)) {}
        const Handle<YieldTermStructure>& termStructure() const {
            return termStructure_;
        }
      private:
        Handle<YieldTermStructure> termStructure_;
    };


    //! Calibrated model class
    class CalibratedModel : public virtual Observer, public virtual Observable {
      public:
        CalibratedModel(Size nArguments);

        void update() override {
            generateArguments();
            notifyObservers();
        }

        //! Calibrate to a set of market instruments (usually caps/swaptions)
        /*! An additional constraint can be passed which must be
            satisfied in addition to the constraints of the model.
        */
        virtual void calibrate(
                const std::vector<ext::shared_ptr<CalibrationHelper> >&,
                OptimizationMethod& method,
                const EndCriteria& endCriteria,
                const Constraint& constraint = Constraint(),
                const std::vector<Real>& weights = std::vector<Real>(),
                const std::vector<bool>& fixParameters = std::vector<bool>());

        Real value(const Array& params,
                   const std::vector<ext::shared_ptr<CalibrationHelper> >&);

        const ext::shared_ptr<Constraint>& constraint() const;

        //! Returns end criteria result
        EndCriteria::Type endCriteria() const { return shortRateEndCriteria_; }

        //! Returns the problem values
        const Array& problemValues() const { return problemValues_; }

        //! Returns array of arguments on which calibration is done
        Array params() const;

        virtual void setParams(const Array& params);
        Integer functionEvaluation() const { return functionEvaluation_; }

      protected:
        virtual void generateArguments() {}
        std::vector<Parameter> arguments_;
        ext::shared_ptr<Constraint> constraint_;
        EndCriteria::Type shortRateEndCriteria_ = EndCriteria::None;
        Array problemValues_;
        Integer functionEvaluation_;

      private:
        //! Constraint imposed on arguments
        class PrivateConstraint;
        //! Calibration cost function class
        class CalibrationFunction;
        friend class CalibrationFunction;
    };

    //! Abstract short-rate model class
    /*! \ingroup shortrate */
    class ShortRateModel : public CalibratedModel {
      public:
        explicit ShortRateModel(Size nArguments);
        virtual ext::shared_ptr<Lattice> tree(const TimeGrid&) const = 0;
    };


    // inline definitions


    inline Real AffineModel::discountBondOption(Option::Type type,
                                                Real strike,
                                                Time maturity,
                                                Time,
                                                Time bondMaturity) const {
        return discountBondOption(type, strike, maturity, bondMaturity);
    }

    inline const ext::shared_ptr<Constraint>&
    CalibratedModel::constraint() const {
        return constraint_;
    }

    class CalibratedModel::PrivateConstraint : public Constraint {
      private:
        class Impl :  public Constraint::Impl {
          public:
            explicit Impl(const std::vector<Parameter>& arguments)
            : arguments_(arguments) {}

            bool test(const Array& params) const override {
                Size k=0;
                for (const auto& argument : arguments_) {
                    Size size = argument.size();
                    Array testParams(size);
                    for (Size j=0; j<size; j++, k++)
                        testParams[j] = params[k];
                    if (!argument.testParams(testParams))
                        return false;
                }
                return true;
            }

            Array upperBound(const Array& params) const override {
                Size k = 0, k2 = 0;
                Size totalSize = 0;
                for (const auto& argument : arguments_) {
                    totalSize += argument.size();
                }
                Array result(totalSize);
                for (const auto& argument : arguments_) {
                    Size size = argument.size();
                    Array partialParams(size);
                    for (Size j = 0; j < size; j++, k++)
                        partialParams[j] = params[k];
                    Array tmpBound = argument.constraint().upperBound(partialParams);
                    for (Size j = 0; j < size; j++, k2++)
                        result[k2] = tmpBound[j];
                }
                return result;
            }

            Array lowerBound(const Array& params) const override {
                Size k = 0, k2 = 0;
                Size totalSize = 0;
                for (const auto& argument : arguments_) {
                    totalSize += argument.size();
                }
                Array result(totalSize);
                for (const auto& argument : arguments_) {
                    Size size = argument.size();
                    Array partialParams(size);
                    for (Size j = 0; j < size; j++, k++)
                        partialParams[j] = params[k];
                    Array tmpBound = argument.constraint().lowerBound(partialParams);
                    for (Size j = 0; j < size; j++, k2++)
                        result[k2] = tmpBound[j];
                }
                return result;
            }

          private:
            const std::vector<Parameter>& arguments_;
        };
      public:
        explicit PrivateConstraint(const std::vector<Parameter>& arguments)
        : Constraint(ext::shared_ptr<Constraint::Impl>(
                                   new PrivateConstraint::Impl(arguments))) {}
    };

}

#endif
