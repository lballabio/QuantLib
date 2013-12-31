/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file markovfunctional.hpp
    \brief Markov Functional 1 Factor Model
*/

#ifndef quantlib_markovfunctional_hpp
#define quantlib_markovfunctional_hpp

#include <ql/experimental/models/gaussian1dmodel.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/termstructures/volatility/optionlet/optionletvolatilitystructure.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/termstructures/volatility/sabrinterpolatedsmilesection.hpp>

#include <ql/experimental/models/mfstateprocess.hpp>
#include <ql/experimental/models/kahalesmilesection.hpp>
#include <ql/experimental/models/atmadjustedsmilesection.hpp>
#include <ql/experimental/models/atmsmilesection.hpp>

namespace QuantLib {

    /*! One factor Markov Functional model class. Some documentation is
        available here
        http://ssrn.com/abstract_id=2183721
        http://quantlib.org/slides/qlws13/caspers.pdf
    */

    /*! The model requires a suitable input smile which means it should be
      arbitrage free, smooth (at least implying a C^1 call price function) and
      with a call price function not decreasing too slow in strike  direction.

      A method for arbitrage free extra- and interpolation due to Kahale is
      provided and may be used to improve an input smile. Alternatively a
      SABR smile with arbitrage free wings can be fitted to the input smile
      to provide an appropriate input smile.

      If you use the Kahale or SABR method for smile pretreatment then this
      implies zero density for negative  underlying rates. This means that
      in this case the market yield term structure must imply positive
      underlying atm forward rates. In principle the mf model is able to produce
      negative rates. To make this work the smileSection provided as input must
      have an digitalOptionPrice (or an optionPrice) implementation that is
      consistent with such a yield term structure and the model setting
      lowerRateBound must be set appropriately as a lower limit for the
      underlying rates.

      If you do not use a smile pretreatment you should ensure that the input
      smileSection is arbitrage free and  that the input smileSection covers the
      strikes from lowerRateBound to upperRateBound.

      During calibration a monocurve setup is assumed with the given yield term
      structure determining the rates throughout, no matter what curves are
      linked to the indices in the volatility term structures. The yield term
      structure should therefore be the main risk curve, i.e. the forwarding curve
      for the respective swaption or cap underlyings.

      The model uses a simplified formula for the npv of a swaps floating leg
      namely $P(t,T_0)-P(t,T_1)$ with  $T_0$ being the start date of the leg
      and $T_1$ being the last payment date, which is an approximation to the
      true npv.

      The model calibrates to slightly modified market options in the sense that
      the start date is set equal to the  fixing date, i.e. there is no delay.
      The model diagnostic outputs refer to this modified instrument. In general
      the actual market instrument including the delay is still matched very
      well though the calibration is done on a slightly different instrument.

      AdjustYts and AdjustDigitals are experimental options. Specifying
      AdjustYts may have a negative impact on the volatility smile match, so
      it should be used with special care. For long term calibration it seems
      an interesting option though.

      A bad fit to the initial yield term structure may be due to a non suitable
      input smile or accumulating numerical errors in very long term calibrations.
      The former point is adressed by smile pretreatment options. The latter point
      may be tackled by higher values for the numerical parameters possibly
      together with NTL high precision computing. */

    class MarkovFunctional : public Gaussian1dModel, public CalibratedModel {

      public:
        struct ModelSettings {

            // NoPayoffExtrapolation overrides ExtrapolatePayoffFlat
            enum Adjustments {
                AdjustNone = 0,
                AdjustDigitals = 1 << 0,
                AdjustYts = 1 << 1,
                ExtrapolatePayoffFlat = 1 << 2,
                NoPayoffExtrapolation = 1 << 3,
                KahaleSmile = 1 << 4,
                SmileExponentialExtrapolation = 1 << 5,
                KahaleInterpolation = 1 << 6,
                SmileDeleteArbitragePoints = 1 << 7,
                SabrSmile = 1 << 8
            };

            ModelSettings()
                : yGridPoints_(64), yStdDevs_(7.0), gaussHermitePoints_(32),
                  digitalGap_(1E-5), marketRateAccuracy_(1E-7),
                  lowerRateBound_(0.0), upperRateBound_(2.0),
                  adjustments_(KahaleSmile | SmileExponentialExtrapolation),
                  smileMoneynessCheckpoints_(std::vector<Real>()) {}

            void validate() {

                if ((adjustments_ & KahaleInterpolation) != 0)
                    addAdjustment(KahaleSmile);

                if ((adjustments_ & KahaleSmile) != 0 &&
                    (adjustments_ & SmileDeleteArbitragePoints)) {
                    addAdjustment(KahaleInterpolation);
                }

                QL_REQUIRE((adjustments_ & SabrSmile) == 0 ||
                               (adjustments_ & KahaleSmile) == 0,
                           "KahaleSmile and SabrSmile can not specified at the "
                           "same time");
                QL_REQUIRE(yGridPoints_ > 0, "At least one grid point ("
                                                 << yGridPoints_
                                                 << ") for the state process "
                                                    "discretization must be "
                                                    "given");
                QL_REQUIRE(yStdDevs_ > 0.0,
                           "Multiple of standard deviations covered by state "
                           "process discretization ("
                               << yStdDevs_ << ") must be positive");
                QL_REQUIRE(gaussHermitePoints_ > 0,
                           "Number of gauss hermite integration points ("
                               << gaussHermitePoints_ << ") must be positive");
                QL_REQUIRE(digitalGap_ > 0.0, "Digital gap ("
                                                  << digitalGap_
                                                  << ") must be positive");
                QL_REQUIRE(marketRateAccuracy_ > 0.0,
                           "Market rate accuracy (" << marketRateAccuracy_
                                                    << ") must be positive");
                QL_REQUIRE(
                    (adjustments_ & KahaleSmile) == 0 || lowerRateBound_ == 0.0,
                    "If Kahale extrapolation is used, the lower rate bound ("
                        << lowerRateBound_ << ") must be zero.");
                QL_REQUIRE(
                    lowerRateBound_ < upperRateBound_,
                    "Lower rate bound ("
                        << lowerRateBound_
                        << ") must be strictly less than upper rate bound ("
                        << upperRateBound_ << ")");
            }

            ModelSettings &withYGridPoints(Size n) {
                yGridPoints_ = n;
                return *this;
            }
            ModelSettings &withYStdDevs(Real s) {
                yStdDevs_ = s;
                return *this;
            }
            ModelSettings &withGaussHermitePoints(Size n) {
                gaussHermitePoints_ = n;
                return *this;
            }
            ModelSettings &withDigitalGap(Real d) {
                digitalGap_ = d;
                return *this;
            }
            ModelSettings &withMarketRateAccuracy(Real a) {
                marketRateAccuracy_ = a;
                return *this;
            }
            ModelSettings &withUpperRateBound(Real u) {
                upperRateBound_ = u;
                return *this;
            }
            ModelSettings &withLowerRateBound(Real l) {
                lowerRateBound_ = l;
                return *this;
            }
            ModelSettings &withAdjustments(int a) {
                adjustments_ = a;
                return *this;
            }
            ModelSettings &addAdjustment(int a) {
                adjustments_ |= a;
                return *this;
            }
            ModelSettings &removeAdjustment(int a) {
                adjustments_ &= ~a;
                return *this;
            }
            ModelSettings &withSmileMoneynessCheckpoints(std::vector<Real> m) {
                smileMoneynessCheckpoints_ = m;
                return *this;
            }

            Size yGridPoints_;
            Real yStdDevs_;
            Size gaussHermitePoints_;
            Real digitalGap_, marketRateAccuracy_;
            Real lowerRateBound_, upperRateBound_;
            int adjustments_;
            std::vector<Real> smileMoneynessCheckpoints_;
        };

        struct CalibrationPoint {
            bool isCaplet_;
            Period tenor_;
            std::vector<Date> paymentDates_;
            std::vector<Real> yearFractions_;
            Real atm_;
            Real annuity_;
            boost::shared_ptr<SmileSection> smileSection_;
            boost::shared_ptr<SmileSection> rawSmileSection_;
            Real minRateDigital_;
            Real maxRateDigital_;
        };

// utility macro to write messages to the model outputs

#define QL_MFMESSAGE(o, message)                                               \
    {                                                                          \
        std::ostringstream os;                                                 \
        os << message;                                                         \
        o.messages_.push_back(os.str());                                       \
    }

        struct ModelOutputs {
            bool dirty_;
            ModelSettings settings_;
            std::vector<Date> expiries_;
            std::vector<Period> tenors_;
            std::vector<Real> atm_;
            std::vector<Real> annuity_;
            std::vector<Real> adjustmentFactors_;
            std::vector<Real> digitalsAdjustmentFactors_;
            std::vector<std::string> messages_;
            std::vector<std::vector<Real> > smileStrikes_;
            std::vector<std::vector<Real> > marketRawCallPremium_;
            std::vector<std::vector<Real> > marketRawPutPremium_;
            std::vector<std::vector<Real> > marketCallPremium_;
            std::vector<std::vector<Real> > marketPutPremium_;
            std::vector<std::vector<Real> > modelCallPremium_;
            std::vector<std::vector<Real> > modelPutPremium_;
            std::vector<std::vector<Real> > marketVega_;
            std::vector<Real> marketZerorate_;
            std::vector<Real> modelZerorate_;
        };

        // Constructor for a swaption smile calibrated model
        MarkovFunctional(const Handle<YieldTermStructure> &termStructure,
                         const Real reversion,
                         const std::vector<Date> &volstepdates,
                         const std::vector<Real> &volatilities,
                         const Handle<SwaptionVolatilityStructure> &swaptionVol,
                         const std::vector<Date> &swaptionExpiries,
                         const std::vector<Period> &swaptionTenors,
                         const boost::shared_ptr<SwapIndex> &swapIndexBase,
                         const MarkovFunctional::ModelSettings &modelSettings =
                             ModelSettings());

        // Constructor for a caplet smile calibrated model
        MarkovFunctional(const Handle<YieldTermStructure> &termStructure,
                         const Real reversion,
                         const std::vector<Date> &volstepdates,
                         const std::vector<Real> &volatilities,
                         const Handle<OptionletVolatilityStructure> &capletVol,
                         const std::vector<Date> &capletExpiries,
                         const boost::shared_ptr<IborIndex> &iborIndex,
                         const MarkovFunctional::ModelSettings &modelSettings =
                             ModelSettings());

        const ModelSettings &modelSettings() const { return modelSettings_; }
        const ModelOutputs &modelOutputs() const;

        const Date &numeraireDate() const { return numeraireDate_; }
        const Time &numeraireTime() const { return numeraireTime_; }

        const Array &volatility() const { return sigma_.params(); }

        void calibrate(
            const std::vector<boost::shared_ptr<CalibrationHelper> > &helper,
            OptimizationMethod &method, const EndCriteria &endCriteria,
            const Constraint &constraint = Constraint(),
            const std::vector<Real> &weights = std::vector<Real>(),
            const std::vector<bool> &fixParameters = std::vector<bool>()) {

            CalibratedModel::calibrate(helper, method, endCriteria, constraint,
                                       weights, fixParameters.size() == 0
                                                    ? FixedFirstVolatility()
                                                    : fixParameters);
        }

      protected:

        const Real numeraireImpl(const Time t, const Real y,
                                 const Handle<YieldTermStructure> &yts) const;

        const Real zerobondImpl(const Time T, const Time t, const Real y,
                                const Handle<YieldTermStructure> &yts) const;

        void generateArguments() {
            calculate();
            updateNumeraireTabulation();
            notifyObservers();
        }

        void update() { LazyObject::update(); }

        void performCalculations() const {
            updateSmiles();
            updateNumeraireTabulation();
        }

        Disposable<std::vector<bool> > FixedFirstVolatility() const {
            std::vector<bool> c(volatilities_.size(), false);
            c[0] = true;
            return c;
        }

      private:

        void initialize();
        void updateSmiles() const;
        void updateNumeraireTabulation() const;

        void makeSwaptionCalibrationPoint(const Date &expiry,
                                          const Period &tenor);
        void makeCapletCalibrationPoint(const Date &expiry);

        const Real marketSwapRate(const Date &expiry, const CalibrationPoint &p,
                                  const Real digitalPrice,
                                  const Real guess = 0.03) const;
        const Real marketDigitalPrice(const Date &expiry,
                                      const CalibrationPoint &p,
                                      const Option::Type &type,
                                      const Real strike) const;

        const Disposable<Array>
        deflatedZerobondArray(const Time T, const Time t, const Array &y) const;
        const Disposable<Array> numeraireArray(const Time t,
                                               const Array &y) const;
        const Disposable<Array> zerobondArray(const Time T, const Time t,
                                              const Array &y) const;

        const Real deflatedZerobond(const Time T, const Time t = 0.0,
                                    const Real y = 0.0) const;

        // the following methods (tagged internal) are indended only to produce
        // the volatility diagnostics in the model outputs
        // due to the special convention of the instruments used for numeraire
        // calibration there is on direct way to use the usual pricing engines
        // for this purpose

        const Real forwardRateInternal(
            const Date &fixing, const Date &referenceDate = Null<Date>(),
            const Real y = 0.0, const bool zeroFixingDays = false,
            boost::shared_ptr<IborIndex> iborIdx =
                boost::shared_ptr<IborIndex>()) const;

        const Real swapRateInternal(const Date &fixing, const Period &tenor,
                                    const Date &referenceDate = Null<Date>(),
                                    const Real y = 0.0,
                                    const bool zeroFixingDays = false,
                                    boost::shared_ptr<SwapIndex> swapIdx =
                                        boost::shared_ptr<SwapIndex>()) const;

        const Real
        swapAnnuityInternal(const Date &fixing, const Period &tenor,
                            const Date &referenceDate = Null<Date>(),
                            const Real y = 0.0,
                            const bool zeroFixingDays = false,
                            boost::shared_ptr<SwapIndex> swapIdx =
                                boost::shared_ptr<SwapIndex>()) const;

        const Real capletPriceInternal(
            const Option::Type &type, const Date &expiry, const Rate strike,
            const Date &referenceDate = Null<Date>(), const Real y = 0.0,
            const bool zeroFixingDays = false,
            boost::shared_ptr<IborIndex> iborIdx =
                boost::shared_ptr<IborIndex>()) const;

        const Real swaptionPriceInternal(
            const Option::Type &type, const Date &expiry, const Period &tenor,
            const Rate strike, const Date &referenceDate = Null<Date>(),
            const Real y = 0.0, const bool zeroFixingDays = false,
            boost::shared_ptr<SwapIndex> swapIdx =
                boost::shared_ptr<SwapIndex>()) const;

        class ZeroHelper;
        friend class ZeroHelper;
        class ZeroHelper {
          public:
            ZeroHelper(const MarkovFunctional *model, const Date &expiry,
                       const CalibrationPoint &p, const Real marketPrice)
                : model_(model), marketPrice_(marketPrice), expiry_(expiry),
                  p_(p) {}
            double operator()(double strike) const {
                Real modelPrice = model_->marketDigitalPrice(
                    expiry_, p_, Option::Call, strike);
                return modelPrice - marketPrice_;
            };
            const MarkovFunctional *model_;
            const Real marketPrice_;
            const Date &expiry_;
            const CalibrationPoint &p_;
        };

        ModelSettings modelSettings_;
        mutable ModelOutputs modelOutputs_;

        const bool capletCalibrated_;

        boost::shared_ptr<Matrix> discreteNumeraire_;
        // vector of interpolated numeraires in y direction for all calibration
        // times
        std::vector<boost::shared_ptr<Interpolation> > numeraire_;

        Parameter reversion_;
        Parameter &sigma_;

        std::vector<Date> volstepdates_;
        std::vector<Time> volsteptimes_;
        Array volsteptimesArray_; // FIXME this is redundant (just a copy of
                                  // volsteptimes_)
        std::vector<Real> volatilities_;

        Date numeraireDate_;
        Time numeraireTime_;

        Handle<SwaptionVolatilityStructure> swaptionVol_;
        Handle<OptionletVolatilityStructure> capletVol_;

        std::vector<Date> swaptionExpiries_, capletExpiries_;
        std::vector<Period> swaptionTenors_;
        boost::shared_ptr<SwapIndex> swapIndexBase_;
        boost::shared_ptr<IborIndex> iborIndex_;

        mutable std::map<Date, CalibrationPoint> calibrationPoints_;
        std::vector<Real> times_;
        Array y_;

        Array normalIntegralX_;
        Array normalIntegralW_;
    };

    std::ostream &operator<<(std::ostream &out,
                             const MarkovFunctional::ModelOutputs &m);
}

#endif
