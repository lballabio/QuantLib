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

#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/projectedconstraint.hpp>
#include <ql/math/optimization/projection.hpp>
#include <ql/models/model.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

using std::vector;

namespace QuantLib {

    CalibratedModel::CalibratedModel(Size nArguments)
    : arguments_(nArguments),
      constraint_(new PrivateConstraint(arguments_)),
      shortRateEndCriteria_(EndCriteria::None) {}

    class CalibratedModel::CalibrationFunction : public CostFunction {
      public:
        CalibrationFunction(CalibratedModel* model,
                            const vector<ext::shared_ptr<CalibrationHelper> >& h,
                            vector<Real> weights,
                            const Projection& projection)
        : model_(model, null_deleter()), instruments_(h), weights_(std::move(weights)),
          projection_(projection) {}

        ~CalibrationFunction() override = default;

        Real value(const Array& params) const override {
            model_->setParams(projection_.include(params));
            Real value = 0.0;
            for (Size i=0; i<instruments_.size(); i++) {
                Real diff = instruments_[i]->calibrationError();
                value += diff*diff*weights_[i];
            }
            return std::sqrt(value);
        }

        Array values(const Array& params) const override {
            model_->setParams(projection_.include(params));
            Array values(instruments_.size());
            for (Size i=0; i<instruments_.size(); i++) {
                values[i] = instruments_[i]->calibrationError()
                           *std::sqrt(weights_[i]);
            }
            return values;
        }

        Real finiteDifferenceEpsilon() const override { return 1e-6; }

      private:
        ext::shared_ptr<CalibratedModel> model_;
        const vector<ext::shared_ptr<CalibrationHelper> >& instruments_;
        vector<Real> weights_;
        const Projection projection_;
    };

    void CalibratedModel::calibrate(
            const vector<ext::shared_ptr<CalibrationHelper> >& instruments,
            OptimizationMethod& method,
            const EndCriteria& endCriteria,
            const Constraint& additionalConstraint,
            const vector<Real>& weights,
            const vector<bool>& fixParameters) {

        QL_REQUIRE(!instruments.empty(), "no instruments provided");

        Constraint c;
        if (additionalConstraint.empty())
            c = *constraint_;
        else
            c = CompositeConstraint(*constraint_,additionalConstraint);

        QL_REQUIRE(weights.empty() || weights.size() == instruments.size(),
                   "mismatch between number of instruments (" <<
                   instruments.size() << ") and weights (" <<
                   weights.size() << ")");
        vector<Real> w =
            weights.empty() ? vector<Real>(instruments.size(), 1.0): weights;

        Array prms = params();
        QL_REQUIRE(fixParameters.empty() || fixParameters.size() == prms.size(),
                   "mismatch between number of parameters (" <<
                   prms.size() << ") and fixed-parameter specs (" <<
                   fixParameters.size() << ")");
        vector<bool> all(prms.size(), false);
        Projection proj(prms, !fixParameters.empty() ? fixParameters : all);
        CalibrationFunction f(this,instruments,w,proj);
        ProjectedConstraint pc(c,proj);
        Problem prob(f, pc, proj.project(prms));
        shortRateEndCriteria_ = method.minimize(prob, endCriteria);
        Array result(prob.currentValue());
        setParams(proj.include(result));
        problemValues_ = prob.values(result);
        functionEvaluation_ = prob.functionEvaluation();

        notifyObservers();
    }

    Real CalibratedModel::value(
                const Array& params,
                const vector<ext::shared_ptr<CalibrationHelper> >& instruments) {
        vector<Real> w = vector<Real>(instruments.size(), 1.0);
        Projection p(params);
        CalibrationFunction f(this, instruments, w, p);
        return f.value(params);
    }

    Array CalibratedModel::params() const {
        Size size=0;
        for (const auto& argument : arguments_)
            size += argument.size();
        Array params(size);
        for (Size i=0, k=0; i<arguments_.size(); ++i) {
            for (Size j=0; j<arguments_[i].size(); ++j, ++k)
                params[k] = arguments_[i].params()[j];
        }
        return params;
    }

    void CalibratedModel::setParams(const Array& params) {
        Array::const_iterator p = params.begin();
        for (auto& argument : arguments_) {
            for (Size j = 0; j < argument.size(); ++j, ++p) {
                QL_REQUIRE(p!=params.end(),"parameter array too small");
                argument.setParam(j, *p);
            }
        }
        QL_REQUIRE(p==params.end(),"parameter array too big!");
        generateArguments();
        notifyObservers();
    }

    ShortRateModel::ShortRateModel(Size nArguments)
    : CalibratedModel(nArguments) {}

}
