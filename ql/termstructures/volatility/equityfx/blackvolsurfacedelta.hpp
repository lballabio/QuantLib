/*
 Copyright (C) 2019 Quaternion Risk Management Ltd
 Copyright (C) 2022 Skandinaviska Enskilda Banken AB (publ)
 Copyright (C) 2025 Paolo D'Elia
 All rights reserved.

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

/*! \file blackvolsurfacedelta.hpp
 \brief Black volatility surface based on delta
 \ingroup termstructures
 */

#ifndef quantlib_black_variance_surface_delta_hpp
#define quantlib_black_variance_surface_delta_hpp

#include <ql/experimental/fx/deltavolquote.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/math/matrix.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>

namespace QuantLib {

    /*! \brief Black volatility surface parameterized by market deltas
        \ingroup termstructures

        BlackVolatilitySurfaceDelta represents a Black volatility term
        structure where market quotes are expressed as delta-based points
        (put deltas, call deltas and optionally an ATM quote) for a set of
        option expiries.  The surface converts the provided deltas to strikes
        (using spot and domestic/foreign yield curves and the configured
        delta/ATM conventions) and builds per-expiry smile sections by
        interpolating/extrapolating the input volatility matrix.

        Use-cases:
        - Building FX/equity volatility surfaces from market delta quotes.
        - Creating per-expiry SmileSections for pricing and risk calculations.
    */
    class BlackVolatilitySurfaceDelta : public BlackVolatilityTermStructure {
    public:
        //! Supported interpolation methods for the smile sections
        enum class SmileInterpolationMethod { Linear, NaturalCubic, FinancialCubic, CubicSpline };
        
        //! \name Constructors
        //@{
        /*! \brief Construct a delta-parametrized Black vol surface
            \param referenceDate    reference date for the term structure (valuation date)
            \param dates            vector of option expiry dates (must be increasing)
            \param putDeltas        vector of put-side deltas corresponding to matrix columns
            \param callDeltas       vector of call-side deltas corresponding to matrix columns
            \param hasAtm           true if an ATM column is present in the vol matrix
            \param blackVolMatrix   matrix of Black volatilities (rows = expiries,
                                    columns = deltas/ATM). Rows must match `dates`.
            \param dayCounter       day-count convention used to convert dates to times
            \param cal              calendar used for date adjustments where needed
            \param spot             handle to spot quote (used for delta->strike conversion)
            \param domesticTS       handle to domestic yield curve (discounting / forward)
            \param foreignTS        handle to foreign yield curve (for FX forward)
            \param dt               delta convention (Spot or Fwd, premium-adjusted, ...)
            \param at               ATM convention used when extracting/constructing ATM quote
            \param atmDeltaType     optional override delta-type for ATM computation
            \param switchTenor      tenor after which long-term delta/ATM conventions apply
            \param ltdt             long-term delta type (used for expiries beyond switchTenor)
            \param ltat             long-term ATM convention (used beyond switchTenor)
            \param longTermAtmDeltaType optional override for long-term ATM delta-type
            \param interpolationMethod interpolation used to build SmileSection for each expiry
                                       can be choosen across the following interpolation methods:
                                       Linear, NaturalCubic, FinancialCubic, CubicSpline
            \param flatStrikeExtrapolation enable flat-vol extrapolation in strike dimension
            \param timeExtrapolation policy for extrapolating/interpolating in time
        */
        BlackVolatilitySurfaceDelta(Date referenceDate, const std::vector<Date>& dates, const std::vector<Real>& putDeltas,
                                    const std::vector<Real>& callDeltas, bool hasAtm, const Matrix& blackVolMatrix,
                                    const DayCounter& dayCounter, const Calendar& cal, const Handle<Quote>& spot,
                                    const Handle<YieldTermStructure>& domesticTS,
                                    const Handle<YieldTermStructure>& foreignTS,
                                    DeltaVolQuote::DeltaType dt = DeltaVolQuote::DeltaType::Spot,
                                    DeltaVolQuote::AtmType at = DeltaVolQuote::AtmType::AtmDeltaNeutral,
                                    ext::optional<DeltaVolQuote::DeltaType> atmDeltaType = ext::nullopt,
                                    const Period& switchTenor = 0 * Days,
                                    DeltaVolQuote::DeltaType ltdt = DeltaVolQuote::DeltaType::Fwd,
                                    DeltaVolQuote::AtmType ltat = DeltaVolQuote::AtmType::AtmDeltaNeutral,
                                    ext::optional<DeltaVolQuote::DeltaType> longTermAtmDeltaType = ext::nullopt,
                                    SmileInterpolationMethod interpolationMethod =
                                        SmileInterpolationMethod::Linear,
                                    bool flatStrikeExtrapolation = true,
                                    BlackVolTimeExtrapolation timeExtrapolation =
                                        BlackVolTimeExtrapolation::FlatVolatility);
        //@}

        //! \name TermStructure interface
        //@{
        Date maxDate() const override { return Date::maxDate(); }
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override { return 0; }
        Real maxStrike() const override { return QL_MAX_REAL; }
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&) override;
        //@}

        //! \name Inspectors
        //@{
        const std::vector<Date>& dates() const { return dates_; }
        //@}

        //! Return an FxSmile for the time t
        /*!
          \brief Build and return a SmileSection representing the Black vol smile
                 at time t (year fraction from the reference date).

          The returned SmileSection provides volatilities as a function of strike
          for the requested expiry time. The SmileSection is constructed from
          the underlying delta-based surface data and the interpolation method
          configured for this surface.

          \note The produced SmileSection does not hold live references to the
                spot or yield-term-structure handles used to construct the
                BlackVolatilitySurfaceDelta. As a consequence, the SmileSection
                will not update if those handles change; callers that require
                automatic updates should recreate the SmileSection.

          \param t  time to expiry expressed as a year fraction from the reference date
          \return    a shared_ptr to a SmileSection describing the Black vol smile at time t

          \throws Error if t is out of the supported time range and extrapolation
                 is disabled.
        */
        ext::shared_ptr<SmileSection> blackVolSmile(Time t) const;

        //! Return an FxSmile for Date d
        /*!
          \brief Convenience overload that resolves a calendar Date to time and
                 returns the corresponding SmileSection.

          \param d  expiry date
          \return   a shared_ptr to a SmileSection describing the Black vol smile at date d

          \throws Error if the date cannot be converted to a valid time-to-expiry
                 or if the resulting time is out of range and extrapolation is disabled.
        */
        ext::shared_ptr<SmileSection> blackVolSmile(const Date& d) const;

    protected:
        virtual Volatility blackVolImpl(Time t, Real strike) const override;

    private:
        std::vector<Date> dates_;
        std::vector<Time> times_;

        std::vector<Real> putDeltas_;
        std::vector<Real> callDeltas_;
        bool hasAtm_;
        std::vector<ext::shared_ptr<BlackVarianceCurve> > interpolators_;

        Handle<Quote> spot_;
        Handle<YieldTermStructure> domesticTS_;
        Handle<YieldTermStructure> foreignTS_;

        DeltaVolQuote::DeltaType dt_;
        DeltaVolQuote::AtmType at_;
        ext::optional<DeltaVolQuote::DeltaType> atmDeltaType_;
        Period switchTenor_;
        DeltaVolQuote::DeltaType ltdt_;
        DeltaVolQuote::AtmType ltat_;
        ext::optional<DeltaVolQuote::DeltaType> longTermAtmDeltaType_;

        SmileInterpolationMethod interpolationMethod_;
        bool flatStrikeExtrapolation_;
        BlackVolTimeExtrapolation timeExtrapolation_;
        Real switchTime_;

        // calculate forward for time $t$
        Real forward(Time t) const;
    };

    // inline definitions

    inline void BlackVolatilitySurfaceDelta::accept(AcyclicVisitor& v) {
        Visitor<BlackVolatilitySurfaceDelta>* v1 = dynamic_cast<Visitor<BlackVolatilitySurfaceDelta>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            BlackVolatilityTermStructure::accept(v);
    }

} // namespace QuantLib

#endif
