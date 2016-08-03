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

/*! \file zabrinterpolatedsmilesection.hpp
    \brief zabr interpolating smile section
*/

#ifndef quantlib_zabr_interpolated_smile_section_hpp
#define quantlib_zabr_interpolated_smile_section_hpp

#include <ql/handle.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/experimental/volatility/zabrinterpolation.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

template <typename Evaluation>
class ZabrInterpolatedSmileSection : public SmileSection, public LazyObject {
  public:
    //! \name Constructors
    //@{
    //! all market data are quotes
    ZabrInterpolatedSmileSection(
        const Date &optionDate, const Handle<Quote> &forward,
        const std::vector<Rate> &strikes, bool hasFloatingStrikes,
        const Handle<Quote> &atmVolatility,
        const std::vector<Handle<Quote> > &volHandles, Real alpha, Real beta,
        Real nu, Real rho, Real gamma, bool isAlphaFixed = false,
        bool isBetaFixed = false, bool isNuFixed = false,
        bool isRhoFixed = false, bool isGammaFixed = false,
        bool vegaWeighted = true,
        const boost::shared_ptr<EndCriteria> &endCriteria =
            boost::shared_ptr<EndCriteria>(),
        const boost::shared_ptr<OptimizationMethod> &method =
            boost::shared_ptr<OptimizationMethod>(),
        const DayCounter &dc = Actual365Fixed());
    //! no quotes
    ZabrInterpolatedSmileSection(
        const Date &optionDate, const Rate &forward,
        const std::vector<Rate> &strikes, bool hasFloatingStrikes,
        const Volatility &atmVolatility, const std::vector<Volatility> &vols,
        Real alpha, Real beta, Real nu, Real rho, Real gamma,
        bool isAlphaFixed = false, bool isBetaFixed = false,
        bool isNuFixed = false, bool isRhoFixed = false,
        bool isGammaFixed = false, bool vegaWeighted = true,
        const boost::shared_ptr<EndCriteria> &endCriteria =
            boost::shared_ptr<EndCriteria>(),
        const boost::shared_ptr<OptimizationMethod> &method =
            boost::shared_ptr<OptimizationMethod>(),
        const DayCounter &dc = Actual365Fixed());
    //@}
    //! \name LazyObject interface
    //@{
    virtual void performCalculations() const;
    virtual void update();
    //@}
    //! \name SmileSection interface
    //@{
    Real minStrike() const;
    Real maxStrike() const;
    Real atmLevel() const;
    //@}
    Real varianceImpl(Rate strike) const;
    Volatility volatilityImpl(Rate strike) const;
    //! \name Inspectors
    //@{
    Real alpha() const;
    Real beta() const;
    Real nu() const;
    Real rho() const;
    Real gamma() const;
    Real rmsError() const;
    Real maxError() const;
    EndCriteria::Type endCriteria() const;
    //@}

  protected:
    //! Creates the mutable SABRInterpolation
    void createInterpolation() const;
    mutable boost::shared_ptr<ZabrInterpolation<Evaluation> > zabrInterpolation_;

    //! Market data
    const Handle<Quote> forward_;
    const Handle<Quote> atmVolatility_;
    std::vector<Handle<Quote> > volHandles_;
    mutable std::vector<Rate> strikes_;
    //! Only strikes corresponding to valid market data
    mutable std::vector<Rate> actualStrikes_;
    bool hasFloatingStrikes_;

    mutable Real forwardValue_;
    mutable std::vector<Volatility> vols_;
    //! Sabr parameters
    Real alpha_, beta_, nu_, rho_, gamma_;
    //! Sabr interpolation settings
    bool isAlphaFixed_, isBetaFixed_, isNuFixed_, isRhoFixed_, isGammaFixed_;
    bool vegaWeighted_;
    const boost::shared_ptr<EndCriteria> endCriteria_;
    const boost::shared_ptr<OptimizationMethod> method_;
};

template <typename Evaluation>
inline void ZabrInterpolatedSmileSection<Evaluation>::update() {
    LazyObject::update();
    SmileSection::update();
}

template <typename Evaluation>
inline Real
ZabrInterpolatedSmileSection<Evaluation>::volatilityImpl(Rate strike) const {
    calculate();
    return (*zabrInterpolation_)(strike, true);
}

template <typename Evaluation>
inline Real ZabrInterpolatedSmileSection<Evaluation>::alpha() const {
    calculate();
    return zabrInterpolation_->alpha();
}

template <typename Evaluation>
inline Real ZabrInterpolatedSmileSection<Evaluation>::beta() const {
    calculate();
    return zabrInterpolation_->beta();
}

template <typename Evaluation>
inline Real ZabrInterpolatedSmileSection<Evaluation>::nu() const {
    calculate();
    return zabrInterpolation_->nu();
}

template <typename Evaluation>
inline Real ZabrInterpolatedSmileSection<Evaluation>::rho() const {
    calculate();
    return zabrInterpolation_->rho();
}

template <typename Evaluation>
inline Real ZabrInterpolatedSmileSection<Evaluation>::rmsError() const {
    calculate();
    return zabrInterpolation_->rmsError();
}

template <typename Evaluation>
inline Real ZabrInterpolatedSmileSection<Evaluation>::maxError() const {
    calculate();
    return zabrInterpolation_->maxError();
}

template <typename Evaluation>
inline EndCriteria::Type
ZabrInterpolatedSmileSection<Evaluation>::endCriteria() const {
    calculate();
    return zabrInterpolation_->endCriteria();
}

template <typename Evaluation>
inline Real ZabrInterpolatedSmileSection<Evaluation>::minStrike() const {
    calculate();
    return actualStrikes_.front();
}

template <typename Evaluation>
inline Real ZabrInterpolatedSmileSection<Evaluation>::maxStrike() const {
    calculate();
    return actualStrikes_.back();
}

template <typename Evaluation>
inline Real ZabrInterpolatedSmileSection<Evaluation>::atmLevel() const {
    calculate();
    return forwardValue_;
}

template <typename Evaluation>
ZabrInterpolatedSmileSection<Evaluation>::ZabrInterpolatedSmileSection(
    const Date &optionDate, const Handle<Quote> &forward,
    const std::vector<Rate> &strikes, bool hasFloatingStrikes,
    const Handle<Quote> &atmVolatility,
    const std::vector<Handle<Quote> > &volHandles, Real alpha, Real beta,
    Real nu, Real rho, Real gamma, bool isAlphaFixed, bool isBetaFixed,
    bool isNuFixed, bool isRhoFixed, bool isGammaFixed, bool vegaWeighted,
    const boost::shared_ptr<EndCriteria> &endCriteria,
    const boost::shared_ptr<OptimizationMethod> &method, const DayCounter &dc)
    : SmileSection(optionDate, dc), forward_(forward),
      atmVolatility_(atmVolatility), volHandles_(volHandles), strikes_(strikes),
      actualStrikes_(strikes), hasFloatingStrikes_(hasFloatingStrikes),
      vols_(volHandles.size()), alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
      gamma_(gamma), isAlphaFixed_(isAlphaFixed), isBetaFixed_(isBetaFixed),
      isNuFixed_(isNuFixed), isRhoFixed_(isRhoFixed),
      isGammaFixed_(isGammaFixed), vegaWeighted_(vegaWeighted),
      endCriteria_(endCriteria), method_(method) {

    LazyObject::registerWith(forward_);
    LazyObject::registerWith(atmVolatility_);
    for (Size i = 0; i < volHandles_.size(); ++i)
        LazyObject::registerWith(volHandles_[i]);
}

template <typename Evaluation>
ZabrInterpolatedSmileSection<Evaluation>::ZabrInterpolatedSmileSection(
    const Date &optionDate, const Rate &forward,
    const std::vector<Rate> &strikes, bool hasFloatingStrikes,
    const Volatility &atmVolatility, const std::vector<Volatility> &volHandles,
    Real alpha, Real beta, Real nu, Real rho, Real gamma, bool isAlphaFixed,
    bool isBetaFixed, bool isNuFixed, bool isRhoFixed, bool isGammaFixed,
    bool vegaWeighted, const boost::shared_ptr<EndCriteria> &endCriteria,
    const boost::shared_ptr<OptimizationMethod> &method, const DayCounter &dc)
    : SmileSection(optionDate, dc),
      forward_(
          Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(forward)))),
      atmVolatility_(Handle<Quote>(
          boost::shared_ptr<Quote>(new SimpleQuote(atmVolatility)))),
      volHandles_(volHandles.size()), strikes_(strikes),
      actualStrikes_(strikes), hasFloatingStrikes_(hasFloatingStrikes),
      vols_(volHandles.size()), alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
      gamma_(gamma), isAlphaFixed_(isAlphaFixed), isBetaFixed_(isBetaFixed),
      isNuFixed_(isNuFixed), isRhoFixed_(isRhoFixed),
      isGammaFixed_(isGammaFixed), vegaWeighted_(vegaWeighted),
      endCriteria_(endCriteria), method_(method) {

    for (Size i = 0; i < volHandles_.size(); ++i)
        volHandles_[i] = Handle<Quote>(
            boost::shared_ptr<Quote>(new SimpleQuote(volHandles[i])));
}

template <typename Evaluation>
void ZabrInterpolatedSmileSection<Evaluation>::createInterpolation() const {
    boost::shared_ptr<ZabrInterpolation<Evaluation> > tmp(
        new ZabrInterpolation<Evaluation>(
            actualStrikes_.begin(), actualStrikes_.end(), vols_.begin(),
            exerciseTime(), forwardValue_, alpha_, beta_, nu_, rho_, gamma_,
            isAlphaFixed_, isBetaFixed_, isNuFixed_, isRhoFixed_, isGammaFixed_,
            vegaWeighted_, endCriteria_, method_));
    swap(tmp, zabrInterpolation_);
}

template <typename Evaluation>
void ZabrInterpolatedSmileSection<Evaluation>::performCalculations() const {
    forwardValue_ = forward_->value();
    vols_.clear();
    actualStrikes_.clear();
    // we populate the volatilities, skipping the invalid ones
    for (Size i = 0; i < volHandles_.size(); ++i) {
        if (volHandles_[i]->isValid()) {
            if (hasFloatingStrikes_) {
                actualStrikes_.push_back(forwardValue_ + strikes_[i]);
                vols_.push_back(atmVolatility_->value() +
                                volHandles_[i]->value());
            } else {
                actualStrikes_.push_back(strikes_[i]);
                vols_.push_back(volHandles_[i]->value());
            }
        }
    }
    // we are recreating the sabrinterpolation object unconditionnaly to
    // avoid iterator invalidation
    createInterpolation();
    zabrInterpolation_->update();
}

template <typename Evaluation>
Real ZabrInterpolatedSmileSection<Evaluation>::varianceImpl(Real strike) const {
    calculate();
    Real v = (*zabrInterpolation_)(strike, true);
    return v * v * exerciseTime();
}
}

#endif
