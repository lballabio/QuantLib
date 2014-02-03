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

#include <ql/experimental/models/markovfunctional.hpp>
#include <ql/experimental/models/smilesectionutils.hpp>

namespace QuantLib {

    MarkovFunctional::MarkovFunctional(
        const Handle<YieldTermStructure> &termStructure, const Real reversion,
        const std::vector<Date> &volstepdates,
        const std::vector<Real> &volatilities,
        const Handle<SwaptionVolatilityStructure> &swaptionVol,
        const std::vector<Date> &swaptionExpiries,
        const std::vector<Period> &swaptionTenors,
        const boost::shared_ptr<SwapIndex> &swapIndexBase,
        const MarkovFunctional::ModelSettings &modelSettings)
        : Gaussian1dModel(termStructure), CalibratedModel(1),
          modelSettings_(modelSettings), capletCalibrated_(false),
          reversion_(ConstantParameter(reversion, NoConstraint())),
          sigma_(arguments_[0]), volstepdates_(volstepdates),
          volatilities_(volatilities), swaptionVol_(swaptionVol),
          capletVol_(Handle<OptionletVolatilityStructure>()),
          swaptionExpiries_(swaptionExpiries),
          capletExpiries_(std::vector<Date>()), swaptionTenors_(swaptionTenors),
          swapIndexBase_(swapIndexBase),
          iborIndex_(swapIndexBase->iborIndex()) {

        QL_REQUIRE(swaptionExpiries.size() == swaptionTenors.size(),
                   "number of swaption expiries ("
                       << swaptionExpiries.size()
                       << ") is differnt from number of swaption tenors ("
                       << swaptionTenors.size() << ")");
        QL_REQUIRE(swaptionExpiries.size() >= 1,
                   "need at least one swaption expiry to calibrate numeraire");
        QL_REQUIRE(!termStructure.empty(),
                   "yield term structure handle is empty");
        QL_REQUIRE(!swaptionVol.empty(),
                   "swaption volatility structure is empty");
        modelSettings_.validate();
        initialize();
    }

    MarkovFunctional::MarkovFunctional(
        const Handle<YieldTermStructure> &termStructure, const Real reversion,
        const std::vector<Date> &volstepdates,
        const std::vector<Real> &volatilities,
        const Handle<OptionletVolatilityStructure> &capletVol,
        const std::vector<Date> &capletExpiries,
        const boost::shared_ptr<IborIndex> &iborIndex,
        const MarkovFunctional::ModelSettings &modelSettings)
        : Gaussian1dModel(termStructure), CalibratedModel(1),
          modelSettings_(modelSettings), capletCalibrated_(true),
          reversion_(ConstantParameter(reversion, NoConstraint())),
          sigma_(arguments_[0]), volstepdates_(volstepdates),
          volatilities_(volatilities),
          swaptionVol_(Handle<SwaptionVolatilityStructure>()),
          capletVol_(capletVol), swaptionExpiries_(std::vector<Date>()),
          capletExpiries_(capletExpiries),
          swaptionTenors_(std::vector<Period>()), iborIndex_(iborIndex) {

        QL_REQUIRE(capletExpiries.size() >= 1,
                   "need at least one caplet expiry to calibrate numeraire");
        QL_REQUIRE(!termStructure.empty(),
                   "yield term structure handle is empty");
        QL_REQUIRE(!capletVol.empty(), "caplet volatility structure is empty");
        modelSettings_.validate();
        initialize();
    }

    void MarkovFunctional::initialize() {

        QL_MFMESSAGE(modelOutputs_, "initializing");
        modelOutputs_.dirty_ = true;

        modelOutputs_.settings_ = modelSettings_;

        GaussHermiteIntegration gaussHermite(
            modelSettings_.gaussHermitePoints_);
        normalIntegralX_ = gaussHermite.x();
        normalIntegralW_ = gaussHermite.weights();
        for (Size i = 0; i < normalIntegralX_.size(); i++) {
            normalIntegralW_[i] *=
                exp(-normalIntegralX_[i] * normalIntegralX_[i]) * M_1_SQRTPI;
            normalIntegralX_[i] *= M_SQRT2;
        }

        volsteptimes_.clear();
        volsteptimesArray_ = Array(volstepdates_.size());
        int j = 0;
        for (std::vector<Date>::const_iterator i = volstepdates_.begin();
             i != volstepdates_.end(); i++, j++) {
            volsteptimes_.push_back(termStructure()->timeFromReference(*i));
            volsteptimesArray_[j] = volsteptimes_[j];
            if (j == 0)
                QL_REQUIRE(volsteptimes_[0] > 0.0,
                           "volsteptimes must be positive (" << volsteptimes_[0]
                                                             << ")");
            else
                QL_REQUIRE(volsteptimes_[j] > volsteptimes_[j - 1],
                           "volsteptimes must be strictly increasing ("
                               << volsteptimes_[j - 1] << "@" << (j - 1) << ", "
                               << volsteptimes_[j] << "@" << j << ")");
        }

        if (capletCalibrated_) {
            for (std::vector<Date>::const_iterator i = capletExpiries_.begin(); i != capletExpiries_.end(); i++) {
                makeCapletCalibrationPoint(*i);
            }
        } else {
            std::vector<Date>::const_iterator i;
            std::vector<Period>::const_iterator j;
            for (i = swaptionExpiries_.begin(), j = swaptionTenors_.begin();
                 i != swaptionExpiries_.end(); i++, j++) {
                makeSwaptionCalibrationPoint(*i, *j);
            }
        }

        bool done;
        numeraireDate_ = Date::minDate();
        do {
            Date numeraireKnown = numeraireDate_;
            done = true;
            for (std::map<Date, CalibrationPoint>::reverse_iterator i =
                     calibrationPoints_.rbegin();
                 i != calibrationPoints_.rend() && done; i++) {
                if (i->second.paymentDates_.back() > numeraireDate_) {
                    numeraireDate_ = i->second.paymentDates_.back();
                    numeraireKnown = i->second.paymentDates_.back();
                    if (i != calibrationPoints_.rbegin()) {
                        done = false;
                    }
                }
                // Inlining this into the loop condition causes
                // a bogus compilation error wih g++ 4.0.1 on Mac OS X
                std::vector<Date>::const_reverse_iterator rend =
                    i->second.paymentDates_.rend();
                for (std::vector<Date>::const_reverse_iterator j =
                         i->second.paymentDates_.rbegin();
                     j != rend && done; j++) {
                    if (*j < numeraireKnown) {
                        if (capletCalibrated_) {
                            makeCapletCalibrationPoint(*j);
                            done = false;
                            break;
                        } else {
                            UpRounding rounder(0);
                            makeSwaptionCalibrationPoint(
                                *j,
                                Period(
                                    static_cast<Integer>(rounder(
                                        (swapIndexBase_->dayCounter()
                                             .yearFraction(*j, numeraireKnown) -
                                         0.5 / 365) *
                                        12.0)),
                                    Months));
                            done = false;
                            break;
                        }
                    }
                }
                if (done) {
                    numeraireKnown = i->first;
                }
            }
        } while (!done);

        numeraireTime_ = termStructure()->timeFromReference(numeraireDate_);

        times_.clear();
        times_.push_back(0.0);

        modelOutputs_.expiries_.clear();
        modelOutputs_.tenors_.clear();
        for (std::map<Date, CalibrationPoint>::iterator k =
                 calibrationPoints_.begin();
             k != calibrationPoints_.end(); k++) {
            times_.push_back(termStructure()->timeFromReference(k->first));
            modelOutputs_.expiries_.push_back(k->first);
            modelOutputs_.tenors_.push_back(k->second.tenor_);
        }
        times_.push_back(numeraireTime_);

        QL_REQUIRE(volatilities_.size() == volsteptimes_.size() + 1,
                   "there must be n+1 volatilities ("
                       << volatilities_.size()
                       << ") for n volatility step times ("
                       << volsteptimes_.size() << ")");
        sigma_ =
            PiecewiseConstantParameter(volsteptimes_, PositiveConstraint());
        for (Size i = 0; i < sigma_.size(); i++) {
            sigma_.setParam(i, volatilities_[i]);
        }

        stateProcess_ = boost::shared_ptr<MfStateProcess>(new MfStateProcess(
            reversion_(0.0), volsteptimesArray_, sigma_.params()));

        y_ = yGrid(modelSettings_.yStdDevs_, modelSettings_.yGridPoints_);

        discreteNumeraire_ = boost::shared_ptr<Matrix>(new Matrix(
            times_.size(), 2 * modelSettings_.yGridPoints_ + 1, 1.0));
        for (Size i = 0; i < times_.size(); i++) {
            boost::shared_ptr<Interpolation> numInt(new CubicInterpolation(
                y_.begin(), y_.end(), discreteNumeraire_->row_begin(i),
                CubicInterpolation::Spline, true, CubicInterpolation::Lagrange,
                0.0, CubicInterpolation::Lagrange, 0.0));
            numInt->enableExtrapolation();
            numeraire_.push_back(numInt);
        }

        LazyObject::registerWith(termStructure());
        if (!swaptionVol_.empty())
            LazyObject::registerWith(swaptionVol_);
        if (!capletVol_.empty())
            LazyObject::registerWith(capletVol_);
    }

    void MarkovFunctional::makeSwaptionCalibrationPoint(const Date &expiry,
                                                        const Period &tenor) {

        QL_REQUIRE(calibrationPoints_.count(expiry) == 0,
                   "swaption expiry ("
                       << expiry
                       << ") occurs more than once in calibration set");

        CalibrationPoint p;
        p.isCaplet_ = false;
        p.tenor_ = tenor;

        SwapIndex tmpIndex(
            swapIndexBase_->familyName(), tenor, swapIndexBase_->fixingDays(),
            swapIndexBase_->currency(), swapIndexBase_->fixingCalendar(),
            swapIndexBase_->fixedLegTenor(),
            swapIndexBase_->fixedLegConvention(), swapIndexBase_->dayCounter(),
            swapIndexBase_->iborIndex());
        boost::shared_ptr<VanillaSwap> underlying =
            tmpIndex.underlyingSwap(expiry);
        Schedule sched = underlying->fixedSchedule();
        Calendar cal = sched.calendar();
        BusinessDayConvention bdc = underlying->paymentConvention();

        for (unsigned int k = 1; k < sched.size(); k++) {
            p.yearFractions_.push_back(
                swapIndexBase_->dayCounter().yearFraction(
                    k == 1 ? expiry : sched.date(k - 1), sched.date(k)));
            p.paymentDates_.push_back(cal.adjust(sched.date(k), bdc));
        }
        calibrationPoints_[expiry] = p;
    }

    void MarkovFunctional::makeCapletCalibrationPoint(const Date &expiry) {

        QL_REQUIRE(
            calibrationPoints_.count(expiry) == 0,
            "caplet expiry (" << expiry
                              << ") occurs more than once in calibration set");

        CalibrationPoint p;
        p.isCaplet_ = true;
        // p.expiry_ = expiry;
        p.tenor_ = iborIndex_->tenor();
        Date valueDate = iborIndex_->valueDate(expiry);
        Date endDate = iborIndex_->fixingCalendar().advance(
            valueDate, iborIndex_->tenor(), iborIndex_->businessDayConvention(),
            iborIndex_->endOfMonth());
        // FIXME Here we should use a calculation date calendar ?
        p.paymentDates_.push_back(endDate);
        p.yearFractions_.push_back(
            iborIndex_->dayCounter().yearFraction(expiry, endDate));
        // adjust the first period to start on expiry
        calibrationPoints_[expiry] = p;
    }

    void MarkovFunctional::updateSmiles() const {

        QL_MFMESSAGE(modelOutputs_, "updating smiles");
        modelOutputs_.dirty_ = true;

        for (std::map<Date, CalibrationPoint>::reverse_iterator i =
                 calibrationPoints_.rbegin();
             i != calibrationPoints_.rend(); i++) {

            boost::shared_ptr<SmileSection> smileSection;
            if (i->second.isCaplet_) {
                i->second.annuity_ =
                    i->second.yearFractions_[0] *
                    termStructure()->discount(i->second.paymentDates_[0], true);
                i->second.atm_ = (termStructure()->discount(i->first, true) -
                                  termStructure()->discount(
                                      i->second.paymentDates_[0], true)) /
                                 i->second.annuity_;
                smileSection = capletVol_->smileSection(i->first, true);
            } else {
                Real annuity = 0.0;
                for (unsigned int k = 0; k < i->second.paymentDates_.size();
                     k++) {
                    annuity += i->second.yearFractions_[k] *
                               termStructure()->discount(
                                   i->second.paymentDates_[k], true);
                }
                i->second.annuity_ = annuity;
                i->second.atm_ = (termStructure()->discount(i->first, true) -
                                  termStructure()->discount(
                                      i->second.paymentDates_.back(), true)) /
                                 annuity;
                smileSection = swaptionVol_->smileSection(
                    i->first, i->second.tenor_, true);
            }

            i->second.rawSmileSection_ = boost::shared_ptr<SmileSection>(
                new AtmSmileSection(smileSection, i->second.atm_));

            if (modelSettings_.adjustments_ & ModelSettings::KahaleSmile) {

                i->second.smileSection_ = boost::shared_ptr<KahaleSmileSection>(
                    new KahaleSmileSection(
                        i->second.rawSmileSection_, i->second.atm_,
                        (modelSettings_.adjustments_ &
                         ModelSettings::KahaleInterpolation) != 0,
                        (modelSettings_.adjustments_ &
                         ModelSettings::SmileExponentialExtrapolation) != 0,
                        (modelSettings_.adjustments_ &
                         ModelSettings::SmileDeleteArbitragePoints) != 0,
                        modelSettings_.smileMoneynessCheckpoints_,
                        modelSettings_.digitalGap_));

            } else {

                if (modelSettings_.adjustments_ & ModelSettings::SabrSmile) {

                    SmileSectionUtils ssutils(
                        *i->second.rawSmileSection_,
                        modelSettings_.smileMoneynessCheckpoints_);
                    std::vector<Real> k = ssutils.strikeGrid();
                    k.erase(k.begin()); // the first strike is zero which we do
                                        // not want in the sabr calibration
                    QL_REQUIRE(
                        k.size() >= 4,
                        "for sabr calibration at least 4 points are needed (is "
                            << k.size() << ")");
                    std::vector<Real> v;
                    for (Size j = 0; j < k.size(); j++) {
                        v.push_back(
                            i->second.rawSmileSection_->volatility(k[j]));
                    }

                    // TODO should we fix beta to avoid numerical instabilities
                    // during calibration ?
                    boost::shared_ptr<SabrInterpolatedSmileSection>
                    sabrSection(new SabrInterpolatedSmileSection(
                        i->first, i->second.atm_, k, false,
                        i->second.rawSmileSection_->volatility(i->second.atm_),
                        v, 0.03, 0.80, 0.50, 0.00, false, false, false, false));

                    // we make the sabr section arbitrage free by superimposing
                    // a kahalesection

                    i->second.smileSection_ = boost::shared_ptr<
                        KahaleSmileSection>(new KahaleSmileSection(
                        sabrSection, i->second.atm_, false,
                        (modelSettings_.adjustments_ &
                         ModelSettings::SmileExponentialExtrapolation) != 0,
                        (modelSettings_.adjustments_ &
                         ModelSettings::SmileDeleteArbitragePoints) != 0,
                        modelSettings_.smileMoneynessCheckpoints_,
                        modelSettings_.digitalGap_));

                } else { // no smile pretreatment

                    i->second.smileSection_ = i->second.rawSmileSection_;
                }
            }

            i->second.minRateDigital_ =
                i->second.smileSection_->digitalOptionPrice(
                    modelSettings_.lowerRateBound_, Option::Call,
                    i->second.annuity_, modelSettings_.digitalGap_);
            i->second.maxRateDigital_ =
                i->second.smileSection_->digitalOptionPrice(
                    modelSettings_.upperRateBound_, Option::Call,
                    i->second.annuity_, modelSettings_.digitalGap_);

            // output smile for testing
            // boost::shared_ptr<SmileSection> sec1 =
            // i->second.rawSmileSection_;
            // boost::shared_ptr<KahaleSmileSection> sec2 =
            //     boost::dynamic_pointer_cast<KahaleSmileSection>(i->second.smileSection_);
            // const std::vector<double> &money =
            // modelSettings_.smileMoneynessCheckpoints_;
            // SmileSectionUtils sutils(*sec1, money);
            // std::cout
            //     <<
            // "-------------------------------------------------------------------"
            //     << std::endl;
            // std::cout << "Smile for expiry " << i->first << " tenor " <<
            // i->second.tenor_
            //           << " atm is " << i->second.atm_ << std::endl;
            // std::cout << "Arbitrage free region " <<
            // sutils.arbitragefreeRegion().first
            //           << " ... " << sutils.arbitragefreeRegion().second <<
            // std::endl;
            // if (sec2)
            //     std::cout << "Kahale core region    " <<
            // sec2->leftCoreStrike()
            //               << " ... " << sec2->rightCoreStrike() << std::endl;
            // std::cout <<
            // "strike;rawVol;rawVar;rawCall;Call;rawDigial;Digital;"
            //     "rawDensity;Density;callDiff;Arb" << std::endl;
            // Real strike = 0.00001;
            // while (strike <= 0.20 + 1E-8) {
            //     std::cout << strike << ";" << sec1->volatility(strike) << ";"
            //               << sec1->variance(strike) << ";" <<
            // sec1->optionPrice(strike)
            //               << ";" << (sec2 ? sec2->optionPrice(strike) : 0.0)
            // << ";"
            //               << sec1->digitalOptionPrice(strike) << ";"
            //               << (sec2 ? sec2->digitalOptionPrice(strike) : 0.0)
            // << ";"
            //               << sec1->density(strike) << ";"
            //               << (sec2 ? sec2->density(strike) : 0.0) << ";"
            //               << (sec2
            //                   ? sec1->optionPrice(strike) -
            // sec2->optionPrice(strike)
            //                   : 0.0) << ";" << ((sec2 ? sec2->density(strike)
            //                                      : sec1->density(strike)) <
            // 0.0
            //                                     ? "**********"
            //                                     : "") << std::endl;
            //     strike += 0.0010;
            // }
            // std::cout
            //     <<
            // "-------------------------------------------------------------------"
            //     << std::endl;
            // end output smile
        }
    }

    void MarkovFunctional::updateNumeraireTabulation() const {

        QL_MFMESSAGE(modelOutputs_, "updating numeraire tabulation");
        modelOutputs_.dirty_ = true;

        modelOutputs_.adjustmentFactors_.clear();
        modelOutputs_.digitalsAdjustmentFactors_.clear();

        int idx = times_.size() - 2;

        for (std::map<Date, CalibrationPoint>::reverse_iterator
                 i = calibrationPoints_.rbegin();
             i != calibrationPoints_.rend(); i++, idx--) {

            Array discreteDeflatedAnnuities(y_.size(), 0.0);
            Array deflatedFinalPayments;

            Real numeraire0 = termStructure()->discount(numeraireTime_, true);
            Real normalization =
                termStructure()->discount(times_[idx], true) / numeraire0;

            for (unsigned int k = 0; k < i->second.paymentDates_.size(); k++) {
                deflatedFinalPayments =
                    deflatedZerobondArray(termStructure()->timeFromReference(
                                              i->second.paymentDates_[k]),
                                          times_[idx], y_);
                discreteDeflatedAnnuities +=
                    deflatedFinalPayments * i->second.yearFractions_[k];
            }

            CubicInterpolation deflatedAnnuities(
                y_.begin(), y_.end(), discreteDeflatedAnnuities.begin(),
                CubicInterpolation::Spline, true, CubicInterpolation::Lagrange,
                0.0, CubicInterpolation::Lagrange, 0.0);
            deflatedAnnuities.enableExtrapolation();

            Real digitalsCorrectionFactor = 1.0;
            modelOutputs_.digitalsAdjustmentFactors_.insert(
                modelOutputs_.digitalsAdjustmentFactors_.begin(),
                digitalsCorrectionFactor);

            Real digital = 0.0, swapRate, swapRate0;

            for (int c = 0;
                 c == 0 || (c == 1 && (modelSettings_.adjustments_ &
                                       ModelSettings::AdjustDigitals));
                 c++) {

                if (c == 1) {
                    digitalsCorrectionFactor = i->second.annuity_ / digital;
                    modelOutputs_.digitalsAdjustmentFactors_.front() =
                        digitalsCorrectionFactor;
                }

                digital = 0.0;
                swapRate0 =
                    modelSettings_.upperRateBound_ / 2.0; // initial guess
                for (int j = y_.size() - 1; j >= 0; j--) {

                    Real integral = 0.0;

                    if (j == (int)(y_.size() - 1)) {
                        if ((modelSettings_.adjustments_ &
                             ModelSettings::NoPayoffExtrapolation) == 0) {
                            if ((modelSettings_.adjustments_ &
                                 ModelSettings::ExtrapolatePayoffFlat) != 0) {
                                integral = gaussianShiftedPolynomialIntegral(
                                    0.0, 0.0, 0.0, 0.0,
                                    discreteDeflatedAnnuities[j - 1], y_[j - 1],
                                    y_[j], 100.0);
                            } else {
                                Real ca =
                                    deflatedAnnuities.aCoefficients()[j - 1];
                                Real cb =
                                    deflatedAnnuities.bCoefficients()[j - 1];
                                Real cc =
                                    deflatedAnnuities.cCoefficients()[j - 1];
                                integral = gaussianShiftedPolynomialIntegral(
                                    0.0, cc, cb, ca,
                                    discreteDeflatedAnnuities[j - 1], y_[j - 1],
                                    y_[j], 100.0);
                            }
                        }
                    } else {
                        Real ca = deflatedAnnuities.aCoefficients()[j];
                        Real cb = deflatedAnnuities.bCoefficients()[j];
                        Real cc = deflatedAnnuities.cCoefficients()[j];
                        integral = gaussianShiftedPolynomialIntegral(
                            0.0, cc, cb, ca, discreteDeflatedAnnuities[j],
                            y_[j], y_[j], y_[j + 1]);
                    }

                    if (integral < 0) {
                        QL_MFMESSAGE(modelOutputs_,
                                     "WARNING: integral for digitalPrice is "
                                     "negative for j="
                                         << j << " (" << integral
                                         << ") --- reset it to zero.");
                        integral = 0.0;
                    }

                    digital += integral * numeraire0 * digitalsCorrectionFactor;

                    if (digital >= i->second.minRateDigital_)
                        swapRate = modelSettings_.lowerRateBound_;
                    else {
                        if (digital <= i->second.maxRateDigital_)
                            swapRate = modelSettings_.upperRateBound_;
                        else {
                            swapRate = marketSwapRate(i->first, i->second,
                                                      digital, swapRate0);
                            if (j < (int)y_.size() - 1 &&
                                swapRate > swapRate0) {
                                QL_MFMESSAGE(
                                    modelOutputs_,
                                    "WARNING: swap rate is decreasing in y for "
                                    "t="
                                        << times_[idx] << ", j=" << j
                                        << " (y, swap rate) is (" << y_[j]
                                        << "," << swapRate
                                        << ") but for j=" << j + 1 << " it is ("
                                        << y_[j + 1] << "," << swapRate0
                                        << ") --- reset rate to " << swapRate0
                                        << " in node j=" << j);
                                swapRate = swapRate0;
                            }
                        }
                    }
                    swapRate0 = swapRate;
                    Real numeraire =
                        1.0 / (swapRate * discreteDeflatedAnnuities[j] +
                               deflatedFinalPayments[j]);
                    (*discreteNumeraire_)[idx][j] = numeraire * normalization;
                }
            }

            if (modelSettings_.adjustments_ & ModelSettings::AdjustYts) {
                numeraire_[idx]->update();
                Real modelDeflatedZerobond = deflatedZerobond(times_[idx], 0.0);
                Real marketDeflatedZerobond =
                    termStructure()->discount(times_[idx], true) /
                    termStructure()->discount(numeraireTime_, true);
                for (int j = y_.size() - 1; j >= 0; j--) {
                    (*discreteNumeraire_)[idx][j] *=
                        modelDeflatedZerobond / marketDeflatedZerobond;
                }
                modelOutputs_.adjustmentFactors_.insert(
                    modelOutputs_.adjustmentFactors_.begin(),
                    modelDeflatedZerobond / marketDeflatedZerobond);
            } else {
                modelOutputs_.adjustmentFactors_.insert(
                    modelOutputs_.adjustmentFactors_.begin(), 1.0);
            }

            numeraire_[idx]->update();
        }
    }

    const MarkovFunctional::ModelOutputs &
    MarkovFunctional::modelOutputs() const {

        if (modelOutputs_.dirty_) {

            calculate();

            // yield term structure
            modelOutputs_.marketZerorate_.clear();
            modelOutputs_.modelZerorate_.clear();
            for (Size i = 1; i < times_.size() - 1; i++) {
                modelOutputs_.marketZerorate_.push_back(
                    termStructure()->zeroRate(times_[i], QuantLib::Continuous,
                                              QuantLib::Annual));
                modelOutputs_.modelZerorate_.push_back(
                    -std::log(zerobond(times_[i])) / times_[i]);
            }

            // volatility surface
            modelOutputs_.smileStrikes_.clear();
            modelOutputs_.marketCallPremium_.clear();
            modelOutputs_.marketPutPremium_.clear();
            modelOutputs_.modelCallPremium_.clear();
            modelOutputs_.modelPutPremium_.clear();
            modelOutputs_.marketVega_.clear();
            modelOutputs_.marketRawCallPremium_.clear();
            modelOutputs_.marketRawPutPremium_.clear();

            for (std::map<Date, CalibrationPoint>::iterator i =
                     calibrationPoints_.begin();
                 i != calibrationPoints_.end(); i++) {
                modelOutputs_.atm_.push_back(i->second.atm_);
                modelOutputs_.annuity_.push_back(i->second.annuity_);
                boost::shared_ptr<SmileSection> sec = i->second.smileSection_;
                boost::shared_ptr<SmileSection> rawSec =
                    i->second.rawSmileSection_;
                SmileSectionUtils ssutils(
                    *sec, modelSettings_.smileMoneynessCheckpoints_,
                    i->second.atm_);
                std::vector<Real> money = ssutils.moneyGrid();
                std::vector<Real> strikes, marketCall, marketPut, modelCall,
                    modelPut, marketVega, marketRawCall, marketRawPut;
                for (Size j = 0; j < money.size(); j++) {
                    strikes.push_back(money[j] * i->second.atm_);
                    try {
                        marketRawCall.push_back(rawSec->optionPrice(
                            strikes[j], Option::Call, i->second.annuity_));
                        marketRawPut.push_back(rawSec->optionPrice(
                            strikes[j], Option::Put, i->second.annuity_));
                    }
                    catch (QuantLib::Error) {
                        // the smile section might not be able to output an
                        // option price because it has no atm level
                        marketRawCall.push_back(0.0);
                        marketRawPut.push_back(0.0);
                    }
                    marketCall.push_back(sec->optionPrice(
                        strikes[j], Option::Call, i->second.annuity_));
                    marketPut.push_back(sec->optionPrice(
                        strikes[j], Option::Put, i->second.annuity_));
                    modelCall.push_back(
                        i->second.isCaplet_
                            ? capletPriceInternal(Option::Call, i->first, strikes[j],
                                             Null<Date>(), 0.0, true)
                            : swaptionPriceInternal(Option::Call, i->first,
                                               i->second.tenor_, strikes[j],
                                               Null<Date>(), 0.0, true));
                    modelPut.push_back(
                        i->second.isCaplet_
                            ? capletPriceInternal(Option::Put, i->first, strikes[j],
                                             Null<Date>(), 0.0, true)
                            : swaptionPriceInternal(Option::Put, i->first,
                                               i->second.tenor_, strikes[j],
                                               Null<Date>(), 0.0, true));
                    marketVega.push_back(
                        sec->vega(strikes[j], i->second.annuity_));
                }
                modelOutputs_.smileStrikes_.push_back(strikes);
                modelOutputs_.marketCallPremium_.push_back(marketCall);
                modelOutputs_.marketPutPremium_.push_back(marketPut);
                modelOutputs_.modelCallPremium_.push_back(modelCall);
                modelOutputs_.modelPutPremium_.push_back(modelPut);
                modelOutputs_.marketVega_.push_back(marketVega);
                modelOutputs_.marketRawCallPremium_.push_back(marketRawCall);
                modelOutputs_.marketRawPutPremium_.push_back(marketRawPut);
            }

            modelOutputs_.dirty_ = false;
        }

        return modelOutputs_;
    }

    const Disposable<Array>
    MarkovFunctional::numeraireArray(const Time t, const Array &y) const {

        calculate();
        Array res(y.size(), termStructure()->discount(numeraireTime_, true));
        if (t < QL_EPSILON)
            return res;

        Real inverseNormalization =
            termStructure()->discount(numeraireTime_, true) /
            termStructure()->discount(t, true);

        Time tz = std::min(t, times_.back());
        Size i = std::min<Size>(
            std::upper_bound(times_.begin(), times_.end() - 1, t) -
                times_.begin(),
            times_.size() - 1);

        Real ta = times_[i - 1];
        Real tb = times_[i];
        Real dt = tb - ta;

        for (Size j = 0; j < y.size(); j++) {
            Real yv = y[j];
            if (yv < y_.front())
                yv = y_.front();
            // FIXME flat extrapolation should be incoperated into interpolation
            // object, see above
            if (yv > y_.back())
                yv = y_.back();
            Real na = (*numeraire_[i - 1])(yv);
            Real nb = (*numeraire_[i])(yv);
            res[j] =
                inverseNormalization / ((tz - ta) / nb + (tb - tz) / na) * dt;
            // linear in reciprocal of normalized numeraire
        }

        return res;
    }

    const Disposable<Array>
    MarkovFunctional::zerobondArray(const Time T, const Time t,
                                    const Array &y) const {

        return deflatedZerobondArray(T, t, y) * numeraireArray(t, y);
    }

    const Disposable<Array>
    MarkovFunctional::deflatedZerobondArray(const Time T, const Time t,
                                            const Array &y) const {

        calculate();

        Array result(y.size(), 0.0);

        // Gauss Hermite

        Real stdDev_0_t = stateProcess_->stdDeviation(0.0, 0.0, t);
        // we use that the standard deviation is independent of $x$ here
        Real stdDev_0_T = stateProcess_->stdDeviation(0.0, 0.0, T);
        Real stdDev_t_T = stateProcess_->stdDeviation(t, 0.0, T - t);

        for (Size j = 0; j < y.size(); j++) {
            Array ya(modelSettings_.gaussHermitePoints_);
            for (Size i = 0; i < modelSettings_.gaussHermitePoints_; i++) {
                ya[i] = (y[j] * stdDev_0_t + stdDev_t_T * normalIntegralX_[i]) /
                        stdDev_0_T;
            }
            Array res = numeraireArray(T, ya);
            for (Size i = 0; i < modelSettings_.gaussHermitePoints_; i++) {
                result[j] += normalIntegralW_[i] / res[i];
            }
        }

        return result;
    }

    const Real MarkovFunctional::numeraireImpl(
        const Time t, const Real y,
        const Handle<YieldTermStructure> &yts) const {

        if (t == 0)
            return yts.empty()
                       ? this->termStructure()->discount(numeraireTime(), true)
                       : yts->discount(numeraireTime());

        Array ya(1, y);
        return numeraireArray(t, ya)[0] *
               (yts.empty() ? 1.0
                            : (yts->discount(numeraireTime()) /
                               yts->discount(t) * termStructure()->discount(t) /
                               termStructure()->discount(numeraireTime())));
    }

    const Real MarkovFunctional::zerobondImpl(
        const Time T, const Time t, const Real y,
        const Handle<YieldTermStructure> &yts) const {

        if (t == 0.0)
            return yts.empty() ? this->termStructure()->discount(T, true)
                               : yts->discount(T, true);
        Array ya(1, y);
        return zerobondArray(T, t, ya)[0] *
               (yts.empty() ? 1.0 : (yts->discount(T) / yts->discount(t) *
                                     termStructure()->discount(t) /
                                     termStructure()->discount(T)));
    }

    const Real MarkovFunctional::deflatedZerobond(Time T, Time t,
                                                  Real y) const {

        Array ya(1, y);
        return deflatedZerobondArray(T, t, ya)[0];
    }

    const Real MarkovFunctional::marketSwapRate(const Date &expiry,
                                                const CalibrationPoint &p,
                                                const Real digitalPrice,
                                                const Real guess) const {

        ZeroHelper z(this, expiry, p, digitalPrice);
        Brent b;
        Real solution = b.solve(
            z, modelSettings_.marketRateAccuracy_,
            std::max(std::min(guess, modelSettings_.upperRateBound_ - 0.00001),
                     modelSettings_.lowerRateBound_ + 0.00001),
            modelSettings_.lowerRateBound_, modelSettings_.upperRateBound_);
        return solution;
    }

    const Real MarkovFunctional::marketDigitalPrice(const Date &expiry,
                                                    const CalibrationPoint &p,
                                                    const Option::Type &type,
                                                    const Real strike) const {

        return p.smileSection_->digitalOptionPrice(strike, type, p.annuity_,
                                                   modelSettings_.digitalGap_);
    }

    std::ostream &operator<<(std::ostream &out,
                             const MarkovFunctional::ModelOutputs &m) {
        out << "Markov functional model trace output " << std::endl;
        out << "Model settings" << std::endl;
        out << "Grid points y        : " << m.settings_.yGridPoints_
            << std::endl;
        out << "Std devs y           : " << m.settings_.yStdDevs_ << std::endl;
        out << "Lower rate bound     : " << m.settings_.lowerRateBound_
            << std::endl;
        out << "Upper rate bound     : " << m.settings_.upperRateBound_
            << std::endl;
        out << "Gauss Hermite points : " << m.settings_.gaussHermitePoints_
            << std::endl;
        out << "Digital gap          : " << m.settings_.digitalGap_
            << std::endl;
        out << "Adjustments          : "
            << (m.settings_.adjustments_ &
                        MarkovFunctional::ModelSettings::AdjustDigitals
                    ? "Digitals "
                    : "") << (m.settings_.adjustments_ &
                                      MarkovFunctional::ModelSettings::AdjustYts
                                  ? "Yts "
                                  : "")
            << (m.settings_.adjustments_ &
                        MarkovFunctional::ModelSettings::ExtrapolatePayoffFlat
                    ? "FlatPayoffExt "
                    : "")
            << (m.settings_.adjustments_ &
                        MarkovFunctional::ModelSettings::NoPayoffExtrapolation
                    ? "NoPayoffExt "
                    : "")
            << (m.settings_.adjustments_ &
                        MarkovFunctional::ModelSettings::KahaleSmile
                    ? "Kahale "
                    : "")
            << (m.settings_.adjustments_ & MarkovFunctional::ModelSettings::
                                               SmileExponentialExtrapolation
                    ? "SmileExp "
                    : "")
            << (m.settings_.adjustments_ &
                        MarkovFunctional::ModelSettings::KahaleInterpolation
                    ? "KahaleInt "
                    : "")
            << (m.settings_.adjustments_ & MarkovFunctional::ModelSettings::
                                               SmileDeleteArbitragePoints
                    ? "SmileDelArb "
                    : "") << (m.settings_.adjustments_ &
                                      MarkovFunctional::ModelSettings::SabrSmile
                                  ? "Sabr"
                                  : "") << std::endl;
        out << "Smile moneyness checkpoints: ";
        for (Size i = 0; i < m.settings_.smileMoneynessCheckpoints_.size(); i++)
            out << m.settings_.smileMoneynessCheckpoints_[i]
                << (i < m.settings_.smileMoneynessCheckpoints_.size() - 1 ? ";"
                                                                          : "");
        out << std::endl;

        QL_REQUIRE(!m.dirty_, "model outputs are dirty");

        if (m.expiries_.size() == 0)
            return out; // no trace information was collected so no output
        out << std::endl;
        out << "Messages:" << std::endl;
        for (std::vector<std::string>::const_iterator i = m.messages_.begin();
             i != m.messages_.end(); i++)
            out << (*i) << std::endl;
        out << std::endl << std::setprecision(16);
        out << "Yield termstructure fit:" << std::endl;
        out << "expiry;tenor;atm;annuity;digitalAdj;ytsAdj;marketzerorate;"
               "modelzerorate;diff(bp)" << std::endl;
        for (Size i = 0; i < m.expiries_.size(); i++) {
            out << m.expiries_[i] << ";" << m.tenors_[i] << ";" << m.atm_[i]
                << ";" << m.annuity_[i] << ";"
                << m.digitalsAdjustmentFactors_[i] << ";"
                << m.adjustmentFactors_[i] << ";" << m.marketZerorate_[i] << ";"
                << m.modelZerorate_[i] << ";"
                << (m.marketZerorate_[i] - m.modelZerorate_[i]) * 10000.0
                << std::endl;
        }
        out << std::endl;
        out << "Volatility smile fit:" << std::endl;
        for (Size i = 0; i < m.expiries_.size(); i++) {
            std::ostringstream os;
            os << m.expiries_[i] << "/" << m.tenors_[i];
            std::string p = os.str();
            out << "strike(" << p << ");marketCallRaw(" << p << ";marketCall("
                << p << ");modelCall(" << p << ");marketPutRaw(" << p
                << ");marketPut(" << p << ");modelPut(" << p << ");marketVega("
                << p << ")" << (i < m.expiries_.size() - 1 ? ";" : "");
        }
        out << std::endl;
        for (Size j = 0; j < m.smileStrikes_[0].size(); j++) {
            for (Size i = 0; i < m.expiries_.size(); i++) {
                out << m.smileStrikes_[i][j] << ";"
                    << m.marketRawCallPremium_[i][j] << ";"
                    << m.marketCallPremium_[i][j] << ";"
                    << m.modelCallPremium_[i][j] << ";"
                    << m.marketRawPutPremium_[i][j] << ";"
                    << m.marketPutPremium_[i][j] << ";"
                    << m.modelPutPremium_[i][j] << ";" << m.marketVega_[i][j]
                    << (i < m.expiries_.size() - 1 ? ";" : "");
            }
            out << std::endl;
        }
        return out;
    }

    const Real MarkovFunctional::forwardRateInternal(
        const Date &fixing, const Date &referenceDate, const Real y,
        const bool zeroFixingDays, boost::shared_ptr<IborIndex> iborIdx) const {

        calculate();

        if (!iborIdx)
            iborIdx = iborIndex_;

        Date valueDate = zeroFixingDays ? fixing : iborIdx->valueDate(fixing);
        Date endDate = iborIdx->fixingCalendar().advance(
            iborIdx->valueDate(fixing), iborIdx->tenor(),
            iborIdx->businessDayConvention(),
            iborIdx->endOfMonth()); // FIXME Here we should use the calculation
                                    // date calendar ?
        Real dcf = iborIdx->dayCounter().yearFraction(valueDate, endDate);

        return (zerobond(valueDate, referenceDate, y) -
                zerobond(endDate, referenceDate, y)) /
               (dcf * zerobond(endDate, referenceDate, y));
    }

    const Real
    MarkovFunctional::swapRateInternal(const Date &fixing, const Period &tenor,
                                  const Date &referenceDate, const Real y,
                                  bool zeroFixingDays,
                                  boost::shared_ptr<SwapIndex> swapIdx) const {

        calculate();

        if (!swapIdx)
            swapIdx = swapIndexBase_;
        QL_REQUIRE(swapIdx, "No swap index given");

        SwapIndex tmpIdx =
            SwapIndex(swapIdx->familyName(), tenor, swapIdx->fixingDays(),
                      swapIdx->currency(), swapIdx->fixingCalendar(),
                      swapIdx->fixedLegTenor(), swapIdx->fixedLegConvention(),
                      swapIdx->dayCounter(), swapIdx->iborIndex());
        boost::shared_ptr<VanillaSwap> underlying =
            tmpIdx.underlyingSwap(fixing);
        Schedule sched = underlying->fixedSchedule();
        Real annuity = swapAnnuityInternal(fixing, tenor, referenceDate, y,
                                      zeroFixingDays, swapIdx);
        Rate atm =
            (zerobond(zeroFixingDays ? fixing : sched.dates().front(),
                      referenceDate, y) -
             zerobond(sched.calendar().adjust(sched.dates().back(),
                                              underlying->paymentConvention()),
                      referenceDate, y)) /
            annuity;
        return atm;
    }

    const Real MarkovFunctional::swapAnnuityInternal(
        const Date &fixing, const Period &tenor, const Date &referenceDate,
        const Real y, const bool zeroFixingDays,
        boost::shared_ptr<SwapIndex> swapIdx) const {

        calculate();

        if (!swapIdx)
            swapIdx = swapIndexBase_;
        QL_REQUIRE(swapIdx, "No swap index given");

        SwapIndex tmpIdx =
            SwapIndex(swapIdx->familyName(), tenor, swapIdx->fixingDays(),
                      swapIdx->currency(), swapIdx->fixingCalendar(),
                      swapIdx->fixedLegTenor(), swapIdx->fixedLegConvention(),
                      swapIdx->dayCounter(), swapIdx->iborIndex());
        boost::shared_ptr<VanillaSwap> underlying =
            tmpIdx.underlyingSwap(fixing);
        Schedule sched = underlying->fixedSchedule();

        Real annuity = 0.0;
        for (unsigned int j = 1; j < sched.size(); j++) {
            annuity +=
                zerobond(sched.calendar().adjust(
                             sched.date(j), underlying->paymentConvention()),
                         referenceDate, y) *
                swapIdx->dayCounter().yearFraction(
                    j == 1 && zeroFixingDays ? fixing : sched.date(j - 1),
                    sched.date(j));
        }
        return annuity;
    }

    const Real MarkovFunctional::swaptionPriceInternal(
        const Option::Type &type, const Date &expiry, const Period &tenor,
        const Rate strike, const Date &referenceDate, const Real y,
        const bool zeroFixingDays, boost::shared_ptr<SwapIndex> swapIdx) const {

        calculate();

        Time fixingTime = termStructure()->timeFromReference(expiry);
        Time referenceTime =
            referenceDate == Null<Date>()
                ? 0.0
                : termStructure()->timeFromReference(referenceDate);

        Array yg = yGrid(modelSettings_.yStdDevs_, modelSettings_.yGridPoints_,
                         fixingTime, referenceTime, y);
        Array z = yGrid(modelSettings_.yStdDevs_, modelSettings_.yGridPoints_);
        Array p(yg.size());

        for (Size i = 0; i < yg.size(); i++) {
            Real annuity = swapAnnuityInternal(expiry, tenor, expiry, yg[i],
                                          zeroFixingDays, swapIdx);
            Rate atm = swapRateInternal(expiry, tenor, expiry, yg[i], zeroFixingDays,
                                   swapIdx);
            p[i] = annuity * std::max((type == Option::Call ? 1.0 : -1.0) *
                                          (atm - strike),
                                      0.0) /
                   numeraire(fixingTime, yg[i]);
        }

        CubicInterpolation payoff(z.begin(), z.end(), p.begin(),
                                  CubicInterpolation::Spline, true,
                                  CubicInterpolation::Lagrange, 0.0,
                                  CubicInterpolation::Lagrange, 0.0);

        Real price = 0.0;
        for (Size i = 0; i < z.size() - 1; i++) {
            price += gaussianShiftedPolynomialIntegral(
                0.0, payoff.cCoefficients()[i], payoff.bCoefficients()[i],
                payoff.aCoefficients()[i], p[i], z[i], z[i], z[i + 1]);
        }
        if ((modelSettings_.adjustments_ &
             ModelSettings::NoPayoffExtrapolation) == 0) {
            if ((modelSettings_.adjustments_ &
                 ModelSettings::ExtrapolatePayoffFlat) != 0) {
                price += gaussianShiftedPolynomialIntegral(
                    0.0, 0.0, 0.0, 0.0, p[z.size() - 2], z[z.size() - 2],
                    z[z.size() - 1], 100.0);
                price += gaussianShiftedPolynomialIntegral(
                    0.0, 0.0, 0.0, 0.0, p[0], z[0], -100.0, z[0]);
            } else {
                if (type == Option::Call)
                    price += gaussianShiftedPolynomialIntegral(
                        0.0, payoff.cCoefficients()[z.size() - 2],
                        payoff.bCoefficients()[z.size() - 2],
                        payoff.aCoefficients()[z.size() - 2], p[z.size() - 2],
                        z[z.size() - 2], z[z.size() - 1], 100.0);
                if (type == Option::Put)
                    price += gaussianShiftedPolynomialIntegral(
                        0.0, payoff.cCoefficients()[0],
                        payoff.bCoefficients()[0], payoff.aCoefficients()[0],
                        p[0], z[0], -100.0, z[0]);
            }
        }

        return numeraire(referenceTime, y) * price;
    }

    const Real MarkovFunctional::capletPriceInternal(
        const Option::Type &type, const Date &expiry, const Rate strike,
        const Date &referenceDate, const Real y, const bool zeroFixingDays,
        boost::shared_ptr<IborIndex> iborIdx) const {

        calculate();

        if (!iborIdx)
            iborIdx = iborIndex_;

        Time fixingTime = termStructure()->timeFromReference(expiry);
        Time referenceTime =
            referenceDate == Null<Date>()
                ? 0.0
                : termStructure()->timeFromReference(referenceDate);

        Array yg = yGrid(modelSettings_.yStdDevs_, modelSettings_.yGridPoints_,
                         fixingTime, referenceTime, y);
        Array z = yGrid(modelSettings_.yStdDevs_, modelSettings_.yGridPoints_);
        Array p(yg.size());

        Date valueDate = iborIdx->valueDate(expiry);
        Date endDate = iborIdx->fixingCalendar().advance(
            valueDate, iborIdx->tenor(), iborIdx->businessDayConvention(),
            iborIdx->endOfMonth()); // FIXME Here we should use the calculation
                                    // date calendar ?
        Real dcf = iborIdx->dayCounter().yearFraction(
            zeroFixingDays ? expiry : valueDate, endDate);

        for (Size i = 0; i < yg.size(); i++) {
            Real annuity = zerobond(endDate, expiry, yg[i]) * dcf;
            Rate atm =
                forwardRateInternal(expiry, expiry, yg[i], zeroFixingDays, iborIdx);
            p[i] = annuity * std::max((type == Option::Call ? 1.0 : -1.0) *
                                          (atm - strike),
                                      0.0) /
                   numeraire(fixingTime, yg[i]);
        }

        CubicInterpolation payoff(z.begin(), z.end(), p.begin(),
                                  CubicInterpolation::Spline, true,
                                  CubicInterpolation::Lagrange, 0.0,
                                  CubicInterpolation::Lagrange, 0.0);

        Real price = 0.0;
        for (Size i = 0; i < z.size() - 1; i++) {
            price += gaussianShiftedPolynomialIntegral(
                0.0, payoff.cCoefficients()[i], payoff.bCoefficients()[i],
                payoff.aCoefficients()[i], p[i], z[i], z[i], z[i + 1]);
        }
        if ((modelSettings_.adjustments_ &
             ModelSettings::NoPayoffExtrapolation) == 0) {
            if ((modelSettings_.adjustments_ &
                 ModelSettings::ExtrapolatePayoffFlat) != 0) {
                price += gaussianShiftedPolynomialIntegral(
                    0.0, 0.0, 0.0, 0.0, p[z.size() - 2], z[z.size() - 2],
                    z[z.size() - 1], 100.0);
                price += gaussianShiftedPolynomialIntegral(
                    0.0, 0.0, 0.0, 0.0, p[0], z[0], -100.0, z[0]);
            } else {
                if (type == Option::Call)
                    price += gaussianShiftedPolynomialIntegral(
                        0.0, payoff.cCoefficients()[z.size() - 2],
                        payoff.bCoefficients()[z.size() - 2],
                        payoff.aCoefficients()[z.size() - 2], p[z.size() - 2],
                        z[z.size() - 2], z[z.size() - 1], 100.0);
                if (type == Option::Put)
                    price += gaussianShiftedPolynomialIntegral(
                        0.0, payoff.cCoefficients()[0],
                        payoff.bCoefficients()[0], payoff.aCoefficients()[0],
                        p[0], z[0], -100.0, z[0]);
            }
        }

        return numeraire(referenceTime, y) * price;
    }
}
