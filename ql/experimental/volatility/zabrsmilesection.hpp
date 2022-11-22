/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

/*! \file zabrsmilesection.hpp
    \brief zabr smile section
*/

#ifndef quantlib_zabr_smile_section_hpp
#define quantlib_zabr_smile_section_hpp

#include <ql/experimental/volatility/zabr.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/termstructures/volatility/smilesectionutils.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <utility>
#include <vector>

using std::exp;

namespace QuantLib {

// Evaluation Tags

struct ZabrShortMaturityLognormal {};
struct ZabrShortMaturityNormal {};
struct ZabrLocalVolatility {};
struct ZabrFullFd {};

template <typename Evaluation> class ZabrSmileSection : public SmileSection {
  public:
    ZabrSmileSection(Time timeToExpiry,
                     Rate forward,
                     std::vector<Real> zabrParameters,
                     const std::vector<Real>& moneyness = std::vector<Real>(),
                     Size fdRefinement = 5);
    ZabrSmileSection(const Date& d,
                     Rate forward,
                     std::vector<Real> zabrParameters,
                     const DayCounter& dc = Actual365Fixed(),
                     const std::vector<Real>& moneyness = std::vector<Real>(),
                     Size fdRefinement = 5);

    Real minStrike() const override { return 0.0; }
    Real maxStrike() const override { return QL_MAX_REAL; }
    Real atmLevel() const override { return model_->forward(); }
    Real
    optionPrice(Rate strike, Option::Type type = Option::Call, Real discount = 1.0) const override {
        return optionPrice(strike, type, discount, Evaluation());
    }

    ext::shared_ptr<ZabrModel> model() { return model_; }

  protected:
    Volatility volatilityImpl(Rate strike) const override {
        return volatilityImpl(strike, Evaluation());
    }

  private:
    void init(const std::vector<Real> &moneyness) {
        init(moneyness, Evaluation());
        init2(Evaluation());
        init3(Evaluation());
    }
    void init(const std::vector<Real> &moneyness, ZabrShortMaturityLognormal);
    void init(const std::vector<Real> &moneyness, ZabrShortMaturityNormal);
    void init(const std::vector<Real> &moneyness, ZabrLocalVolatility);
    void init(const std::vector<Real> &moneyness, ZabrFullFd);
    void init2(ZabrShortMaturityLognormal);
    void init2(ZabrShortMaturityNormal);
    void init2(ZabrLocalVolatility);
    void init2(ZabrFullFd);
    void init3(ZabrShortMaturityLognormal);
    void init3(ZabrShortMaturityNormal);
    void init3(ZabrLocalVolatility);
    void init3(ZabrFullFd);
    Real optionPrice(Rate strike, Option::Type type, Real discount,
                     ZabrShortMaturityLognormal) const;
    Real optionPrice(Rate strike, Option::Type type, Real discount,
                     ZabrShortMaturityNormal) const;
    Real optionPrice(Rate strike, Option::Type type, Real discount,
                     ZabrLocalVolatility) const;
    Real optionPrice(Rate strike, Option::Type type, Real discount,
                     ZabrFullFd) const;
    Volatility volatilityImpl(Rate strike, ZabrShortMaturityLognormal) const;
    Volatility volatilityImpl(Rate strike, ZabrShortMaturityNormal) const;
    Volatility volatilityImpl(Rate strike, ZabrLocalVolatility) const;
    Volatility volatilityImpl(Rate strike, ZabrFullFd) const;
    ext::shared_ptr<ZabrModel> model_;
    Evaluation evaluation_;
    Rate forward_;
    std::vector<Real> params_;
    const Size fdRefinement_;
    std::vector<Real> strikes_, callPrices_;
    ext::shared_ptr<Interpolation> callPriceFct_;
    Real a_, b_;
};

template <typename Evaluation>
ZabrSmileSection<Evaluation>::ZabrSmileSection(Time timeToExpiry,
                                               Rate forward,
                                               std::vector<Real> zabrParams,
                                               const std::vector<Real>& moneyness,
                                               const Size fdRefinement)
: SmileSection(timeToExpiry, DayCounter()), forward_(forward), params_(std::move(zabrParams)),
  fdRefinement_(fdRefinement) {
    init(moneyness);
}

template <typename Evaluation>
ZabrSmileSection<Evaluation>::ZabrSmileSection(const Date& d,
                                               Rate forward,
                                               std::vector<Real> zabrParams,
                                               const DayCounter& dc,
                                               const std::vector<Real>& moneyness,
                                               const Size fdRefinement)
: SmileSection(d, dc, Date()), forward_(forward), params_(std::move(zabrParams)),
  fdRefinement_(fdRefinement) {
    init(moneyness);
}

template <typename Evaluation>
void ZabrSmileSection<Evaluation>::init(const std::vector<Real> &,
                                        ZabrShortMaturityLognormal) {

    model_ = ext::make_shared<ZabrModel>(
        exerciseTime(), forward_, params_[0], params_[1],
                      params_[2], params_[3], params_[4]);
}

template <typename Evaluation>
void ZabrSmileSection<Evaluation>::init(const std::vector<Real> &a,
                                        ZabrShortMaturityNormal) {
    init(a, ZabrShortMaturityLognormal());
}

template <typename Evaluation>
void ZabrSmileSection<Evaluation>::init(const std::vector<Real> &moneyness,
                                        ZabrLocalVolatility) {

    QL_REQUIRE(params_.size() >= 5,
               "zabr expects 5 parameters (alpha,beta,nu,rho,gamma) but ("
                   << params_.size() << ") given");

    model_ = ext::make_shared<ZabrModel>(
        exerciseTime(), forward_, params_[0], params_[1],
                      params_[2], params_[3], params_[4]);

    // set up strike grid for local vol or full fd flavour of this section
    // this is shared with SmileSectionUtils - unify later ?
    static const Real defaultMoney[] = {
        0.0, 0.01, 0.05, 0.10, 0.25, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90,
        1.0, 1.25, 1.5,  1.75, 2.0,  5.0,  7.5,  10.0, 15.0, 20.0};
    std::vector<Real> tmp;
    if (moneyness.empty())
        tmp = std::vector<Real>(defaultMoney, defaultMoney + 21);
    else
        tmp = std::vector<Real>(moneyness);

    strikes_.clear(); // should not be necessary, anyway
    Real lastF = 0.0;
    bool firstStrike = true;
    for (Real i : tmp) {
        Real f = i * forward_;
        if (f > 0.0) {
            if (!firstStrike) {
                for (Size j = 1; j <= fdRefinement_; ++j) {
                    strikes_.push_back(lastF +
                                       ((double)j) * (f - lastF) /
                                           (fdRefinement_ + 1));
                }
            }
            firstStrike = false;
            lastF = f;
            strikes_.push_back(f);
        }
    }
}

template <typename Evaluation>
void ZabrSmileSection<Evaluation>::init(const std::vector<Real> &moneyness,
                                        ZabrFullFd) {
    init(moneyness, ZabrLocalVolatility());
}

template <typename Evaluation>
void ZabrSmileSection<Evaluation>::init2(ZabrShortMaturityLognormal) {}

template <typename Evaluation>
void ZabrSmileSection<Evaluation>::init2(ZabrShortMaturityNormal) {}

template <typename Evaluation>
void ZabrSmileSection<Evaluation>::init2(ZabrLocalVolatility) {
    callPrices_ = model_->fdPrice(strikes_);
}

template <typename Evaluation>
void ZabrSmileSection<Evaluation>::init2(ZabrFullFd) {
    callPrices_.resize(strikes_.size());
#pragma omp parallel for
    for (long i = 0; i < (long)strikes_.size(); i++) {
        callPrices_[i] = model_->fullFdPrice(strikes_[i]);
    }
}

template <typename Evaluation>
void ZabrSmileSection<Evaluation>::init3(ZabrShortMaturityLognormal) {}

template <typename Evaluation>
void ZabrSmileSection<Evaluation>::init3(ZabrShortMaturityNormal) {}

template <typename Evaluation>
void ZabrSmileSection<Evaluation>::init3(ZabrLocalVolatility) {
    strikes_.insert(strikes_.begin(), 0.0);
    callPrices_.insert(callPrices_.begin(), forward_);

    callPriceFct_ = ext::shared_ptr<Interpolation>(new CubicInterpolation(
        strikes_.begin(), strikes_.end(), callPrices_.begin(),
        CubicInterpolation::Spline, true, CubicInterpolation::SecondDerivative,
        0.0, CubicInterpolation::SecondDerivative, 0.0));
    // callPriceFct_ =
    //     ext::shared_ptr<Interpolation>(new LinearInterpolation(
    //         strikes_.begin(), strikes_.end(), callPrices_.begin()));

    callPriceFct_->enableExtrapolation();

    // on the right side we extrapolate exponetially (because spline
    // does not make sense)
    // we precompute the necessary parameters here
    static const Real eps = 1E-5; // gap for first derivative computation

    Real c0 = (*callPriceFct_)(strikes_.back());
    Real c0p = ((*callPriceFct_)(strikes_.back() - eps) - c0) / eps;

    a_ = c0p / c0;
    b_ = std::log(c0) + a_ * strikes_.back();
}

template <typename Evaluation>
void ZabrSmileSection<Evaluation>::init3(ZabrFullFd) {
    init3(ZabrLocalVolatility());
}

template <typename Evaluation>
Real
ZabrSmileSection<Evaluation>::optionPrice(Real strike, Option::Type type,
                                          Real discount,
                                          ZabrShortMaturityLognormal) const {
    return SmileSection::optionPrice(strike, type, discount);
}

template <typename Evaluation>
Real ZabrSmileSection<Evaluation>::optionPrice(Real strike, Option::Type type,
                                               Real discount,
                                               ZabrShortMaturityNormal) const {
    return bachelierBlackFormula(
        type, strike, forward_,
        model_->normalVolatility(strike) * std::sqrt(exerciseTime()), discount);
}

template <typename Evaluation>
Real ZabrSmileSection<Evaluation>::optionPrice(Rate strike, Option::Type type,
                                               Real discount,
                                               ZabrLocalVolatility) const {
    Real call = strike <= strikes_.back() ? (*callPriceFct_)(strike)
                                          : exp(-a_ * strike + b_);
    if (type == Option::Call)
        return call * discount;
    else
        return (call - (forward_ - strike)) * discount;
}

template <typename Evaluation>
Real ZabrSmileSection<Evaluation>::optionPrice(Rate strike, Option::Type type,
                                               Real discount,
                                               ZabrFullFd) const {
    return optionPrice(strike, type, discount, ZabrLocalVolatility());
}

template <typename Evaluation>
Real
ZabrSmileSection<Evaluation>::volatilityImpl(Rate strike,
                                             ZabrShortMaturityLognormal) const {
    strike = std::max(1E-6, strike);
    return model_->lognormalVolatility(strike);
}

template <typename Evaluation>
Real
ZabrSmileSection<Evaluation>::volatilityImpl(Rate strike,
                                             ZabrShortMaturityNormal) const {
    Real impliedVol = 0.0;
    try {
        Option::Type type;
        if (strike >= model_->forward())
            type = Option::Call;
        else
            type = Option::Put;
        impliedVol =
            blackFormulaImpliedStdDev(type, strike, model_->forward(),
                                      optionPrice(strike, type, 1.0), 1.0) /
            std::sqrt(exerciseTime());
    } catch (...) {
    }
    return impliedVol;
}

template <typename Evaluation>
Real ZabrSmileSection<Evaluation>::volatilityImpl(Rate strike,
                                                  ZabrLocalVolatility) const {
    return volatilityImpl(strike, ZabrShortMaturityNormal());
}

template <typename Evaluation>
Real ZabrSmileSection<Evaluation>::volatilityImpl(Rate strike,
                                                  ZabrFullFd) const {
    return volatilityImpl(strike, ZabrShortMaturityNormal());
}
}

#endif
