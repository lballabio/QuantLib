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
    ql/InterestRateModelling/%model.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_model_h
#define quantlib_interest_rate_modelling_model_h

#include <ql/array.hpp>
#include <ql/option.hpp>
#include <ql/termstructure.hpp>
#include <ql/InterestRateModelling/parameter.hpp>
#include <ql/Optimization/optimizer.hpp>


namespace QuantLib {

    namespace InterestRateModelling {

        class CalibrationSet;

        class Model {
          public:
            Model(Size nParameters, 
                  const RelinkableHandle<TermStructure>& termStructure)
            : parameters_(nParameters), termStructure_(termStructure) {}

            virtual ~Model() {}

            virtual bool hasDiscountBondFormula() const { return false; }
            virtual double discountBond(
                Time now, 
                Time maturity, 
                Rate r) const { return Null<double>(); }

            virtual bool hasDiscountBondOptionFormula() const { return false; }
            virtual double discountBondOption(
                Option::Type type,
                double strike,
                Time maturity,
                Time bondMaturity) const { return Null<double>(); }

            void calibrate(
                CalibrationSet& instruments,
                const Handle<Optimization::OptimizationMethod>& method);

            const RelinkableHandle<TermStructure>& termStructure() const {
                return termStructure_;
            }

          protected:
            virtual void generateParameters() {}

            Handle<Optimization::Constraint> constraint_;
            std::vector<Parameter> parameters_;

          private:
            Array params() {
                Size size = 0, i;
                for (i=0; i<parameters_.size(); i++)
                    size += parameters_[i].size();
                Array params(size);
                Size k = 0;
                for (i=0; i<parameters_.size(); i++) {
                    for (Size j=0; j<parameters_[i].size(); j++, k++) {
                        params[k] = parameters_[i].params()[j];
                    }
                }
                return params; 
            }

            void setParams(const Array& params) {
                Array::const_iterator p = params.begin();
                for (Size i=0; i<parameters_.size(); i++) {
                    for (Size j=0; j<parameters_[i].size(); j++, p++) {
                        QL_REQUIRE(p!=params.end(),"Parameter array too small");
                        parameters_[i].setParam(j, *p);
                    }
                }
                QL_REQUIRE(p==params.end(),"Parameter array too big!");
                generateParameters();
            }

            class CalibrationFunction;
            friend class CalibrationFunction;
            const RelinkableHandle<TermStructure>& termStructure_;
        };

        class ModelTermStructure : public DiscountStructure {
          public:
            ModelTermStructure(const Model& model, Time t0, Rate r0) 
            : model_(model), t0_(t0), r0_(r0) {
                QL_REQUIRE(model_.hasDiscountBondFormula(),
                    "No discount bond formula for this model");
            }

            virtual DiscountFactor discountImpl(Time t, bool extrapolate) {
                return model_.discountBond(t0_, t0_+t, r0_);
            }
          private:
            const Model& model_;
            Time t0_;
            Rate r0_;
        };

    }

}
#endif
