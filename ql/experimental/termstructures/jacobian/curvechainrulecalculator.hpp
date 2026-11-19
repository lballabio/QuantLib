/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file curvechainrulecalculator.hpp
    \brief chain-rule propagation across curve-value dependencies
*/

#ifndef quantlib_experimental_curve_chain_rule_calculator_hpp
#define quantlib_experimental_curve_chain_rule_calculator_hpp

#include <ql/errors.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/date.hpp>
#include <ql/types.hpp>
#include <algorithm>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace QuantLib {

    namespace detail {

        using CurveId = const TermStructure*;
        using DatedCurveSensitivities = std::vector<std::pair<Date, Real>>;

        //! curves whose values enter another curve's values
        class CurveDependencies {
          public:
            void add(CurveId target) {
                QL_REQUIRE(target != nullptr, "null dependency target");
                if (std::find(targets_.begin(), targets_.end(), target) == targets_.end())
                    targets_.push_back(target);
            }

            //! by value: callers often iterate the targets of a temporary
            std::vector<CurveId> targets() const { return targets_; }

          private:
            std::vector<CurveId> targets_;
        };

        //! chain-rule propagation across curve-value dependencies
        /*! Every dependency is a multiplicative discount spread: the source
            curve discounts as \f$ P_s(d) = P_t(d) S(d) \f$ over its target,
            with \f$ S \f$ fixed by the source's own nodes.  A sensitivity to
            \f$ P_s(d) \f$ therefore becomes \f$ P_s(d)/P_t(d) \f$ times a
            sensitivity to \f$ P_t(d) \f$.
        */
        class CurveChainRuleCalculator {
          public:
            void add(CurveId source, CurveId target) {
                QL_REQUIRE(source != nullptr, "null dependency source");
                dependencies_[source].add(target);
            }

            void add(CurveId source,
                     const CurveDependencies& dependencies) {
                QL_REQUIRE(source != nullptr, "null dependency source");
                for (CurveId target : dependencies.targets())
                    add(source, target);
            }

            void add(const CurveChainRuleCalculator& other) {
                for (const auto& [source, dependencies] : other.dependencies_)
                    add(source, dependencies);
            }

            std::vector<CurveId> targets(CurveId source) const {
                return outgoing(source).targets();
            }

            bool dependsOn(CurveId source, CurveId target) const {
                if (source == target)
                    return true;
                std::set<CurveId> visited;
                std::vector<CurveId> pending{source};
                while (!pending.empty()) {
                    CurveId current = pending.back();
                    pending.pop_back();
                    if (!visited.insert(current).second)
                        continue;
                    for (CurveId next : outgoing(current).targets()) {
                        if (next == target)
                            return true;
                        pending.push_back(next);
                    }
                }
                return false;
            }

            bool propagate(CurveId source,
                           CurveId target,
                           const DatedCurveSensitivities& input,
                           DatedCurveSensitivities& output) const {
                std::set<CurveId> visiting;
                return propagate(source, target, input, output, visiting);
            }

          private:
            bool propagate(CurveId source,
                           CurveId target,
                           const DatedCurveSensitivities& input,
                           DatedCurveSensitivities& output,
                           std::set<CurveId>& visiting) const {
                if (source == target) {
                    output.insert(output.end(), input.begin(), input.end());
                    return true;
                }
                if (!visiting.insert(source).second)
                    return false;

                bool foundPath = false;
                DatedCurveSensitivities accumulated;
                for (CurveId next : outgoing(source).targets()) {
                    if (next != target && !dependsOn(next, target))
                        continue;
                    DatedCurveSensitivities direct, branch;
                    if (!transform(source, next, input, direct) ||
                        !propagate(next, target, direct, branch, visiting)) {
                        visiting.erase(source);
                        return false;
                    }
                    foundPath = true;
                    accumulated.insert(accumulated.end(),
                                       branch.begin(), branch.end());
                }
                visiting.erase(source);
                if (foundPath)
                    output.insert(output.end(), accumulated.begin(), accumulated.end());
                return foundPath;
            }

            //! dQ/dP_t(d) = dQ/dP_s(d) * P_s(d)/P_t(d) along one edge
            static bool transform(CurveId source,
                                  CurveId target,
                                  const DatedCurveSensitivities& input,
                                  DatedCurveSensitivities& output) {
                const auto* s = dynamic_cast<const YieldTermStructure*>(source);
                const auto* t = dynamic_cast<const YieldTermStructure*>(target);
                if (s == nullptr || t == nullptr)
                    return false;
                output.reserve(output.size() + input.size());
                for (const auto& [date, dQdP] : input)
                    output.emplace_back(
                        date, dQdP * s->discount(date, true) / t->discount(date, true));
                return true;
            }

            const CurveDependencies& outgoing(CurveId source) const {
                static const CurveDependencies empty;
                auto i = dependencies_.find(source);
                return i == dependencies_.end() ? empty : i->second;
            }

            std::map<CurveId, CurveDependencies> dependencies_;
        };

        //! dependency context for one cross-curve block
        class CurveCrossJacobianContext {
          public:
            void addCurve(
                    CurveId curve,
                    const CurveDependencies& dependencies = {}) {
                QL_REQUIRE(curve != nullptr, "null accounted curve");
                accountedCurves_.insert(curve);
                chainRule_.add(curve, dependencies);
            }

            void addDependencies(
                    CurveId curve,
                    const CurveDependencies& dependencies) {
                chainRule_.add(curve, dependencies);
            }

            void addDependencies(const CurveChainRuleCalculator& dependencies) {
                chainRule_.add(dependencies);
            }

            void addNumericallyPropagatedCurve(CurveId curve) {
                QL_REQUIRE(curve != nullptr,
                           "null numerically propagated curve");
                numericallyPropagatedCurves_.insert(curve);
            }

            void addNumericallyPropagatedCurves(
                    const std::set<CurveId>& curves) {
                for (CurveId curve : curves)
                    addNumericallyPropagatedCurve(curve);
            }

            void assumeUnlistedCurvesIndependent(bool enabled = true) {
                unlistedCurvesAreIndependent_ = enabled;
            }

            bool accountsFor(CurveId curve, CurveId differentiatedCurve) const {
                return curve == differentiatedCurve ||
                       accountedCurves_.count(curve) != 0 ||
                       (unlistedCurvesAreIndependent_ &&
                        numericallyPropagatedCurves_.count(curve) == 0);
            }

            bool dependencyIsResolved(CurveId curve) const {
                return accountedCurves_.count(curve) != 0 ||
                       (unlistedCurvesAreIndependent_ &&
                        numericallyPropagatedCurves_.count(curve) == 0);
            }

            bool dependsOn(CurveId source, CurveId target) const {
                return chainRule_.dependsOn(source, target);
            }

            bool propagate(CurveId source,
                           CurveId target,
                           const DatedCurveSensitivities& input,
                           DatedCurveSensitivities& output) const {
                return chainRule_.propagate(source, target, input, output);
            }

          private:
            std::set<CurveId> accountedCurves_;
            std::set<CurveId> numericallyPropagatedCurves_;
            bool unlistedCurvesAreIndependent_ = false;
            CurveChainRuleCalculator chainRule_;
        };

    }

}

#endif
