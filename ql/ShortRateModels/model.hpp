
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

/*! \file model.hpp
    \brief Abstract interest rate model class

    \fullpath
    ql/ShortRateModels/%model.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_model_h
#define quantlib_interest_rate_modelling_model_h

#include <ql/option.hpp>
#include <ql/Lattices/lattice.hpp>
#include <ql/ShortRateModels/parameter.hpp>
#include <ql/Optimization/problem.hpp>

namespace QuantLib {

    /*! \namespace QuantLib::ShortRateModels
        \brief Implementations of short-rate models

        See sect. \ref fixedincome
    */

    namespace ShortRateModels {

        //! Affine model class
        /*! This is the base class for analytically tractable models */
        class AffineModel {
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
        */
        class TermStructureConsistentModel {
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

        class CalibrationSet;

        //! Abstract short-rate model class
        class Model : public Patterns::Observer, 
                      public Patterns::Observable {
          public:
            Model(Size nArguments);

            void update() { 
                generateArguments();
                notifyObservers(); 
            }

            virtual Handle<Lattices::Lattice> tree(
                const TimeGrid& grid) const = 0;

            //! Calibrate to a set of market instruments (caps/swaptions)
            void calibrate(
                CalibrationSet& instruments,
                const Handle<Optimization::Method>& method);

            const Handle<Optimization::Constraint>& constraint() const;

            //! Returns array of arguments on which calibration is done
            Array params();
            void setParams(const Array& params);
          protected:
            virtual void generateArguments() {}

            std::vector<Parameter> arguments_;
            Handle<Optimization::Constraint> constraint_;

          private:
            //! Constraint imposed on arguments
            class PrivateConstraint;
            //! Calibration cost function class
            class CalibrationFunction;
            friend class CalibrationFunction;
        };

        // inline definitions

        inline 
        const Handle<Optimization::Constraint>& Model::constraint() const {
            return constraint_;
        }

        class Model::PrivateConstraint : public Optimization::Constraint {
          private:
            class Impl :  public Optimization::Constraint::Impl {
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
                        if (!arguments_[i].testParams(params))
                            return false;
                    }
                    return true;
                }
              private:
                const std::vector<Parameter>& arguments_;
            };
          public:
            PrivateConstraint(const std::vector<Parameter>& arguments)
            : Optimization::Constraint(Handle<Constraint::Impl>(
                new PrivateConstraint::Impl(arguments))) {}
        };

    }

}
#endif
