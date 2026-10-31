/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Raghav Subbarao

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

/*! \file fxcostsmilesection.hpp
    \brief FX cost-model smile sections (flat, scaled, and general-alpha dynamics).
*/

#ifndef quantlib_cost_fx_smile_section_hpp
#define quantlib_cost_fx_smile_section_hpp

#include <ql/math/array.hpp>
#include <ql/math/polynomialmathfunction.hpp>
#include <ql/math/quadratic.hpp>
#include <ql/experimental/fxslv/fxsmilesectionbystrike.hpp>
#include <ql/experimental/fxslv/fxstickydeltasolver.hpp>

namespace QuantLib {

    //! Cubic polynomial with its real roots
    /*! Coefficients are given in increasing powers. */
    class CubicPolynomial : public PolynomialFunction {
      public:
        explicit CubicPolynomial(const std::vector<Real>& coeff) : PolynomialFunction(coeff) {
            QL_REQUIRE(coeff.size() == 4, "cubic requires four coefficients to initialize");
        }

        //! Replaces \c zeros with the real roots and returns their number.
        Integer roots(std::vector<Real>& zeros);
    };

    //! Quartic polynomial with its real roots
    /*! Coefficients are given in increasing powers. */
    class QuarticPolynomial : public PolynomialFunction {
      public:
        explicit QuarticPolynomial(const std::vector<Real>& coeff) : PolynomialFunction(coeff) {
            QL_REQUIRE(coeff.size() == 5, "quartic requires five coefficients to initialize");
        }

        //! Replaces \c zeros with the real roots and returns their number.
        Integer roots(std::vector<Real>& zeros);
    };

    /*! Base class for FX cost-model smile sections.

        Calibrates the four cost parameters [Cgamma, Ctheta, Ceta, Comega] via
        weighted SVD regression on the quoted market smile, for a general power-law
        dynamics exponent alpha in [0,1]:

            d(rho) = c * rho^alpha

        The default volByStrikeImpl() solves the cost equation by Newton-Raphson.
        Derived classes may override volByStrikeImpl() with closed-form solvers when
        alpha takes a special value (0 → quartic, 1 → biquadratic/quadratic).
    */
    class FxCostSmileSection : public FxSmileSectionByStrike {
      public:
        //! Date mode (see FxSmileSection).
        FxCostSmileSection(const Date& exerciseDate,
                           const Handle<Quote>& spot,
                           const ext::shared_ptr<FxSmileQuotes>& quotes,
                           const Handle<YieldTermStructure>& foreignDiscount,
                           const Handle<YieldTermStructure>& domesticDiscount,
                           DeltaVolQuote::DeltaType deltaType,
                           DeltaVolQuote::AtmType atmType,
                           Real alpha,
                           const DayCounter& dayCounter,
                           const FxSettleConvention& settlement,
                           const Date& referenceDate = Date(),
                           bool weightedCalibrationFlag = true);

        //! Time mode (see FxSmileSection).
        FxCostSmileSection(Time exerciseTime,
                           const Handle<Quote>& spot,
                           const ext::shared_ptr<FxSmileQuotes>& quotes,
                           const Handle<YieldTermStructure>& foreignDiscount,
                           const Handle<YieldTermStructure>& domesticDiscount,
                           DeltaVolQuote::DeltaType deltaType,
                           DeltaVolQuote::AtmType atmType,
                           Real alpha = 0.5,
                           const DayCounter& dayCounter = DayCounter(),
                           bool weightedCalibrationFlag = true);

        bool weightedCalibration() const { return weightedCalibrationFlag_; }
        Real alpha() const { return alpha_; }
        //! Fitted costs \f$ [C_\gamma, C_\theta, C_\eta, C_\omega] \f$.
        const std::vector<Real>& costs() const { calculate(); return params_; }

        //! Regression of the total vol shock on the forward move
        /*! With dynamics \f$ \Delta\rho = \rho^\alpha \Delta\tilde\rho \f$ the
            costs give \f$ E[\Delta\tilde\rho \,|\, dF/F] = (C_\eta / C_\gamma)\, dF/F \f$,
            so the ATM vol moves by
            \f$ (C_\eta / C_\gamma)\, \rho_{ATM}^\alpha / \sqrt{\tau} \f$
            per unit \f$ dF/F \f$.  With flat dynamics the move is parallel in
            total vol, i.e. an ATM move with unchanged risk reversals and
            butterflies.
        */
        Real atmVolForwardSensitivity() const override;

      private:
        //! \name FxSmileSection interface
        //@{
        virtual void calibrate() const override;
        //@}

        //! \name FxSmileSectionByStrike interface
        //@{
        Volatility volByStrikeImpl(Real strike,
                                   Real fwd,
                                   Time tau,
                                   const std::vector<Real>& params) const override;
        //@}

        bool weightedCalibrationFlag_;
        Real alpha_;

      protected:
        Array initialParams() const override;
    };

    /*! FX cost smile section with alpha = 0 (flat / constant-vol dynamics).

        The cost equation reduces to a quartic in total vol rho, which is solved
        analytically (Ferrari's method).  Calibration is inherited from the base
        class using alpha = 0.
    */
    class FxCostSmileSectionFlatDynamics : public FxCostSmileSection {
      public:
        //! Date mode (see FxSmileSection).
        FxCostSmileSectionFlatDynamics(const Date& exerciseDate,
                                       const Handle<Quote>& spot,
                                       const ext::shared_ptr<FxSmileQuotes>& quotes,
                                       const Handle<YieldTermStructure>& foreignDiscount,
                                       const Handle<YieldTermStructure>& domesticDiscount,
                                       DeltaVolQuote::DeltaType deltaType,
                                       DeltaVolQuote::AtmType atmType,
                                       const DayCounter& dayCounter,
                                       const FxSettleConvention& settlement,
                                       const Date& referenceDate = Date(),
                                       bool weightedCalibrationFlag = true);

        //! Time mode (see FxSmileSection).
        FxCostSmileSectionFlatDynamics(Time exerciseTime,
                                       const Handle<Quote>& spot,
                                       const ext::shared_ptr<FxSmileQuotes>& quotes,
                                       const Handle<YieldTermStructure>& foreignDiscount,
                                       const Handle<YieldTermStructure>& domesticDiscount,
                                       DeltaVolQuote::DeltaType deltaType,
                                       DeltaVolQuote::AtmType atmType,
                                       const DayCounter& dayCounter = DayCounter(),
                                       bool weightedCalibrationFlag = true);

      private:
        //! \name FxSmileSectionByStrike interface
        //@{
        Volatility volByStrikeImpl(Real strike,
                                   Real fwd,
                                   Time tau,
                                   const std::vector<Real>& params) const override;
        //@}
    };

    /*! FX cost smile section with alpha = 1 (scaled / log-normal total-vol dynamics).

        The cost equation reduces to a biquadratic (quadratic in rho^2), which is solved
        analytically.  Calibration is inherited from the base class using alpha = 1.
    */
    class FxCostSmileSectionScaledDynamics : public FxCostSmileSection {
      public:
        //! Date mode (see FxSmileSection).
        FxCostSmileSectionScaledDynamics(const Date& exerciseDate,
                                         const Handle<Quote>& spot,
                                         const ext::shared_ptr<FxSmileQuotes>& quotes,
                                         const Handle<YieldTermStructure>& foreignDiscount,
                                         const Handle<YieldTermStructure>& domesticDiscount,
                                         DeltaVolQuote::DeltaType deltaType,
                                         DeltaVolQuote::AtmType atmType,
                                         const DayCounter& dayCounter,
                                         const FxSettleConvention& settlement,
                                         const Date& referenceDate = Date(),
                                         bool weightedCalibrationFlag = true);

        //! Time mode (see FxSmileSection).
        FxCostSmileSectionScaledDynamics(Time exerciseTime,
                                         const Handle<Quote>& spot,
                                         const ext::shared_ptr<FxSmileQuotes>& quotes,
                                         const Handle<YieldTermStructure>& foreignDiscount,
                                         const Handle<YieldTermStructure>& domesticDiscount,
                                         DeltaVolQuote::DeltaType deltaType,
                                         DeltaVolQuote::AtmType atmType,
                                         const DayCounter& dayCounter = DayCounter(),
                                         bool weightedCalibrationFlag = true);

      private:
        //! \name FxSmileSectionByStrike interface
        //@{
        Volatility volByStrikeImpl(Real strike,
                                   Real fwd,
                                   Time tau,
                                   const std::vector<Real>& params) const override;
        //@}
    };

    /*! FX cost smile section with flat (alpha = 0) total-vol dynamics and a
        sticky-delta smile.

        The smile moves with the forward: the change in total vol at a fixed
        strike is \f$ \Delta w = \Delta \tilde w - \beta \Delta F / F \f$
        with \f$ \beta = \partial w / \partial \ln K \f$ the local slope of
        the smile.  The cost equation then involves \f$ \beta \f$ and is an
        ODE for the smile, which FxStickyDeltaSmileSolver integrates from the
        strike where \f$ d_- = 0 \f$.

        Calibration starts from a linear (SVD) fit of the cost equation with
        slopes taken from finite differences of the quoted smile, then fits
        the cost ratios \f$ (C_\theta, C_\eta, C_\omega) / C_\gamma \f$ so
        that the ODE smile reprices the quotes, by Levenberg-Marquardt.  Like
        the other cost models it needs quotes that are points on the smile
        (delta-vol quotes or smile strangles).
    */
    class FxCostSmileSectionStickyDelta : public FxCostSmileSection {
      public:
        //! Date mode (see FxSmileSection).
        FxCostSmileSectionStickyDelta(const Date& exerciseDate,
                                      const Handle<Quote>& spot,
                                      const ext::shared_ptr<FxSmileQuotes>& quotes,
                                      const Handle<YieldTermStructure>& foreignDiscount,
                                      const Handle<YieldTermStructure>& domesticDiscount,
                                      DeltaVolQuote::DeltaType deltaType,
                                      DeltaVolQuote::AtmType atmType,
                                      const DayCounter& dayCounter,
                                      const FxSettleConvention& settlement,
                                      const Date& referenceDate = Date(),
                                      bool weightedCalibrationFlag = true);

        //! Time mode (see FxSmileSection).
        FxCostSmileSectionStickyDelta(Time exerciseTime,
                                      const Handle<Quote>& spot,
                                      const ext::shared_ptr<FxSmileQuotes>& quotes,
                                      const Handle<YieldTermStructure>& foreignDiscount,
                                      const Handle<YieldTermStructure>& domesticDiscount,
                                      DeltaVolQuote::DeltaType deltaType,
                                      DeltaVolQuote::AtmType atmType,
                                      const DayCounter& dayCounter = DayCounter(),
                                      bool weightedCalibrationFlag = true);

        //! Local slope \f$ \beta = \partial w / \partial \ln K \f$ of the total vol smile.
        Real slope(Rate strike) const;
        //! Convexity \f$ \kappa \f$ of the smile dynamics (see FxStickyDeltaSmile::convexity()).
        /*! Zero for these dynamics; see FxCostSmileSectionStickyDeltaConvex. */
        Real convexity(Rate strike) const;
        //! Exact derivative of the vol with respect to strike, \f$ \beta / (K \sqrt{\tau}) \f$.
        Real volDerivative(Rate strike) const override;

        //! Half-width of the integrated smile in log-moneyness, in reference total vols.
        /*! Beyond it (about \f$ |d_\pm| > 8 \f$) the smile is extrapolated linearly. */
        static constexpr Real smileRange = 8.0;

      protected:
        //! \name FxSmileSection interface
        /*! The parameters are the four costs
            \f$ [C_\gamma, C_\theta, C_\eta, C_\omega] \f$; since only their
            ratios matter, the fit fixes \f$ C_\gamma = 1 \f$ and searches
            \f$ (C_\theta, C_\eta, C_\omega) / C_\gamma \f$.
        */
        //@{
        Array initialParams() const override;
        void setParams(const Array& params) const override;
        //@}

        //! Smile for the given costs, solved over smileRange.
        virtual ext::shared_ptr<const FxStickyDeltaSmile>
        makeSolver(const std::vector<Real>& costs, Time tau) const;
        //! Half-width in log-moneyness of the solved smile for an expiry time.
        Real solvedRange(Time tau) const;

      private:
        //! \name FxSmileSection interface
        //@{
        void calibrate() const override;
        //@}

        //! \name FxSmileSectionByStrike interface
        //@{
        Volatility volByStrikeImpl(Real strike,
                                   Real fwd,
                                   Time tau,
                                   const std::vector<Real>& params) const override;
        //@}

        // smile of the current params_, rebuilt by setParams()
        mutable ext::shared_ptr<const FxStickyDeltaSmile> solver_;
    };

    /*! FX cost smile section with flat (alpha = 0) total-vol dynamics, a
        sticky-delta smile and the convexity of exact sticky moneyness.

        As FxCostSmileSectionStickyDelta, except that the change in total
        vol at a fixed strike gains \f$ \kappa (\Delta F/F)^2 \f$ with
        \f$ \kappa = (\beta + w'')/2 \f$, given by the smile itself.  The
        cost equation is then a second-order boundary value problem, solved
        by FxStickyDeltaConvexSmileSolver; there is no extra parameter.
    */
    class FxCostSmileSectionStickyDeltaConvex : public FxCostSmileSectionStickyDelta {
      public:
        //! Date mode (see FxSmileSection).
        FxCostSmileSectionStickyDeltaConvex(const Date& exerciseDate,
                                            const Handle<Quote>& spot,
                                            const ext::shared_ptr<FxSmileQuotes>& quotes,
                                            const Handle<YieldTermStructure>& foreignDiscount,
                                            const Handle<YieldTermStructure>& domesticDiscount,
                                            DeltaVolQuote::DeltaType deltaType,
                                            DeltaVolQuote::AtmType atmType,
                                            const DayCounter& dayCounter,
                                            const FxSettleConvention& settlement,
                                            const Date& referenceDate = Date(),
                                            bool weightedCalibrationFlag = true);

        //! Time mode (see FxSmileSection).
        FxCostSmileSectionStickyDeltaConvex(Time exerciseTime,
                                            const Handle<Quote>& spot,
                                            const ext::shared_ptr<FxSmileQuotes>& quotes,
                                            const Handle<YieldTermStructure>& foreignDiscount,
                                            const Handle<YieldTermStructure>& domesticDiscount,
                                            DeltaVolQuote::DeltaType deltaType,
                                            DeltaVolQuote::AtmType atmType,
                                            const DayCounter& dayCounter = DayCounter(),
                                            bool weightedCalibrationFlag = true);

      protected:
        ext::shared_ptr<const FxStickyDeltaSmile>
        makeSolver(const std::vector<Real>& costs, Time tau) const override;
    };
} // namespace QuantLib

#endif
