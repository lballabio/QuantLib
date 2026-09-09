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

/*! \file curvejacobiangraph.hpp
    \brief cross-curve Jacobians of a set of bootstrapped curves
*/

#ifndef quantlib_experimental_curve_jacobian_graph_hpp
#define quantlib_experimental_curve_jacobian_graph_hpp

#include <ql/math/array.hpp>
#include <ql/math/matrix.hpp>
#include <ql/experimental/termstructures/jacobian/curvesensitivitypropagation.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <algorithm>
#include <map>
#include <utility>
#include <vector>

namespace QuantLib {

    //! cross-curve Jacobians for bootstrapped curves
    /*! Combines the bootstrap equations. Rows are helpers and columns are nodes.
    */
    class CurveJacobianGraph {
      public:
        explicit CurveJacobianGraph(bool errorOnIncomplete = false)
        : errorOnIncomplete_(errorOnIncomplete) {}

        /*! Whether every curve dependency reported by curves and their helpers are represented in the graph.
        */
        bool isComplete() const {
            for (const auto& node : nodes_) {
                if (!dependenciesAreRegistered(
                        node.valueDependencies().targets()))
                    return false;
                for (const auto& helper : node.aliveHelpers()) {
                    ImpliedQuoteSensitivities sensitivities =
                        helper->impliedQuoteSensitivitiesByCurve();
                    for (const auto& sensitivity : sensitivities.sensitivities)
                        if (!isRegistered(sensitivity.first))
                            return false;
                    if (!dependenciesAreRegistered(sensitivities.incomplete))
                        return false;
                }
            }
            return true;
        }

        /*! Adds (registers) a bootstrapped curve.
            By default, adding a spread curve also adds its underlying curve.
        */
        template <class Curve>
        void add(const ext::shared_ptr<Curve>& curve, bool addUnderlying = true) {
            static_assert(detail::supportsCurveJacobianNode<Curve>,
                          "CurveJacobianGraph::add requires a curve that "
                          "provides bootstrap Jacobians");
            QL_REQUIRE(curve, "null curve");
            if constexpr (detail::hasBaseCurveHandle<Curve>) {
                QL_REQUIRE(!curve->baseCurve().empty(),
                           "spread curve has an empty base-curve handle");
                if (addUnderlying)
                    addUnderlyingCurve(curve->baseCurve().currentLink());
            }
            addNode(detail::BootstrapJacobianAccess<Curve>::makeNode(curve));
        }

        /*! Partial Jacobian of the first curve's helper quotes with respect
            to the second curve's nodes. Other curve nodes are fixed.
        */
        Matrix crossJacobian(const YieldTermStructure& of,
                             const YieldTermStructure& withRespectTo,
            std::vector<bool>* analyticEquations = nullptr) const {
            requireComplete();
            detail::CurveCrossJacobianContext context = jacobianContext();
            return detail::curveCrossJacobian(node(of), node(withRespectTo),
                                              context, analyticEquations);
        }

        /*! Block of the inverse Jacobian.
            Rows are the first curve's nodes and columns are the second curve's helper quotes.
        */
        Matrix inverseJacobian(const YieldTermStructure& of,
                               const YieldTermStructure& withRespectTo,
                               std::vector<bool>* analyticEquations = nullptr) const {
            requireComplete();
            Size a = index(of), b = index(withRespectTo);
            detail::CurveCrossJacobianContext context = jacobianContext(false);
            detail::CurveJacobianBlocks blocks =
                detail::curveJacobianBlocks(nodes_, context);
            Matrix block = detail::inverseCurveJacobianBlock(blocks, a, b);
            if (analyticEquations != nullptr)
                *analyticEquations = blocks.analyticQuotes[b];
            return block;
        }

        /*! PV01-like: sensitivities to market quotes (not scaled, i.e. not multiplied by 0.0001)
            Input keys must be registered and arrays must match node counts.
        */
        std::map<const YieldTermStructure*, Array> marketQuoteSensitivities(
            const std::map<const YieldTermStructure*, Array>& nodeSensitivities,
            std::vector<bool>* analyticEquations = nullptr) const {
            requireComplete();
            detail::CurveCrossJacobianContext context = jacobianContext(false);
            detail::CurveJacobianBlocks blocks =
                detail::curveJacobianBlocks(nodes_, context);

            std::vector<Matrix> direct;
            direct.reserve(nodes_.size());
            for (Size i = 0; i < nodes_.size(); ++i)
                direct.emplace_back(blocks.numNodes(i), 1, 0.0);
            for (const auto& [curve, sensitivities] : nodeSensitivities) {
                Size a = index(*curve);
                QL_REQUIRE(sensitivities.size() == blocks.numNodes(a),
                           "node sensitivity size (" << sensitivities.size() <<
                           ") does not match the number of curve nodes (" <<
                           blocks.numNodes(a) << ")");
                for (Size i = 0; i < sensitivities.size(); ++i)
                    direct[a][i][0] = sensitivities[i];
            }

            std::vector<Matrix> propagated =
                detail::propagateCurveNodeSensitivities(blocks, direct);
            std::map<const YieldTermStructure*, Array> result;
            for (Size b = 0; b < nodes_.size(); ++b) {
                Array sensitivities(propagated[b].rows());
                for (Size i = 0; i < sensitivities.size(); ++i)
                    sensitivities[i] = propagated[b][i][0];
                result[nodes_[b].curve.get()] = std::move(sensitivities);
            }

            if (analyticEquations != nullptr) {
                std::vector<bool> flat;
                flat.reserve(blocks.quoteOffset.back());
                for (const auto& curveFlags : blocks.analyticQuotes)
                    flat.insert(flat.end(),
                                curveFlags.begin(), curveFlags.end());
                *analyticEquations = std::move(flat);
            }
            return result;
        }

      private:
        void addNode(detail::CurveJacobianNode node) {
            for (auto& existing : nodes_) {
                if (existing.id == node.id) {
                    existing = std::move(node);
                    return;
                }
            }
            nodes_.push_back(std::move(node));
        }

        void addUnderlyingCurve(const ext::shared_ptr<YieldTermStructure>& curve) {
            QL_REQUIRE(curve, "null underlying curve");
            const auto* id = static_cast<const TermStructure*>(curve.get());
            if (isRegistered(id))
                return;

            auto provider = ext::dynamic_pointer_cast<CurveJacobianNodeProvider>(curve);
            QL_REQUIRE(provider,
                       "underlying curve does not expose a Jacobian adapter; "
                       "add it explicitly or pass addUnderlying = false");
            addNode(provider->makeJacobianNode(curve));
        }

        void requireComplete() const {
            QL_REQUIRE(!errorOnIncomplete_ || isComplete(),
                       "incomplete curve Jacobian graph");
        }

        //! whether the curve was added
        bool isRegistered(detail::CurveId id) const {
            return std::any_of(
                nodes_.begin(), nodes_.end(),
                [id](const detail::CurveJacobianNode& node) {
                    return node.id == id;
                });
        }

        template <class Container>
        bool dependenciesAreRegistered(const Container& dependencies) const {
            return std::all_of(
                dependencies.begin(), dependencies.end(),
                [this](detail::CurveId id) { return isRegistered(id); });
        }

        Size index(const YieldTermStructure& curve) const {
            for (Size i = 0; i < nodes_.size(); ++i)
                if (nodes_[i].curve.get() == &curve)
                    return i;
            QL_FAIL("the given curve was not added to the graph");
        }

        const detail::CurveJacobianNode& node(const YieldTermStructure& curve) const {
            return nodes_[index(curve)];
        }

        detail::CurveCrossJacobianContext jacobianContext(bool includeAccountedCurves = true) const {
            detail::CurveCrossJacobianContext result;
            result.assumeUnlistedCurvesIndependent();
            for (const auto& node : nodes_) {
                if (includeAccountedCurves)
                    result.addCurve(node.id, node.valueDependencies());
                else
                    result.addDependencies(node.id, node.valueDependencies());
            }
            return result;
        }

        std::vector<detail::CurveJacobianNode> nodes_;
        bool errorOnIncomplete_;
    };

}

#endif
