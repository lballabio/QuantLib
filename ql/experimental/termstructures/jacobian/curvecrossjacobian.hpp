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

/*! \file curvecrossjacobian.hpp
    \brief cross-curve Jacobian construction
*/

#ifndef quantlib_experimental_curve_cross_jacobian_hpp
#define quantlib_experimental_curve_cross_jacobian_hpp

#include <ql/experimental/termstructures/jacobian/bootstrapequationjacobian.hpp>
#include <ql/experimental/termstructures/jacobian/curvechainrulecalculator.hpp>
#include <algorithm>
#include <cmath>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

namespace QuantLib {

    namespace detail {

        template <class C, class = void>
        constexpr bool hasBaseCurveHandle = false;

        template <class C>
        constexpr bool hasBaseCurveHandle<
            C, std::void_t<decltype(std::declval<const C&>().baseCurve())>> = true;

        //! erased interface to the internals of one bootstrapped curve
        class CurveJacobianImpl {
          public:
            virtual ~CurveJacobianImpl() = default;
            //! runs the bootstrap when needed
            virtual void ensure() const = 0;
            //! number of free nodes
            virtual Size numNodes() const = 0;
            //! dates of the free nodes
            virtual std::vector<Date> nodeDates() const = 0;
            //! alive helpers in curve order
            virtual std::vector<ext::shared_ptr<BootstrapHelper<YieldTermStructure>>>
                aliveHelpers() const = 0;
            /*! Converts dated curve-value sensitivities into one Jacobian
                row over the free nodes. Returns false when analytical
                weights are unavailable.
            */
            virtual bool analyticEquationRow(
                const std::vector<std::pair<Date, Real>>& dateSensitivities,
                std::vector<Real>& row) const = 0;
            //! value of free node j (1-based over the stored data)
            virtual Real nodeValue(Size j) const = 0;
            //! writes free node j
            virtual void setNodeValue(Size j, Real v) const = 0;
            /*! term structures used to turn nodes into curve values,
                read from the curve so that relinked handles are followed
            */
            virtual CurveDependencies valueDependencies() const = 0;
        };

        //! type-erased view of a bootstrapped curve
        /*! A null curve means the interface is unavailable. */
        struct CurveJacobianNode {
            ext::shared_ptr<YieldTermStructure> curve;
            CurveId id = nullptr;
            ext::shared_ptr<const CurveJacobianImpl> impl;

            CurveDependencies valueDependencies() const {
                return impl->valueDependencies();
            }
            void ensure() const { impl->ensure(); }
            Size numNodes() const { return impl->numNodes(); }
            std::vector<Date> nodeDates() const { return impl->nodeDates(); }
            std::vector<ext::shared_ptr<BootstrapHelper<YieldTermStructure>>>
            aliveHelpers() const { return impl->aliveHelpers(); }
            bool analyticEquationRow(
                    const std::vector<std::pair<Date, Real>>& dateSensitivities,
                    std::vector<Real>& row) const {
                return impl->analyticEquationRow(dateSensitivities, row);
            }
            Real nodeValue(Size j) const { return impl->nodeValue(j); }
            void setNodeValue(Size j, Real v) const { impl->setNodeValue(j, v); }
        };

        //! calibrated curves and derived wrappers in one group
        struct CurveJacobianGroup {
            std::vector<CurveJacobianNode> members;
            //! index of the curve that requested the group
            Size target = 0;
            std::set<const TermStructure*> dependents;
        };

        // bootstrap classes that expose a multi-curve group
        template <class B, class = void>
        constexpr bool hasJacobianGroup = false;

        template <class B>
        constexpr bool hasJacobianGroup<
            B, std::void_t<decltype(std::declval<const B&>().jacobianGroup())>> = true;

        // bootstrapped yield curves exposing jacobian()
        template <class Curve, class = void>
        constexpr bool supportsCurveJacobianNode = false;

        template <class Curve>
        constexpr bool supportsCurveJacobianNode<
            Curve, std::void_t<decltype(std::declval<const Curve&>().jacobian(
                std::declval<std::vector<bool>*>()))>> =
            std::is_base_of_v<YieldTermStructure, Curve>;

        template <class Curve>
        struct BootstrapJacobianAccess {
            using Traits = typename Curve::traits_type;

            //! CurveJacobianImpl over a concrete bootstrapped curve
            class Adapter : public CurveJacobianImpl {
              public:
                explicit Adapter(ext::shared_ptr<Curve> curve)
                : curve_(std::move(curve)) {}

                void ensure() const override { curve_->calculate(); }

                Size numNodes() const override {
                    curve_->calculate();
                    return curve_->times_.size() - 1;
                }

                std::vector<Date> nodeDates() const override {
                    curve_->calculate();
                    return std::vector<Date>(curve_->dates_.begin() + 1,
                                             curve_->dates_.end());
                }

                std::vector<ext::shared_ptr<BootstrapHelper<YieldTermStructure>>>
                aliveHelpers() const override {
                    curve_->calculate();
                    Date firstDate = Traits::initialDate(curve_.get());
                    std::vector<ext::shared_ptr<BootstrapHelper<YieldTermStructure>>> alive;
                    for (const auto& helper : curve_->instruments_)
                        if (helper->pillarDate() > firstDate) {
                            QL_REQUIRE(helper->termStructure() == curve_.get(),
                                       "the helper with pillar date " <<
                                       helper->pillarDate() << " is seated to "
                                       "a different curve; a rate helper "
                                       "cannot be shared between "
                                       "bootstrapped curves");
                            alive.push_back(helper);
                        }
                    return alive;
                }

                bool analyticEquationRow(
                        const std::vector<std::pair<Date, Real>>& dateSensitivities,
                        std::vector<Real>& row) const override {
                    curve_->calculate();
                    // analytical weights do not include jumps
                    if (!curve_->jumpDates().empty())
                        return false;
                    std::vector<std::pair<Time, Real>> sensitivities;
                    sensitivities.reserve(dateSensitivities.size());
                    for (const auto& [date, dQdP] : dateSensitivities)
                        sensitivities.emplace_back(
                            curve_->timeFromReference(date), dQdP);
                    return analyticBootstrapEquationRow<Traits>(
                        curve_.get(), sensitivities, curve_->times_,
                        curve_->interpolation_, row);
                }

                Real nodeValue(Size j) const override {
                    curve_->calculate();
                    return curve_->data_[j];
                }

                void setNodeValue(Size j, Real v) const override {
                    // preserve interpolation iterators
                    Traits::updateGuess(curve_->data_, v, j);
                    curve_->interpolation_.update();
                }

                //! a stand-alone curve has no value dependencies
                CurveDependencies valueDependencies() const override { return {}; }

              protected:
                ext::shared_ptr<Curve> curve_;
            };

            //! Adapter over a spread curve, which depends on its base curve
            class SpreadAdapter final : public Adapter {
              public:
                using Adapter::Adapter;

                CurveDependencies valueDependencies() const override {
                    // read the handle now: it can be relinked after construction
                    CurveDependencies dependencies;
                    dependencies.add(this->curve_->baseCurve().currentLink().get());
                    return dependencies;
                }
            };

            using NodeAdapter =
                std::conditional_t<hasBaseCurveHandle<Curve>, SpreadAdapter, Adapter>;

            static CurveJacobianNode makeNode(const ext::shared_ptr<Curve>& curve) {
                CurveJacobianNode n;
                n.curve = curve;
                n.id = static_cast<const TermStructure*>(curve.get());
                n.impl = ext::make_shared<NodeAdapter>(curve);
                return n;
            }
        };

        //! Fill selected rows by perturbing each free node
        template <class Value>
        void fillNumericalNodeRows(const CurveJacobianNode& n,
                                   const std::vector<Size>& rows,
                                   Matrix& J,
                                   const Value& value) {
            if (rows.empty())
                return;
            std::vector<Real> up(rows.size());
            for (Size j = 1; j <= J.columns(); ++j) {
                Real v = n.nodeValue(j);
                Real h = 1.0e-6 * std::max(std::abs(v), 0.01);

                struct RestoreNode {  // NOLINT(cppcoreguidelines-special-member-functions)
                    const CurveJacobianNode& node;
                    Size j;
                    Real value;
                    ~RestoreNode() { node.setNodeValue(j, value); }
                } restore{n, j, v};

                n.setNodeValue(j, v + h);
                for (Size k = 0; k < rows.size(); ++k)
                    up[k] = value(rows[k]);

                n.setNodeValue(j, v - h);
                for (Size k = 0; k < rows.size(); ++k)
                    J[rows[k]][j - 1] =
                        (up[k] - value(rows[k])) / (2.0 * h);
            }
            // setNodeValue() sends no notification, so evaluate the helpers at the restored nodes 
			// (otherwise we may end up with last-bumped instruments)
            for (Size k = 0; k < rows.size(); ++k)
                value(rows[k]);
        }

        //! implied-quote sensitivities of a curve's alive helpers, in row order
        inline std::vector<ImpliedQuoteSensitivities> aliveHelperSensitivities(
                const CurveJacobianNode& n) {
            auto helpers = n.aliveHelpers();
            std::vector<ImpliedQuoteSensitivities> result;
            result.reserve(helpers.size());
            for (const auto& helper : helpers)
                result.push_back(helper->impliedQuoteSensitivitiesByCurve());
            return result;
        }

        /*! Jacobian of one curve's helper quotes with respect to another
            curve's nodes. The same-curve case uses the diagonal block.
            Unsupported rows are differentiated numerically
        */
        inline Matrix curveCrossJacobian(const CurveJacobianNode& a,
                                         const CurveJacobianNode& b,
                                         const CurveCrossJacobianContext& context,
                                         std::vector<bool>* analyticEquations = nullptr,
                                         const std::vector<ImpliedQuoteSensitivities>*
                                             rowSensitivities = nullptr) {
            a.ensure();
            b.ensure();
            auto helpers = a.aliveHelpers();
            Size rows = helpers.size();
            Size cols = b.numNodes();
            QL_REQUIRE(rowSensitivities == nullptr ||
                           rowSensitivities->size() == rows,
                       "cached helper sensitivities have " <<
                       (rowSensitivities == nullptr ? 0 : rowSensitivities->size()) <<
                       " rows instead of " << rows);
            Matrix J(rows, cols, 0.0);
            std::vector<bool> analytic(rows, false);

            std::vector<Real> row;
            for (Size i = 0; i < rows; ++i) {
                ImpliedQuoteSensitivities fetched;
                if (rowSensitivities == nullptr)
                    fetched = helpers[i]->impliedQuoteSensitivitiesByCurve();
                const ImpliedQuoteSensitivities& s = rowSensitivities != nullptr
                    ? (*rowSensitivities)[i] : fetched;
                if (!s.available || s.incomplete.count(b.id) != 0)
                    continue;
                bool allAccounted = true;
                for (const auto& [curve, entries] : s.sensitivities)
                    allAccounted = allAccounted && context.accountsFor(curve, b.id);
                for (const auto* curve : s.incomplete)
                    allAccounted = allAccounted && context.accountsFor(curve, b.id);
                if (!allAccounted)
                    continue;
                auto bucket = s.sensitivities.find(b.id);
                bool indirectDependence = false;
                bool transformed = true;
                std::vector<std::pair<Date, Real>> targetSensitivities;
                if (bucket != s.sensitivities.end())
                    targetSensitivities = bucket->second;
                for (const auto& [source, entries] : s.sensitivities) {
                    if (source == b.id ||
                        !context.dependsOn(source, b.id))
                        continue;
                    indirectDependence = true;
                    transformed = transformed &&
                                  context.propagate(
                                      source, b.id, entries, targetSensitivities);
                }
                for (const auto* source : s.incomplete)
                    if (source != b.id &&
                        context.dependsOn(source, b.id)) {
                        indirectDependence = true;
                        transformed = false;
                    }
                if (indirectDependence) {
                    if (transformed && b.analyticEquationRow(targetSensitivities, row)) {
                        std::copy(row.begin(), row.end(), J.row_begin(i));
                        analytic[i] = true;
                    }
                } else if (bucket == s.sensitivities.end()) {
                    // no dependence on this curve
                    analytic[i] = true;
                } else if (b.analyticEquationRow(bucket->second, row)) {
                    std::copy(row.begin(), row.end(), J.row_begin(i));
                    analytic[i] = true;
                }
            }

            std::vector<Size> numericalRows;
            for (Size i = 0; i < rows; ++i)
                if (!analytic[i])
                    numericalRows.push_back(i);
            fillNumericalNodeRows(
                b, numericalRows, J,
                [&](Size i) { return helpers[i]->impliedQuote(); });

            if (analyticEquations != nullptr)
                *analyticEquations = std::move(analytic);
            return J;
        }

    }

    //! factory for a bootstrapped curve's Jacobian node
    /*! Bootstrapped curves implement this to add base curves for spread curves automatically
    */
    class CurveJacobianNodeProvider {
      public:
        virtual ~CurveJacobianNodeProvider() = default;
        virtual detail::CurveJacobianNode makeJacobianNode(
            const ext::shared_ptr<YieldTermStructure>& curve) const = 0;
    };

}

#endif
