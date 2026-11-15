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

/*! \file fxsmilesectionfactory.hpp
    \brief FX smile sections of a model chosen at run time
*/

#ifndef quantlib_fx_smile_section_factory_hpp
#define quantlib_fx_smile_section_factory_hpp

#include <ql/experimental/fxslv/smilesections/fxsmilesection.hpp>
#include <ostream>

namespace QuantLib {

    //! The smile models an FX smile section can be built with
    enum class FxSmileModel {
        Sabr,                  //!< FxSabrSmileSection
        Svi,                   //!< FxSviSmileSection
        Polynomial,            //!< FxPolynomialSmileSection
        Quadratic,             //!< FxQuadraticSmileSection
        CostFlatDynamics,      //!< FxCostSmileSectionFlatDynamics
        CostScaledDynamics,    //!< FxCostSmileSectionScaledDynamics
        CostStickyDelta,       //!< FxCostSmileSectionStickyDelta
        CostStickyDeltaConvex  //!< FxCostSmileSectionStickyDeltaConvex
    };

    std::ostream& operator<<(std::ostream& out, FxSmileModel model);

    //! A smile section of the given model, calibrated to the given quotes
    /*! The one place that maps a model to its class: code holding smile
        sections holds them as FxSmileSection, and builds them here, so
        that the model is a run-time choice, e.g. one made from Python.

        The section is in date mode, with times in Actual/365 (Fixed)
        from the reference date, as vols are quoted.

        \param model         the smile model.
        \param expiry        the expiry date.
        \param spot          the spot, domestic per unit of foreign.
        \param quotes        the quotes the section is calibrated to, e.g.
                             FxRrBfQuotes or FxDeltaVolQuotes.
        \param foreignCurve  foreign discount curve.
        \param domesticCurve domestic discount curve.
        \param deltaType     the delta convention of the quotes.
        \param atmType       the ATM convention of the quotes.
        \param settlement    the pair's settlement convention, for the
                             forward from spot to delivery.
        \param referenceDate the trade date; none for the evaluation date.
    */
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
                       const Date& referenceDate = Date());

}

#endif
