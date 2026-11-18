/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 SoftSolutions! S.r.l.
 Copyright (C) 2025 Peter Caspers
 Copyright (C) 2026 Kyrylo Protsenko

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/termstructures/globalbootstrap.hpp>

namespace QuantLib {

MultiCurveBootstrap::MultiCurveBootstrap(Real accuracy, bool analyticJacobian)
: analyticJacobian_(analyticJacobian) {
    optimizer_ = ext::make_shared<LevenbergMarquardt>(accuracy, accuracy, accuracy,
                                                      analyticJacobian);
    endCriteria_ = ext::make_shared<EndCriteria>(1000, 10, accuracy, accuracy, accuracy);
}

MultiCurveBootstrap::MultiCurveBootstrap(ext::shared_ptr<OptimizationMethod> optimizer,
                                         ext::shared_ptr<EndCriteria> endCriteria,
                                         bool analyticJacobian)
: optimizer_(std::move(optimizer)), endCriteria_(std::move(endCriteria)),
  analyticJacobian_(analyticJacobian) {
    constexpr auto accuracy = 1E-10;
    if (optimizer_ == nullptr)
        optimizer_ = ext::make_shared<LevenbergMarquardt>(accuracy, accuracy, accuracy,
                                                          analyticJacobian);
    if (endCriteria_ == nullptr)
        endCriteria_ = ext::make_shared<EndCriteria>(1000, 10, accuracy, accuracy, accuracy);
}

void MultiCurveBootstrap::add(const MultiCurveBootstrapContributor* c) {
    contributors_.push_back(c);
    c->setParentBootstrapper(shared_from_this());
}

void MultiCurveBootstrap::addObserver(Observer* o) {
    observers_.push_back(o);
}

// the stacked cost function, extended with the optional analytical Jacobian
template <class ValuesFn>
class MultiCurveBootstrap::StackedCostFunction : public SimpleCostFunction<ValuesFn> {
  public:
    StackedCostFunction(ValuesFn values,
                        const MultiCurveBootstrap* b,
                        std::vector<Size> guessSizes)
    : SimpleCostFunction<ValuesFn>(std::move(values)), b_(b),
      guessSizes_(std::move(guessSizes)) {}
    void jacobian(Matrix& jac, const Array& x) const override;
  private:
    const MultiCurveBootstrap* b_;
    std::vector<Size> guessSizes_;
};

void MultiCurveBootstrap::runMultiCurveBootstrap() {

    std::vector<Size> guessSizes;
    std::vector<Real> globalGuess;

    for (auto const& c : contributors_) {
        Array guess = c->setupCostFunction();
        globalGuess.insert(globalGuess.end(), guess.begin(), guess.end());
        guessSizes.push_back(guess.size());
    }

    auto fn = [this, &guessSizes](const Array& x) {
        // call the contributors' cost functions' set part

        std::size_t offset = 0;
        for (std::size_t c = 0; c < contributors_.size(); ++c) {
            Array tmp(guessSizes[c]);
            std::copy(std::next(x.begin(), offset), std::next(x.begin(), offset + guessSizes[c]),
                      tmp.begin());
            offset += guessSizes[c];
            contributors_[c]->setCostFunctionArgument(tmp);
        }

        // update observers
        for(auto *o: observers_)
            o->update();

        // collect the contributors' result

        std::vector<Array> results;
        results.reserve(contributors_.size());
        for (auto& contributor : contributors_) {
            results.push_back(contributor->evaluateCostFunction());
        }

        // concatenate the contributors' values and return the concatenation as the result

        std::size_t resultSize =
            std::accumulate(results.begin(), results.end(), (std::size_t)0,
                            [](std::size_t len, const Array& a) { return len + a.size(); });

        Array result(resultSize);

        offset = 0;
        for (auto const& r : results) {
            std::copy(r.begin(), r.end(), std::next(result.begin(), offset));
            offset += r.size();
        }

        return result;
    };

    // metadata can change between runs (evaluation date, helpers)
    jacobianMetadata_ = std::nullopt;
    analyticUnavailable_ = false;

    if (analyticJacobian_)
        QL_REQUIRE(optimizer_->usesCostFunctionJacobian(),
                   "the analytical Jacobian was requested, but the supplied "
                   "optimizer does not consume CostFunction::jacobian()");

    StackedCostFunction<decltype(fn)> costFunction(fn, this, guessSizes);
    NoConstraint noConstraint;
    Problem problem(costFunction, noConstraint, Array(globalGuess.begin(), globalGuess.end()));
    EndCriteria::Type endType = optimizer_->minimize(problem, *endCriteria_);

    QL_REQUIRE(
        EndCriteria::succeeded(endType),
        "global bootstrap failed to minimize to required accuracy (during multi curve bootstrap): "
            << endType);

    // set all contributors to valid

    for (auto const& c : contributors_)
        c->setToValid();
}

template <class ValuesFn>
void MultiCurveBootstrap::StackedCostFunction<ValuesFn>::jacobian(Matrix& jac,
                                                                  const Array& x) const {
    if (b_->analyticJacobian_ && !b_->analyticUnavailable_) {
        this->values(x);
        if (b_->analyticCostJacobian(jac, x, guessSizes_))
            return;
    }
    b_->analyticUnavailable_ = true;
    CostFunction::jacobian(jac, x);
}

std::set<const TermStructure*> MultiCurveBootstrap::observerTermStructures() const {
    std::set<const TermStructure*> result;
    for (auto* o : observers_)
        if (const auto* ts = dynamic_cast<const TermStructure*>(o))
            result.insert(ts);
    return result;
}

//! Jacobian of the stacked cost values; the contributors' state must be at x
bool MultiCurveBootstrap::analyticCostJacobian(Matrix& jac,
                                               const Array& x,
                                               const std::vector<Size>& guessSizes) const {
    Size n = contributors_.size();

    // gather x-independent contributor metadata once per bootstrap run
    if (!jacobianMetadata_) {
        JacobianMetadata m;
        m.nodes.resize(n);
        m.weights.resize(n);
        m.resOffset.assign(n + 1, 0);
        m.varOffset.assign(n + 1, 0);
        for (Size i = 0; i < n; ++i) {
            m.nodes[i] = contributors_[i]->jacobianNode();
            if (m.nodes[i].curve == nullptr)
                return false;
            m.weights[i] = contributors_[i]->residualWeights();
            if (m.weights[i].empty() ||
                m.weights[i].size() != m.nodes[i].aliveHelpers().size())
                return false;
            if (m.nodes[i].numNodes() != guessSizes[i])
                return false;
            m.resOffset[i + 1] = m.resOffset[i] + m.weights[i].size();
            m.varOffset[i + 1] = m.varOffset[i] + guessSizes[i];
        }
        // known dependent wrappers require numerical propagation
        m.context.addNumericallyPropagatedCurves(observerTermStructures());
        m.context.assumeUnlistedCurvesIndependent();
        for (const auto& node : m.nodes)
            m.context.addCurve(node.id, node.valueDependencies());
        jacobianMetadata_.emplace(std::move(m));
    }
    const JacobianMetadata& m = *jacobianMetadata_;

    std::vector<Array> dT(n);
    for (Size i = 0; i < n; ++i) {
        Array xi(guessSizes[i]);
        std::copy(x.begin() + m.varOffset[i],
                  x.begin() + m.varOffset[i] + guessSizes[i], xi.begin());
        dT[i] = contributors_[i]->transformDerivatives(xi);
        if (dT[i].size() != guessSizes[i])
            return false;
    }
    if (jac.rows() != m.resOffset[n] || jac.columns() != m.varOffset[n])
        return false;

    // differentiate w_i * (quote_i - impliedQuote_i) through curve nodes
    for (Size i = 0; i < n; ++i) {
        std::vector<ImpliedQuoteSensitivities> rowSensitivities =
            detail::aliveHelperSensitivities(m.nodes[i]);
        for (Size j = 0; j < n; ++j) {
            std::vector<bool> analytic;
            Matrix Jq = detail::curveCrossJacobian(m.nodes[i], m.nodes[j],
                                                   m.context, &analytic,
                                                   &rowSensitivities);
            for (auto flag : analytic)  // NOLINT(readability-use-anyofallof)
                if (!flag)
                    return false;
            for (Size r = 0; r < Jq.rows(); ++r)
                for (Size c = 0; c < Jq.columns(); ++c)
                    jac[m.resOffset[i] + r][m.varOffset[j] + c] =
                        -m.weights[i][r] * Jq[r][c] * dT[j][c];
        }
    }
    return true;
}


} // namespace QuantLib
