/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 SoftSolutions! S.r.l.
 Copyright (C) 2025 Peter Caspers

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

#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/termstructures/globalbootstrap.hpp>

namespace QuantLib {

    MultiCurveBootstrap::MultiCurveBootstrap(Real accuracy) {
        optimizer_ = ext::make_shared<LevenbergMarquardt>(accuracy, accuracy, accuracy);
        endCriteria_ = ext::make_shared<EndCriteria>(1000, 10, accuracy, accuracy, accuracy);
    }

    MultiCurveBootstrap::MultiCurveBootstrap(ext::shared_ptr<OptimizationMethod> optimizer,
                                             ext::shared_ptr<EndCriteria> endCriteria)
    : optimizer_(std::move(optimizer)), endCriteria_(std::move(endCriteria)) {}

    void MultiCurveBootstrap::add(const MultiCurveBootstrapContributor* c) {
        contributors_.push_back(c);
        c->setParentBootstrapper(shared_from_this());
    }

    void MultiCurveBootstrap::addCostFunction(std::function<void(const Array&)>* set,
                                              std::function<Array(void)>* eval,
                                              Array* guess) {
        costFunctionsSet_.push_back(set);
        costFunctionsEval_.push_back(eval);
        guesses_.push_back(guess);
    }

    void MultiCurveBootstrap::triggerOtherContributors() const {
        for (std::size_t i = 1; i < contributors_.size(); ++i)
            contributors_[i]->sendContribution();
    }

    void MultiCurveBootstrap::runMultiCurveBootstrap() {

        // concatenate the contributors' guesses to one guess

        std::size_t totalSizeInput =
            std::accumulate(guesses_.begin(), guesses_.end(), 0,
                            [](std::size_t l, Array* a) { return l + a->size(); });

        Array guess(totalSizeInput);

        std::size_t offset = 0;
        for (auto const& g : guesses_) {
            std::copy(g->begin(), g->end(), std::next(guess.begin(), offset));
            offset += g->size();
        }

        auto fn = [this](const Array& x) {
            // call the contributors' cost functions' set part

            std::size_t offset = 0;
            for (std::size_t c = 0; c < guesses_.size(); ++c) {
                Array tmp(guesses_[c]->size());
                std::copy(std::next(x.begin(), offset),
                          std::next(x.begin(), offset + guesses_[c]->size()), tmp.begin());
                offset += guesses_[c]->size();
                costFunctionsSet_[c]->operator()(tmp);
            }

            // collect the contributoes result

            std::vector<Array> results;
            for (std::size_t c = 0; c < guesses_.size(); ++c) {
                results.push_back(costFunctionsEval_[c]->operator()());
            }

            // concatenate the contributors' values and return the concatenation as the result

            std::size_t resultSize =
                std::accumulate(results.begin(), results.end(), 0,
                                [](std::size_t len, const Array& a) { return len + a.size(); });

            Array result(resultSize);

            offset = 0;
            for (auto const& r : results) {
                std::copy(r.begin(), r.end(), std::next(result.begin(), offset));
                offset += r.size();
            }

            return result;
        };

        SimpleCostFunction<decltype(fn)> costFunction(fn);
        NoConstraint noConstraint;
        Problem problem(costFunction, noConstraint, guess);
        EndCriteria::Type endType = optimizer_->minimize(problem, *endCriteria_);

        QL_REQUIRE(EndCriteria::succeeded(endType),
                   "global bootstrap failed to minimize to required accuracy (during multi curve "
                   "bootstrap): "
                       << endType);
    }

    void MultiCurveBootstrap::setOtherContributorsToValid() const {
        for (std::size_t i = 1; i < contributors_.size(); ++i)
            contributors_[i]->setToValid();
    }

    void MultiCurveBootstrap::finalizeCalculation() {
        contributors_.clear();
        costFunctionsSet_.clear();
        costFunctionsEval_.clear();
        guesses_.clear();
    }

} // namespace QuantLib
