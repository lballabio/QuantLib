/*
 Copyright (C) 2019 Quaternion Risk Management Ltd
 Copyright (C) 2022 Skandinaviska Enskilda Banken AB (publ)
 All rights reserved.

 This file is part of ORE, a free-software/open-source library
 for transparent pricing and risk analysis - http://opensourcerisk.org

 ORE is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program.
 The license is also available online at <http://opensourcerisk.org>

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
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
#include <ql/termstructures/volatility/equityfx/fxsmilesection.hpp>

namespace QuantLib {

    class InterpolatedFxSmileSection : public FxSmileSection, 
                                       public LazyObject {
    public:
        //! Supported interpolation methods
        enum class InterpolationMethod { Linear, NaturalCubic, FinancialCubic, CubicSpline };

        //! ctor
        InterpolatedFxSmileSection(Real spot, Real rd, Real rf, Time t, const std::vector<Real>& strikes,
                                const std::vector<Volatility>& vols, InterpolationMethod method,
                                bool flatExtrapolation = false);
        
        InterpolatedFxSmileSection(Real spot, Real rd, Real rf, Time t, const std::vector<Real>& strikes,
                                const std::vector<Handle<Quote>>& volsHandles, InterpolationMethod method,
                                bool flatExtrapolation = false);

        Volatility volatility(Real strike) const override;


        void performCalculations() const override;
        void update() override;
        //! \name Inspectors
        //@{
        const std::vector<Real>& strikes() const { return strikes_; }
        const std::vector<Volatility>& volatilities() const {
            return vols_;
         }
        //@}

    private:
        void initializeInterpolator(InterpolationMethod method) const;

        mutable Interpolation interpolator_;
        std::vector<Real> strikes_;
        mutable std::vector<Volatility> vols_;
        std::vector<Handle<Quote>> volHandles_;
        bool flatExtrapolation_;
    };

    class ConstantFxSmileSection : public FxSmileSection {
    public:
        //! ctor
        ConstantFxSmileSection(const Volatility vol) : vol_(vol) {}

        Volatility volatility(Real strike) const override { return vol_; }

        //! \name Inspectors
        //@{
        const Volatility volatility() const { return vol_; }
        //@}

    private:
        Volatility vol_;
    };

    //! Abstract Black volatility surface based on delta
    //!  \ingroup termstructures
    class BlackVolatilitySurfaceDelta : public BlackVolatilityTermStructure {
    public:
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
                                    InterpolatedFxSmileSection::InterpolationMethod interpolationMethod =
                                        InterpolatedFxSmileSection::InterpolationMethod::Linear,
                                    bool flatStrikeExtrapolation = true,
                                    BlackVolTimeExtrapolation timeExtrapolation =
                                        BlackVolTimeExtrapolation::FlatVolatility);

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
        /*! Note the smile does not observe the spot or YTS handles, it will
        *  not update when they change.
        *
        *  This is not really FX specific
        */
        ext::shared_ptr<FxSmileSection> blackVolSmile(Time t) const;

        ext::shared_ptr<FxSmileSection> blackVolSmile(const Date& d) const;

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

        InterpolatedFxSmileSection::InterpolationMethod interpolationMethod_;
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
