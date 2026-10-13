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

/*! \file fxsmilesection.hpp
    \brief Base class for FX smile sections calibrated to market quotes
*/

#ifndef quantlib_fx_smile_section_hpp
#define quantlib_fx_smile_section_hpp

#include <ql/experimental/fxslv/fxdeltaconverter.hpp>
#include <ql/experimental/fxslv/fxsettlementconvention.hpp>
#include <ql/experimental/fxslv/fxsmilequotes.hpp>
#include <ql/quotes/deltavolquote.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/option.hpp>
#include <ql/math/array.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/utilities/null.hpp>
#include <optional>

namespace QuantLib {

    //! Base class for FX smile sections calibrated to delta-quoted vols
    /*! A section can be built in one of two modes.

        <b>Date mode</b> (constructors taking an expiry date and an
        FxSettlementConvention).  The reference date \f$ t_0 \f$ is the
        trade date: by default the evaluation date, in which case the
        section floats with it; if a reference date is passed, it is
        fixed.  The exercise date is the expiry date \f$ T_e \f$.  Two
        clocks are used:
        - vol time \f$ \tau = \mathrm{yf}(t_0, T_e) \f$, measured with
          the given day counter, which is exerciseTime() and is used for
          the Black standard deviation \f$ \sigma\sqrt{\tau} \f$;
        - the rate period from the spot date \f$ s_0 \f$ to the delivery
          date \f$ T_d \f$, both given by the settlement convention.  The
          discount factors \f$ P(s_0, T_d) = P(t, T_d) / P(t, s_0) \f$
          used in the forward, in the delta conventions and in premium
          adjustment are taken from each curve by date, so they do not
          depend on the curves' reference dates or day counters.
        The forward is \f$ F = S \, P_f(s_0, T_d) / P_d(s_0, T_d) \f$.

        <b>Time mode</b> (constructors taking an expiry time).  An
        idealised setup for model work, tests and interpolation at
        arbitrary times: the time to expiry is fixed (the section does
        not float), there is no settlement lag, and the same time is
        used as vol time and as rate time, i.e.
        \f$ F = S \, P_f(\tau) / P_d(\tau) \f$ on the curves' time
        axis.  For this to be meaningful both curves must share a
        reference date and a day counter; this is checked on
        calculation.  The section's own day counter is not used in this
        mode, since no date has to be converted to a time.

        The market quotes are given as an FxSmileQuotes object: either
        FxRrBfQuotes (ATM, risk reversals and smile or broker strangles
        per delta) or FxDeltaVolQuotes (generic delta-vol quotes).  The
        section fits its smile to the delta-vol quotes that object
        produces and then derives its ATM vol from the fitted smile, so
        that atmVol() always lies on the calibrated curve.
    */
    class FxSmileSection : public SmileSection, public LazyObject {
      public:
        //! Date mode.
        FxSmileSection(const Date& exerciseDate,
                       const Handle<Quote>& spot,
                       const ext::shared_ptr<FxSmileQuotes>& quotes,
                       const Handle<YieldTermStructure>& foreignDiscount,
                       const Handle<YieldTermStructure>& domesticDiscount,
                       DeltaVolQuote::DeltaType deltaType,
                       DeltaVolQuote::AtmType atmType,
                       const DayCounter& dayCounter,
                       const FxSettleConvention& settlement,
                       const Date& referenceDate = Date());

        //! Time mode.
        FxSmileSection(Time exerciseTime,
                       const Handle<Quote>& spot,
                       const ext::shared_ptr<FxSmileQuotes>& quotes,
                       const Handle<YieldTermStructure>& foreignDiscount,
                       const Handle<YieldTermStructure>& domesticDiscount,
                       DeltaVolQuote::DeltaType deltaType,
                       DeltaVolQuote::AtmType atmType,
                       const DayCounter& dayCounter = DayCounter());

        //! \name Observer interface
        //@{
        void update() override;
        //@}

        //! \name SmileSection interface
        //@{
        //! The smile is defined for every positive strike.
        Real minStrike() const override { return QL_MIN_POSITIVE_REAL; }
        Real maxStrike() const override { return QL_MAX_REAL; }
        //! The forward, as SmileSection requires; see atmStrike() for the ATM strike.
        /*! SmileSection's pricing functions (optionPrice(), vega(),
            density(), ...) use this as the forward.
        */
        Real atmLevel() const override { return forward(); }
        //@}

        // Conventions
        DeltaVolQuote::DeltaType deltaType() const { return deltaType_; }
        DeltaVolQuote::AtmType atmType() const { return atmType_; }
        bool premiumAdjust() const {
            return (deltaType_ == DeltaVolQuote::PaSpot || deltaType_ == DeltaVolQuote::PaFwd);
        }

        // Introspection
        Handle<Quote> spot() const { return spot_; }
        //! ATM vol of the calibrated smile, under the section's ATM convention.
        /*! A value, not a quote: it changes whenever the section
            recalibrates, so observe the section to be notified.
        */
        Volatility atmVol() const { calculate(); return atmVol_; }
        //! ATM strike under the section's ATM convention, at which atmVol() is quoted.
        Rate atmStrike() const { calculate(); return atmStrike_; }
        Real forward() const { calculate(); return fwd_; }
        Handle<YieldTermStructure> foreignDiscount() const { return foreignDiscount_; }
        Handle<YieldTermStructure> domesticDiscount() const { return domesticDiscount_; }

        //! Settlement convention; empty for sections built in time mode.
        const std::optional<FxSettleConvention>& settleConvention() const { return settleConvention_; }

        Date spotDate() const;      //!< Spot date of the reference date (date mode only).
        Date deliveryDate() const;  //!< Delivery date of the expiry (date mode only).

        //! Domestic discount factor from spot to delivery (time mode: to expiry time).
        DiscountFactor domesticDiscountFactor() const { calculate(); return ddom_; }

        //! Foreign discount factor from spot to delivery (time mode: to expiry time).
        DiscountFactor foreignDiscountFactor() const { calculate(); return dfor_; }

        //! Strike and delta conversions at a given vol in the section's delta convention.
        /*! Built from the current spot and discount factors; the
            conversions along the smile are volByDelta(), strikeByDelta()
            and deltaByStrike().
        */
        FxDeltaConverter deltaConverter() const;

        //! Largest call delta in the section's delta convention.
        /*! For premium-adjusted deltas the call delta is not monotonic
            in strike: it peaks at a strike below which every call delta
            is attained twice, and higher call deltas are not attained at
            all.  For unadjusted deltas it is the limit at zero strike:
            the foreign discount factor for spot deltas, 1 for forward
            deltas.
        */
        Real maxCallDelta() const;

        //! Market quotes the section is calibrated to.
        const ext::shared_ptr<FxSmileQuotes>& smileQuotes() const { return smileQuotes_; }

        //! \name Calibration quality
        /*! A model with fewer parameters than targets cannot fit them
            all; these say how far the calibrated smile is from each.
        */
        //@{
        //! Residual of each calibration target, in vol units, in the order the quotes give them.
        const Array& calibrationResiduals() const;
        //! Root mean square of calibrationResiduals(), in vol units.
        Real calibrationError() const;
        //@}

        // Calibration
        virtual Volatility volByStrike(Rate strike) const = 0;
        virtual Volatility volByDelta(Real delta, Option::Type parity) const = 0;
        virtual Real deltaByStrike(Rate strike, Option::Type parity) const = 0;
        virtual Rate strikeByDelta(Real delta, Option::Type parity) const = 0;

        //! Residual for a calibration point (strike, vol), in vol units.
        /*! Measured in the model's natural coordinate: by default the vol
            at the strike; delta-parameterised smiles measure it at the
            point's put delta instead.
        */
        virtual Real volResidual(Rate strike, Volatility vol) const;

        // Interpolation
        //! Derivative of the vol with respect to strike.
        /*! Central finite differences by default; models with a
            closed form should override it.
        */
        virtual Real volDerivative(Rate strike) const;

        //! \name Interpolation in probability space
        /*! Functions of the moneyness \f$ k = K/F \f$, used to
            interpolate smiles across expiries.  The inverse requires a
            smile without butterfly arbitrage, which it checks.
        */
        //@{
        //! Undiscounted call price in units of the forward, \f$ c(k) = E[(S_T/F - k)^+] \f$.
        Real normedCallPrice(Real moneyness) const;
        //! Probability of exercise, \f$ P(S_T > kF) = -c'(k) \f$, computed analytically.
        Probability exerciseProbability(Real moneyness) const;
        //! Moneyness at which the exercise probability equals \f$ p \f$.
        Real moneynessFromProbability(Probability p) const;
        //@}

      private:
        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        //@}

        void registerWithMarketData();
        void calculateForward() const;
        void calculateAtm() const;
        void stripDeltaVolQuotes() const;

        // FxSmileQuotes::fit() is the only caller of fitToTargets().
        friend class FxSmileQuotes;
        //! Fits the smile to the given targets; only valid while the quotes calibrate the section.
        void fitToTargets(FxSmileTargets targets) const;
        mutable bool calibrating_ = false;  // the quotes are calibrating this section
        mutable bool fitRequested_ = false; // the quotes called fitToTargets() during this calibration

        Volatility volatilityImpl(Rate strike) const override { return volByStrike(strike); }

        DeltaVolQuote::DeltaType deltaType_;
        DeltaVolQuote::AtmType atmType_;

        Handle<Quote> spot_;
        ext::shared_ptr<FxSmileQuotes> smileQuotes_;
        Handle<YieldTermStructure> foreignDiscount_;
        Handle<YieldTermStructure> domesticDiscount_;
        std::optional<FxSettleConvention> settleConvention_;
        mutable Date spotDate_, deliveryDate_;

      protected:
        //! Fits the smile to targets_.
        /*! By default a least-squares fit of the vega-weighted target
            residuals over the model parameters, starting from
            initialParams(), which fails if it does not converge; models
            with a closed-form fit can override it, and others can extend
            it, e.g. to check the fitted parameters.
        */
        virtual void calibrate() const;

        //! Vol scale used to seed the smile: the quotes' reference vol.
        /*! Depends on the quotes only.  Initial parameters, root-search
            guesses and starting points must use this, never atmVol_, so
            that the smile does not depend on a previous calibration.
        */
        Volatility referenceVol() const { return smileQuotes_->referenceVol(); }

        //! Strike at which the premium-adjusted call delta of the current smile peaks.
        /*! Computed from the smile as it stands, so during calibration
            it is the trial smile's; the delta includes the smile's slope.
            If the delta has more than one local peak, the one found by
            searching outward from the forward is returned.  Only defined
            for premium-adjusted delta types, and only while or after the
            section calculates, since it needs the forward.
        */
        Rate peakCallDeltaStrike() const;

        //! Initial parameter guess for calibration.
        virtual Array initialParams() const = 0;
        //! Sets the model parameters; used with trial values during calibration.
        virtual void setParams(const Array& params) const = 0;

        mutable Real ddom_ = Null<Real>();
        mutable Real dfor_ = Null<Real>();
        mutable Real fwd_ = Null<Real>();

        mutable Real atmStrike_ = Null<Real>();

        // Computed state: rebuilt on every calibration in stripDeltaVolQuotes().
        // atmVol_ is the fitted smile's ATM vol, set by calculateAtm(); it is
        // Null while the smile is being fitted, and nothing that defines the
        // smile may read it (use referenceVol() instead).
        // targets_ holds the calibration targets while calibrate() runs.
        mutable Volatility atmVol_ = Null<Volatility>();
        mutable FxSmileTargets targets_;
        // unweighted residuals of the fitted smile, set by fitToTargets()
        mutable Array calibrationResiduals_;

    };

    inline void FxSmileSection::update() {
        SmileSection::update();
        LazyObject::update();
    }

}  // namespace QuantLib

#endif
