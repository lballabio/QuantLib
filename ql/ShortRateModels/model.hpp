
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file model.hpp
    \brief Abstract interest rate model class
*/

#ifndef quantlib_interest_rate_modelling_model_h
#define quantlib_interest_rate_modelling_model_h

#include <ql/option.hpp>
#include <ql/Lattices/lattice.hpp>
#include <ql/ShortRateModels/parameter.hpp>
#include <ql/ShortRateModels/calibrationhelper.hpp>
#include <ql/Optimization/problem.hpp>

namespace QuantLib {

    //! Affine model class
    /*! Base class for analytically tractable models. 

        \ingroup shortrate
    */
    class AffineModel : public virtual Observable {
      public:
        //! Implied discount curve
        virtual DiscountFactor discount(Time t) const = 0;

        virtual double discountBondOption(Option::Type type,
                                          double strike,
                                          Time maturity,
                                          Time bondMaturity) const = 0;
    };

    //! Term-structure consistent model class
    /*! This is a base class for models that can reprice exactly
        any discount bond.

        \ingroup shortrate
    */
    class TermStructureConsistentModel : public virtual Observable {
      public:
        TermStructureConsistentModel(
                         const RelinkableHandle<TermStructure>& termStructure)
        : termStructure_(termStructure) {}
        const RelinkableHandle<TermStructure>& termStructure() const {
            return termStructure_;
        }
      private:
        RelinkableHandle<TermStructure> termStructure_;
    };

    //! Abstract short-rate model class
    /*! \ingroup shortrate */
    class ShortRateModel : public Observer, public virtual Observable {
      public:
        ShortRateModel(Size nArguments);

        void update() {
            generateArguments();
            notifyObservers();
        }

        virtual boost::shared_ptr<Lattice> tree(const TimeGrid&) const = 0;

        //! Calibrate to a set of market instruments (caps/swaptions)
        /*! An additional constraint can be passed which must be
          satisfied in addition to the constraints of the model.
        */
        void calibrate(
                   const std::vector<boost::shared_ptr<CalibrationHelper> >&,
                   OptimizationMethod& method,
                   const Constraint& constraint = Constraint());

        const boost::shared_ptr<Constraint>& constraint() const;

        //! Returns array of arguments on which calibration is done
        Disposable<Array> params() const;
        void setParams(const Array& params);
      protected:
        virtual void generateArguments() {}

        std::vector<Parameter> arguments_;
        boost::shared_ptr<Constraint> constraint_;

      private:
        //! Constraint imposed on arguments
        class PrivateConstraint;
        //! Calibration cost function class
        class CalibrationFunction;
        friend class CalibrationFunction;
    };

    // inline definitions

    inline const boost::shared_ptr<Constraint>&
    ShortRateModel::constraint() const {
        return constraint_;
    }

    class ShortRateModel::PrivateConstraint : public Constraint {
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
