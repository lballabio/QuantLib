/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
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

#include <ql/models/model.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/projection.hpp>
#include <ql/math/optimization/projectedconstraint.hpp>

using std::vector;
using boost::shared_ptr;

namespace QuantLib {

    namespace {
        void no_deletion(CalibratedModel*) {}
    }

    CalibratedModel::CalibratedModel(Size nArguments)
    : arguments_(nArguments),
      constraint_(new PrivateConstraint(arguments_)),
      shortRateEndCriteria_(EndCriteria::None) {}

    class CalibratedModel::CalibrationFunction : public CostFunction {
      public:
        CalibrationFunction(CalibratedModel* model,
                            const vector<shared_ptr<CalibrationHelper> >& h,
                            const vector<Real>& weights,
                            const Projection& projection)
        : model_(model, no_deletion), instruments_(h),
          weights_(weights), projection_(projection) { }

        virtual ~CalibrationFunction() {}

        virtual Real value(const Array& params) const {
            model_->setParams(projection_.include(params));
            Real value = 0.0;
            for (Size i=0; i<instruments_.size(); i++) {
                Real diff = instruments_[i]->calibrationError();
                value += diff*diff*weights_[i];
            }
            return std::sqrt(value);
        }

        virtual Disposable<Array> values(const Array& params) const {
            model_->setParams(projection_.include(params));
            Array values(instruments_.size());
            for (Size i=0; i<instruments_.size(); i++) {
                values[i] = instruments_[i]->calibrationError()
                           *std::sqrt(weights_[i]);
            }
            return values;
        }

        virtual Real finiteDifferenceEpsilon() const { return 1e-6; }

      private:
        shared_ptr<CalibratedModel> model_;
        const vector<shared_ptr<CalibrationHelper> >& instruments_;
        vector<Real> weights_;
        const Projection projection_;
    };

    void CalibratedModel::calibrate(
                    const vector<shared_ptr<CalibrationHelper> >& instruments,
                    OptimizationMethod& method,
                    const EndCriteria& endCriteria,
                    const Constraint& additionalConstraint,
                    const vector<Real>& weights,
                    const vector<bool>& fixParameters) {

        QL_REQUIRE(weights.empty() || weights.size() == instruments.size(),
                   "mismatch between number of instruments (" <<
                   instruments.size() << ") and weights(" <<
                   weights.size() << ")");

        Constraint c;
        if (additionalConstraint.empty())
            c = *constraint_;
        else
            c = CompositeConstraint(*constraint_,additionalConstraint);
        vector<Real> w =
            weights.empty() ? vector<Real>(instruments.size(), 1.0): weights;

        Array prms = params();
        vector<bool> all(prms.size(), false);
        Projection proj(prms,fixParameters.size()>0 ? fixParameters : all);
        CalibrationFunction f(this,instruments,w,proj);
        ProjectedConstraint pc(c,proj);
        Problem prob(f, pc, proj.project(prms));
        shortRateEndCriteria_ = method.minimize(prob, endCriteria);
        Array result(prob.currentValue());
        setParams(proj.include(result));
        Array shortRateProblemValues_ = prob.values(result);

        notifyObservers();
    }

    Real CalibratedModel::value(
                const Array& params,
                const vector<shared_ptr<CalibrationHelper> >& instruments) {
        vector<Real> w = vector<Real>(instruments.size(), 1.0);
        Projection p(params);
        CalibrationFunction f(this, instruments, w, p);
        return f.value(params);
    }

    Disposable<Array> CalibratedModel::params() const {
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

    void CalibratedModel::setParams(const Array& params) {
        Array::const_iterator p = params.begin();
        for (Size i=0; i<arguments_.size(); ++i) {
            for (Size j=0; j<arguments_[i].size(); ++j, ++p) {
                QL_REQUIRE(p!=params.end(),"parameter array too small");
                arguments_[i].setParam(j, *p);
            }
        }
        QL_REQUIRE(p==params.end(),"parameter array too big!");
        generateArguments();
        notifyObservers();
    }

    ShortRateModel::ShortRateModel(Size nArguments)
    : CalibratedModel(nArguments) {}

}
