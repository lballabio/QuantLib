
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

#include <ql/ShortRateModels/model.hpp>

namespace QuantLib {

    #if defined(HAVE_BOOST)
    namespace {
        void no_deletion(ShortRateModel*) {}
    }
    #else
    namespace {
        bool no_deletion = false;
    }
    #endif

    ShortRateModel::ShortRateModel(Size nArguments) 
    : arguments_(nArguments),
      constraint_(new PrivateConstraint(arguments_)) {}

    class ShortRateModel::CalibrationFunction : public CostFunction {
      public:
        CalibrationFunction( 
                  ShortRateModel* model,
                  const std::vector<Handle<CalibrationHelper> >& instruments) 
        : model_(model, no_deletion), instruments_(instruments) {}
        virtual ~CalibrationFunction() {}

        virtual double value(const Array& params) const {
            model_->setParams(params);

            double value = 0.0;
            for (Size i=0; i<instruments_.size(); i++) {
                double diff = instruments_[i]->calibrationError();
                value += diff*diff;
            }

            return QL_SQRT(value);
        }
        virtual double finiteDifferenceEpsilon() const { return 1e-6; }
      private:
        Handle<ShortRateModel> model_;
        const std::vector<Handle<CalibrationHelper> >& instruments_;
    };

    void ShortRateModel::calibrate(
                   const std::vector<Handle<CalibrationHelper> >& instruments,
                   OptimizationMethod& method, 
                   const Constraint& additionalConstraint) {

        Constraint c;
        if (additionalConstraint.isNull())
            c = *constraint_;
        else
            c = CompositeConstraint(*constraint_,additionalConstraint);

        CalibrationFunction f(this, instruments);

        method.setInitialValue(params());
        method.endCriteria().setPositiveOptimization();
        Problem prob(f, c, method);
        prob.minimize();

        Array result(prob.minimumValue());
        setParams(result);
    }

    Disposable<Array> ShortRateModel::params() const {
        Size size = 0, i;
        for (i=0; i<arguments_.size(); i++)
            size += arguments_[i].size();
        Array params(size);
        Size k = 0;
        for (i=0; i<arguments_.size(); i++) {
            for (Size j=0; j<arguments_[i].size(); j++, k++) {
                params[k] = arguments_[i].params()[j];
            }
        }
        return params; 
    }

    void ShortRateModel::setParams(const Array& params) {
        Array::const_iterator p = params.begin();
        for (Size i=0; i<arguments_.size(); i++) {
            for (Size j=0; j<arguments_[i].size(); j++, p++) {
                QL_REQUIRE(p!=params.end(),"Parameter array too small");
                arguments_[i].setParam(j, *p);
            }
        }
        QL_REQUIRE(p==params.end(),"Parameter array too big!");
        update();
    }

}
