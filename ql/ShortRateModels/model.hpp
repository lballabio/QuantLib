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

#include <ql/array.hpp>
#include <ql/option.hpp>
#include <ql/termstructure.hpp>
#include <ql/Lattices/tree.hpp>
#include <ql/ShortRateModels/parameter.hpp>
#include <ql/Optimization/problem.hpp>

namespace QuantLib {

    namespace ShortRateModels {

        //! Affine model class
        /*! This is the base class for analytically tractable models */
        class AffineModel {
          public:
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
            Model(Size nParameters);

            void update() { 
                generateParameters();
                notifyObservers(); 
            }

            virtual Handle<Lattices::Tree> tree(const TimeGrid& grid) const = 0;

            void calibrate(
                CalibrationSet& instruments,
                const Handle<Optimization::Method>& method);

          protected:
            virtual void generateParameters() {}

            std::vector<Parameter> parameters_;
            Handle<Optimization::Constraint> constraint_;

          private:
            Array params();

            void setParams(const Array& params);
            
            class PrivateConstraint;
            class CalibrationFunction;
            friend class CalibrationFunction;
        };

        class Model::PrivateConstraint : public Optimization::Constraint {
          public:
            class Implementation :  public ConstraintImpl {
              public:
                Implementation(const std::vector<Parameter>& parameters)
                : parameters_(parameters) {}
                bool test(const Array& params) const {
                    Size k=0;
                    for (Size i=0; i<parameters_.size(); i++) {
                        Size size = parameters_[i].size();
                        Array testParams(size);
                        for (Size j=0; j<size; j++, k++)
                            testParams[j] = params[k];
                        if (!parameters_[i].testParams(params))
                            return false;
                    }
                    return true;
                }
              private:
                const std::vector<Parameter>& parameters_;
            };
            PrivateConstraint(const std::vector<Parameter>& parameters)
            : Constraint(Handle<ConstraintImpl>(new Implementation(parameters)))
            {}
        };

    }

}
#endif
