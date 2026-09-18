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

#include <ql/experimental/termstructures/quotesensitivitycalculator.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/stubiborcoupon.hpp>
#include <ql/errors.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    namespace detail {

        namespace {

            const YieldTermStructure* couponForecastCurve(const ext::shared_ptr<CashFlow>& cf) {
                auto floating = ext::dynamic_pointer_cast<FloatingRateCoupon>(cf);
                if (floating == nullptr)
                    return nullptr;
                auto index = ext::dynamic_pointer_cast<IborIndex>(floating->index());
                if (index == nullptr)
                    return nullptr;
                const Handle<YieldTermStructure>& h = index->forwardingTermStructure();
                return h.empty() ? nullptr : &**h;
            }

            const TermStructure* key(const YieldTermStructure* curve) {
                return static_cast<const TermStructure*>(curve);
            }

            void addSensitivities(ImpliedQuoteSensitivities& result,
                                  const CurvePointSensitivities& entries,
                                  Real scale) {
                for (const auto& e : entries)
                    result.sensitivities[e.curve].emplace_back(e.date, scale*e.derivative);
            }

            /* Rate helpers unregister their index from the curve handle to
               avoid notification loops so we do refreshes
            */
            void refreshCachedResults(const ext::shared_ptr<CashFlow>& cf) {
                if (auto lazy = ext::dynamic_pointer_cast<LazyObject>(cf))
                    lazy->deepUpdate();
            }

        }

        void addSimpleForwardSensitivities(ImpliedQuoteSensitivities& result,
                                           const YieldTermStructure& curve,
                                           const Date& d1,
                                           const Date& d2,
                                           Time tau,
                                           Real scale) {
            DiscountFactor P1 = curve.discount(d1);
            DiscountFactor P2 = curve.discount(d2);
            auto& bucket = result.sensitivities[key(&curve)];
            bucket.emplace_back(d1, scale/(tau*P2));
            bucket.emplace_back(d2, -scale*P1/(tau*P2*P2));
        }

        CouponContribution decomposeCoupon(const ext::shared_ptr<CashFlow>& cf,
                                                bool withSensitivities) {
            CouponContribution a;

            if (!withSensitivities) {
                if (auto cpn = ext::dynamic_pointer_cast<Coupon>(cf)) {
                    a.supported = true;
                    a.ntau = cpn->nominal()*cpn->accrualPeriod();
                    a.amount = cpn->amount();
                    a.forecastCurve = couponForecastCurve(cf);
                }
                return a;
            }

            if (auto fixed = ext::dynamic_pointer_cast<FixedRateCoupon>(cf)) {
                a.supported = true;
                a.ntau = fixed->nominal()*fixed->accrualPeriod();
                a.amount = fixed->amount();
                return a;
            }

            // This should come before the IborCoupon case below
            if (auto stub = ext::dynamic_pointer_cast<StubIborCoupon>(cf)) {
                if (stub->isInArrears())
                    return a;
                a.supported = true;
                a.ntau = stub->nominal()*stub->accrualPeriod();
                Real gearing = stub->gearing();
                a.amount = a.ntau*(gearing*stub->indexFixing() + stub->spread());
                if (!stub->hasFixed()) {
                    auto weighted = ext::dynamic_pointer_cast<
                        StubIborCoupon::WeightedIndex>(stub->iborIndex());
                    if (weighted == nullptr)
                        return {};
                    const Date& fixingDate = stub->fixingDate();
                    Date today = Settings::instance().evaluationDate();
                    for (const auto& [index, weight] : weighted->components()) {
                        // f_i = (P_i(v)/P_i(e)-1)/tau_i on each component's
                        // own forecast curve, weighted into the fixing
                        bool componentFixed = fixingDate < today ||
                            (fixingDate == today &&
                             (Settings::instance().enforcesTodaysHistoricFixings() ||
                              index->hasHistoricalFixing(fixingDate)));
                        if (componentFixed)
                            continue;
                        const Handle<YieldTermStructure>& curve =
                            index->forwardingTermStructure();
                        if (curve.empty())
                            return {};
                        Date v = index->valueDate(fixingDate);
                        Date e = index->maturityDate(v);
                        Time tau = index->dayCounter().yearFraction(v, e);
                        DiscountFactor Pv = curve->discount(v);
                        DiscountFactor Pe = curve->discount(e);
                        Real k = gearing*a.ntau*weight/tau;
                        a.amountSensitivities.push_back(
                            {key(&**curve), v, k/Pe});
                        a.amountSensitivities.push_back(
                            {key(&**curve), e, -k*Pv/(Pe*Pe)});
                    }
                }
                return a;
            }

            if (auto ibor = ext::dynamic_pointer_cast<IborCoupon>(cf)) {
                if (ibor->isInArrears())
                    return a;
                a.supported = true;
                a.ntau = ibor->nominal()*ibor->accrualPeriod();
                Real gearing = ibor->gearing();
                a.amount = a.ntau*(gearing*ibor->indexFixing() + ibor->spread());
                if (withSensitivities && !ibor->hasFixed()) {
                    // f = (P(v)/P(e)-1)/tau on the index's forecast curve
                    const Handle<YieldTermStructure>& curve =
                        ibor->iborIndex()->forwardingTermStructure();
                    if (curve.empty())
                        return {};
                    a.forecastCurve = &**curve;
                    const Date& v = ibor->fixingValueDate();
                    const Date& e = ibor->fixingEndDate();
                    Time tau = ibor->spanningTime();
                    DiscountFactor Pv = curve->discount(v);
                    DiscountFactor Pe = curve->discount(e);
                    Real k = gearing*a.ntau/tau;
                    a.amountSensitivities.push_back(
                        {key(a.forecastCurve), v, k/Pe});
                    a.amountSensitivities.push_back(
                        {key(a.forecastCurve), e, -k*Pv/(Pe*Pe)});
                }
                return a;
            }

            if (auto overnight = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(cf)) {
                if (overnight->averagingMethod() != RateAveraging::Compound ||
                    overnight->compoundSpreadDaily() ||
                    overnight->lockoutDays() != 0 ||
                    overnight->applyObservationShift() ||
                    !overnight->canApplyTelescopicFormula())
                    return a;
                a.supported = true;
                a.ntau = overnight->nominal()*overnight->accrualPeriod();
                Real rate = overnight->rate();
                a.amount = a.ntau*rate;
                if (!withSensitivities)
                    return a;

                // C = C_past * boundary factors * P(v_s)/P(v_e)
                auto index = ext::dynamic_pointer_cast<OvernightIndex>(overnight->index());
                const Handle<YieldTermStructure>& curve = index->forwardingTermStructure();
                if (curve.empty())
                    return {};
                const DayCounter& dc = index->dayCounter();
                Date today = Settings::instance().evaluationDate();

                const auto& fixingDates = overnight->fixingDates();
                const auto& valueDates = overnight->valueDates();
                const auto& interestDates = overnight->interestDates();
                const auto& dt = overnight->dt();
                Size n = fixingDates.size();

                // first forecast fixing
                Size i0 = 0;
                while (i0 < n && fixingDates[i0] < today)
                    ++i0;
                if (i0 < n && fixingDates[i0] == today &&
                    index->hasHistoricalFixing(fixingDates[i0]))
                    ++i0;
                if (i0 == n)  // fully fixed
                    return a;
                a.forecastCurve = &**curve;

                Real gearing = overnight->gearing();
                Time tau = dc.yearFraction(interestDates.front(), interestDates.back());
                Real compound = (rate - overnight->spread())*tau/gearing + 1.0;
                // amount = ntau * (gearing*(C-1)/tau + spread)
                Real amountScale = a.ntau*gearing*compound/tau;

                auto addGrowthFactor = [&](Size i) {
                    // g = 1 + f*dt[i], f = (P(v1)/P(v2)-1)/tauIndex
                    Real f = index->fixing(fixingDates[i]);
                    Real g = 1.0 + f*dt[i];
                    const Date& v1 = valueDates[i];
                    const Date& v2 = valueDates[i+1];
                    Time tauIndex = dc.yearFraction(v1, v2);
                    DiscountFactor P1 = curve->discount(v1);
                    DiscountFactor P2 = curve->discount(v2);
                    a.amountSensitivities.push_back(
                        {key(a.forecastCurve), v1,
                         amountScale*dt[i]/(tauIndex*P2*g)});
                    a.amountSensitivities.push_back(
                        {key(a.forecastCurve), v2,
                         -amountScale*dt[i]*P1/(tauIndex*P2*P2*g)});
                };
                // holiday boundary factors
                Size start = (i0 == 0 && valueDates.front() < interestDates.front())
                             ? Size(1) : i0;
                Size end = n - (valueDates[n] <= interestDates[n] ? 0 : 1);
                if (start < end) {
                    for (Size i = i0; i < start; ++i)
                        addGrowthFactor(i);
                    DiscountFactor Ps = curve->discount(valueDates[start]);
                    DiscountFactor Pe = curve->discount(valueDates[end]);
                    a.amountSensitivities.push_back(
                        {key(a.forecastCurve), valueDates[start], amountScale/Ps});
                    a.amountSensitivities.push_back(
                        {key(a.forecastCurve), valueDates[end], -amountScale/Pe});
                    for (Size i = end; i < n; ++i)
                        addGrowthFactor(i);
                } else {
                    for (Size i = i0; i < n; ++i)
                        addGrowthFactor(i);
                }
                return a;
            }

            return a;
        }

        bool decomposeCouponWithFallback(const ext::shared_ptr<CashFlow>& cf,
                                       CouponContribution& a,
                                       ImpliedQuoteSensitivities& result) {
            refreshCachedResults(cf);
            a = decomposeCoupon(cf, true);
            if (a.supported)
                return true;
            a = decomposeCoupon(cf, false);
            if (!a.supported)
                return false;
            if (ext::dynamic_pointer_cast<FloatingRateCoupon>(cf) != nullptr) {
                if (a.forecastCurve == nullptr)
                    return false;
                result.incomplete.insert(a.forecastCurve);
            }
            a.amountSensitivities.clear();
            return true;
        }

        void LegContribution::addFlow(const FlowSensitivityData& flow) {
            QL_REQUIRE(discountCurve != nullptr, "discount curve not set");
            DiscountFactor P = discountCurve->discount(flow.payDate);
            const TermStructure* discountKey = key(discountCurve);
            // d(x P) = P dx + x dP
            npv += flow.amount*P;
            for (const auto& e : flow.amountSensitivities)
                npvSensitivities.push_back({e.curve, e.date, P*e.derivative});
            if (flow.amount != 0.0)
                npvSensitivities.push_back({discountKey, flow.payDate, flow.amount});
            annuity += flow.ntau*P;
            for (const auto& e : flow.ntauSensitivities)
                annuitySensitivities.push_back({e.curve, e.date, P*e.derivative});
            if (flow.ntau != 0.0)
                annuitySensitivities.push_back({discountKey, flow.payDate, flow.ntau});
        }

        void LegContribution::addFlow(const Date& payDate,
                                             Real amount,
                                             Real ntau) {
            FlowSensitivityData d;
            d.payDate = payDate;
            d.amount = amount;
            d.ntau = ntau;
            addFlow(d);
        }

        bool decomposeLeg(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        ImpliedQuoteSensitivities& result,
                        LegContribution& contribution,
                        std::optional<bool> includeSettlementDateFlows,
                        const FlowDecomposer& customFlows) {
            contribution.discountCurve = &discountCurve;
            Date settlement = discountCurve.referenceDate();
            for (const auto& cf : leg) {
                if (cf->hasOccurred(settlement, includeSettlementDateFlows))
                    continue;
                FlowSensitivityData d;
                d.payDate = cf->date();
                FlowHandling handling = customFlows ? customFlows(cf, d)
                                                   : FlowHandling::NotApplicable;
                if (handling == FlowHandling::Unsupported)
                    return false;
                if (handling == FlowHandling::NotApplicable) {
                    if (ext::dynamic_pointer_cast<Coupon>(cf) != nullptr) {
                        CouponContribution a;
                        if (!decomposeCouponWithFallback(cf, a, result))
                            return false;
                        d.amount = a.amount;
                        d.ntau = a.ntau;
                        d.amountSensitivities = std::move(a.amountSensitivities);
                    } else {
                        refreshCachedResults(cf);
                        d.amount = cf->amount();
                    }
                }
                contribution.addFlow(d);
            }
            return true;
        }

        bool addQuotientSensitivities(ImpliedQuoteSensitivities& result,
                                      const QuotientSensitivitySpec& spec) {
            auto value = [](const std::vector<LegTerm>& terms) {
                Real sum = 0.0;
                for (const auto& t : terms)
                    sum += t.amountWeight*t.leg->npv + t.ntauWeight*t.leg->annuity;
                return sum;
            };
            Real N = value(spec.numerator);
            Real D = value(spec.denominator);
            if (D == 0.0)
                return false;
            Real Q = N/D;

            auto addTerms = [&](const std::vector<LegTerm>& terms, Real factor) {
                for (const auto& t : terms) {
                    if (t.amountWeight != 0.0)
                        addSensitivities(result, t.leg->npvSensitivities,
                                         factor*t.amountWeight);
                    if (t.ntauWeight != 0.0)
                        addSensitivities(result, t.leg->annuitySensitivities,
                                         factor*t.ntauWeight);
                }
            };
            // dQ = dN/D - Q dD/D
            addTerms(spec.numerator, 1.0/D);
            addTerms(spec.denominator, -Q/D);
            addSensitivities(result, spec.numeratorExtra, 1.0/D);
            addSensitivities(result, spec.denominatorExtra, -Q/D);
            return true;
        }

        ImpliedQuoteSensitivities fairRateSensitivities(
            const Leg& fixedLeg,
                              const Leg& floatingLeg,
                              Spread helperSpread,
                              const YieldTermStructure& discountCurve) {
            ImpliedQuoteSensitivities result;
            LegContribution fixed, floating;
            if (!decomposeLeg(fixedLeg, discountCurve, result, fixed) ||
                !decomposeLeg(floatingLeg, discountCurve, result, floating))
                return {};

            // fair rate = (floating NPV + spread * floating annuity) / fixed annuity
            QuotientSensitivitySpec spec;
            spec.numerator = {{&floating, 1.0, helperSpread}};
            spec.denominator = {{&fixed, 0.0, 1.0}};
            if (!addQuotientSensitivities(result, spec))
                return {};
            result.available = true;
            return result;
        }

        ImpliedQuoteSensitivities fairBasisSensitivities(
            const Leg& baseLeg,
                               const Leg& otherLeg,
                               const YieldTermStructure& discountCurve,
                               std::optional<bool> includeSettlementDateFlows) {
            ImpliedQuoteSensitivities result;
            LegContribution base, other;
            if (!decomposeLeg(baseLeg, discountCurve, result, base,
                            includeSettlementDateFlows) ||
                !decomposeLeg(otherLeg, discountCurve, result, other,
                            includeSettlementDateFlows))
                return {};

            // fair basis = (other NPV - base NPV) / base annuity
            QuotientSensitivitySpec spec;
            spec.numerator = {{&other, 1.0, 0.0}, {&base, -1.0, 0.0}};
            spec.denominator = {{&base, 0.0, 1.0}};
            if (!addQuotientSensitivities(result, spec))
                return {};
            result.available = true;
            return result;
        }

    }

}
