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

#include <ql/option.hpp>
#include <ql/methods/lattices/lattice.hpp>
#include <ql/models/parameter.hpp>
#include <ql/models/calibrationhelper.hpp>
#include <ql/math/optimization/endcriteria.hpp>

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
        TermStructureConsistentModel(
                              const Handle<YieldTermStructure>& termStructure)
        : termStructure_(termStructure) {}
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

        void update() {
            generateArguments();
            notifyObservers();
        }

        //! Calibrate to a set of market instruments (usually caps/swaptions)
        /*! An additional constraint can be passed which must be
            satisfied in addition to the constraints of the model.
        */
        virtual void calibrate(
                const std::vector<boost::shared_ptr<CalibrationHelper> >&,
                OptimizationMethod& method,
                const EndCriteria& endCriteria,
                const Constraint& constraint = Constraint(),
                const std::vector<Real>& weights = std::vector<Real>(),
                const std::vector<bool>& fixParameters = std::vector<bool>());

        Real value(const Array& params,
                   const std::vector<boost::shared_ptr<CalibrationHelper> >&);

        const boost::shared_ptr<Constraint>& constraint() const;

        //! Returns end criteria result
        EndCriteria::Type endCriteria() const { return shortRateEndCriteria_; }

        //! Returns array of arguments on which calibration is done
        Disposable<Array> params() const;

        virtual void setParams(const Array& params);

      protected:
        virtual void generateArguments() {}
        std::vector<Parameter> arguments_;
        boost::shared_ptr<Constraint> constraint_;
        EndCriteria::Type shortRateEndCriteria_;

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
        ShortRateModel(Size nArguments);
        virtual boost::shared_ptr<Lattice> tree(const TimeGrid&) const = 0;
    };


    // inline definitions


    inline Real AffineModel::discountBondOption(Option::Type type,
                                                Real strike,
                                                Time maturity,
                                                Time,
                                                Time bondMaturity) const {
        return discountBondOption(type, strike, maturity, bondMaturity);
    }

    inline const boost::shared_ptr<Constraint>&
    CalibratedModel::constraint() const {
        return constraint_;
    }

    class CalibratedModel::PrivateConstraint : public Constraint {
      private:
        class Impl :  public Constraint::Impl {
          public:
            Impl(const std::vector<Parameter>& arguments)
            : arguments_(arguments) {}

            bool test(const Array& params) const {
                Size k=0;
                for (Size i=0; i<arguments_.size(); i++) {
                    Size size = arguments_[i].size();
                    Array testParams(size);
                    for (Size j=0; j<size; j++, k++)
                        testParams[j] = params[k];
                    if (!arguments_[i].testParams(testParams))
                        return false;
                }
                return true;
            }

            Array upperBound(const Array &params) const {
                Size k = 0, k2 = 0;
                Size totalSize = 0;
                for (Size i = 0; i < arguments_.size(); i++) {
                    totalSize += arguments_[i].size();
                }
                Array result(totalSize);
                for (Size i = 0; i < arguments_.size(); i++) {
                    Size size = arguments_[i].size();
                    Array partialParams(size);
                    for (Size j = 0; j < size; j++, k++)
                        partialParams[j] = params[k];
                    Array tmpBound =
                        arguments_[i].constraint().upperBound(partialParams);
                    for (Size j = 0; j < size; j++, k2++)
                        result[k2] = tmpBound[j];
                }
                return result;
            }

            Array lowerBound(const Array &params) const {
                Size k = 0, k2 = 0;
                Size totalSize = 0;
                for (Size i = 0; i < arguments_.size(); i++) {
                    totalSize += arguments_[i].size();
                }
                Array result(totalSize);
                for (Size i = 0; i < arguments_.size(); i++) {
                    Size size = arguments_[i].size();
                    Array partialParams(size);
                    for (Size j = 0; j < size; j++, k++)
                        partialParams[j] = params[k];
                    Array tmpBound =
                        arguments_[i].constraint().lowerBound(partialParams);
                    for (Size j = 0; j < size; j++, k2++)
                        result[k2] = tmpBound[j];
                }
                return result;
            }

          private:
            const std::vector<Parameter>& arguments_;
        };
      public:
        PrivateConstraint(const std::vector<Parameter>& arguments)
        : Constraint(boost::shared_ptr<Constraint::Impl>(
                                   new PrivateConstraint::Impl(arguments))) {}
    };

}

#endif
