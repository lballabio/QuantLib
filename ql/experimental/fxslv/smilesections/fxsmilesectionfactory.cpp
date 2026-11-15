/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

#include <ql/experimental/fxslv/smilesections/fxcostsmilesection.hpp>
#include <ql/experimental/fxslv/smilesections/fxsmilesectionbydelta.hpp>
#include <ql/experimental/fxslv/smilesections/fxsmilesectionbystrike.hpp>
#include <ql/experimental/fxslv/smilesections/fxsmilesectionfactory.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

    std::ostream& operator<<(std::ostream& out, FxSmileModel model) {
        switch (model) {
          case FxSmileModel::Sabr:
            return out << "SABR";
          case FxSmileModel::Svi:
            return out << "SVI";
          case FxSmileModel::Polynomial:
            return out << "polynomial";
          case FxSmileModel::Quadratic:
            return out << "quadratic";
          case FxSmileModel::CostFlatDynamics:
            return out << "cost (flat dynamics)";
          case FxSmileModel::CostScaledDynamics:
            return out << "cost (scaled dynamics)";
          case FxSmileModel::CostStickyDelta:
            return out << "cost (sticky delta)";
          case FxSmileModel::CostStickyDeltaConvex:
            return out << "cost (sticky delta with convexity)";
        }
        QL_FAIL("unknown FX smile model");
    }

    namespace {

        // the arguments every model's constructor takes
        struct Inputs {
            const Date& expiry;
            const Handle<Quote>& spot;
            const ext::shared_ptr<FxSmileQuotes>& quotes;
            const Handle<YieldTermStructure>& foreignCurve;
            const Handle<YieldTermStructure>& domesticCurve;
            DeltaVolQuote::DeltaType deltaType;
            DeltaVolQuote::AtmType atmType;
            const FxSettleConvention& settlement;
            const Date& referenceDate;
        };

        template <class Section>
        ext::shared_ptr<FxSmileSection> build(const Inputs& in) {
            return ext::make_shared<Section>(in.expiry, in.spot, in.quotes, in.foreignCurve,
                                             in.domesticCurve, in.deltaType, in.atmType,
                                             Actual365Fixed(), in.settlement, in.referenceDate);
        }

    }

    ext::shared_ptr<FxSmileSection>
    makeFxSmileSection(FxSmileModel model,
                       const Date& expiry,
                       const Handle<Quote>& spot,
                       const ext::shared_ptr<FxSmileQuotes>& quotes,
                       const Handle<YieldTermStructure>& foreignCurve,
                       const Handle<YieldTermStructure>& domesticCurve,
                       DeltaVolQuote::DeltaType deltaType,
                       DeltaVolQuote::AtmType atmType,
                       const FxSettleConvention& settlement,
                       const Date& referenceDate) {
        const Inputs in{expiry,    spot,    quotes,     foreignCurve, domesticCurve,
                        deltaType, atmType, settlement, referenceDate};
        switch (model) {
          case FxSmileModel::Sabr:
            return build<FxSabrSmileSection>(in);
          case FxSmileModel::Svi:
            return build<FxSviSmileSection>(in);
          case FxSmileModel::Polynomial:
            return build<FxPolynomialSmileSection>(in);
          case FxSmileModel::Quadratic:
            return build<FxQuadraticSmileSection>(in);
          case FxSmileModel::CostFlatDynamics:
            return build<FxCostSmileSectionFlatDynamics>(in);
          case FxSmileModel::CostScaledDynamics:
            return build<FxCostSmileSectionScaledDynamics>(in);
          case FxSmileModel::CostStickyDelta:
            return build<FxCostSmileSectionStickyDelta>(in);
          case FxSmileModel::CostStickyDeltaConvex:
            return build<FxCostSmileSectionStickyDeltaConvex>(in);
        }
        QL_FAIL("unknown FX smile model");
    }

}
