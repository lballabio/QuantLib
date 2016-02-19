/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file mcgaussian1dnonstandardswaptionengine.hpp
    \brief Monte Carlo engine for bermudan (non standard) swaptions
           in a gaussian 1d model
*/

#ifndef quantlib_mc_gaussian1d_nonstandardswaption_engine_hpp
#define quantlib_mc_gaussian1d_nonstandardswaption_engine_hpp

#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/pricingengines/mclongstaffschwartzengine.hpp>
#include <ql/experimental/models/longstaffschwartzproxypathpricer.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/instruments/nonstandardswaption.hpp>

#include <boost/make_shared.hpp>

namespace QuantLib {

/*! All fixed coupons with start date greater or equal to the
    respective option expiry are considered to be part of the
    exercise into right.

    \warning Cash settled swaptions are not supported
*/

template <class RNG = PseudoRandom, class S = Statistics>
class McGaussian1dNonstandardSwaptionEngine
    : public MCLongstaffSchwartzEngine<
          GenericModelEngine<Gaussian1dModel, NonstandardSwaption::arguments,
                             NonstandardSwaption::results>,
          SingleVariate, RNG, S> {

  public:
    typedef MCLongstaffSchwartzEngine<
        GenericModelEngine<Gaussian1dModel, NonstandardSwaption::arguments,
                           NonstandardSwaption::results>,
        SingleVariate, RNG, S> base_class;

    /*! proxy function */
    struct LsFunction : NonstandardSwaption::Proxy::ProxyFunction {
        Real operator()(const Real state) const {
            Real tmp = 0.0;
            if (state > cutoff) {
                for (Size i = 0; i < coeffItm.size(); ++i)
                    tmp += coeffItm[i] * v[i](state);
            } else {
                for (Size i = 0; i < coeffOtm.size(); ++i)
                    tmp += coeffOtm[i] * v[i](state);
            }
            return std::max(tmp, 0.0); // continuation value is always positive
        }
        Array coeffItm, coeffOtm;
        Real cutoff;
        std::vector<boost::function1<Real, Real> > v;
    };

    McGaussian1dNonstandardSwaptionEngine(
        const boost::shared_ptr<Gaussian1dModel> &model,
        const Handle<Quote>
            &oas, // continuously compounded w.r.t. yts daycounter
        const Handle<YieldTermStructure> &discountCurve,
        Size timeSteps, Size timeStepsPerYear, bool brownianBridge,
        bool antitheticVariate, Size requiredSamples, Real requiredTolerance,
        Size maxSamples, BigNatural seed, Size nCalibrationSamples,
        bool generateProxy);

    void calculate() const;
    void reset() const;

  protected:
    boost::shared_ptr<LongstaffSchwartzPathPricer<Path> > lsmPathPricer() const;

  private:
    boost::shared_ptr<Gaussian1dModel> model_;
    Handle<Quote> oas_;
    Handle<YieldTermStructure> discountCurve_;
    bool generateProxy_;
    mutable boost::shared_ptr<NonstandardSwaption::Proxy> proxy_;
};

//! factory

template <class RNG = PseudoRandom, class S = Statistics>
class MakeMcGaussian1dNonstandardSwaptionEngine {
  public:
    MakeMcGaussian1dNonstandardSwaptionEngine(
        const boost::shared_ptr<Gaussian1dModel> &model);
    MakeMcGaussian1dNonstandardSwaptionEngine &withSteps(Size steps);
    MakeMcGaussian1dNonstandardSwaptionEngine &withStepsPerYear(Size steps);
    MakeMcGaussian1dNonstandardSwaptionEngine &
    withBrownianBridge(bool b = true);
    MakeMcGaussian1dNonstandardSwaptionEngine &
    withAntitheticVariate(bool b = true);
    MakeMcGaussian1dNonstandardSwaptionEngine &withSamples(Size samples);
    MakeMcGaussian1dNonstandardSwaptionEngine &
    withAbsoluteTolerance(Real tolerance);
    MakeMcGaussian1dNonstandardSwaptionEngine &withMaxSamples(Size samples);
    MakeMcGaussian1dNonstandardSwaptionEngine &withSeed(BigNatural seed);
    MakeMcGaussian1dNonstandardSwaptionEngine &
    withCalibrationSamples(Size calibrationSamples);
    MakeMcGaussian1dNonstandardSwaptionEngine &
    withOas(const Handle<Quote> &oas);
    MakeMcGaussian1dNonstandardSwaptionEngine &
    withDiscount(const Handle<YieldTermStructure> &discount);
    MakeMcGaussian1dNonstandardSwaptionEngine &withProxy(bool b = true);
    // conversion to pricing engine
    operator boost::shared_ptr<PricingEngine>() const;

  private:
    boost::shared_ptr<Gaussian1dModel> model_;
    Handle<Quote> oas_;
    Handle<YieldTermStructure> discount_;
    bool brownianBridge_, antithetic_;
    Size steps_, stepsPerYear_, samples_, calibrationSamples_, maxSamples_;
    Real tolerance_;
    BigNatural seed_;
    bool generateProxy_;
};

//! Path Pricer

class Gaussian1dNonstandardSwaptionPathPricer
    : public EarlyExercisePathPricer<Path> {
  public:
    Gaussian1dNonstandardSwaptionPathPricer(
        const boost::shared_ptr<Gaussian1dModel> &model,
        const NonstandardSwaption::arguments *arguments,
        const Handle<YieldTermStructure> &discount, const Handle<Quote> &oas);
    Real operator()(const Path &path, Size t) const;
    Real state(const Path &path, Size t) const;
    std::vector<boost::function1<Real, Real> > basisSystem() const;
    std::vector<boost::function1<Real, Real> > basisSystem2() const;

  private:
    void initExerciseIndices(const Path &path) const;
    const boost::shared_ptr<Gaussian1dModel> model_;
    const NonstandardSwaption::arguments *arguments_;
    const Handle<YieldTermStructure> discount_;
    const Handle<Quote> oas_;
    std::vector<boost::function1<Real, Real> > basis_, basis2_;
    mutable std::vector<Size> exerciseIdx_;
    Size minIdxAlive_;
};

// implementation

template <class RNG, class S>
McGaussian1dNonstandardSwaptionEngine<RNG, S>::
    McGaussian1dNonstandardSwaptionEngine(
        const boost::shared_ptr<Gaussian1dModel> &model,
        const Handle<Quote>
            &oas, // continuously compounded w.r.t. yts daycounter
        const Handle<YieldTermStructure> &discountCurve,
        Size timeSteps, Size timeStepsPerYear, bool brownianBridge,
        bool antitheticVariate, Size requiredSamples, Real requiredTolerance,
        Size maxSamples, BigNatural seed, Size nCalibrationSamples,
        bool generateProxy)
    : MCLongstaffSchwartzEngine<
          GenericModelEngine<Gaussian1dModel, NonstandardSwaption::arguments,
                             NonstandardSwaption::results>,
          SingleVariate, RNG, S>(
          model->stateProcess(), timeSteps, timeStepsPerYear, brownianBridge,
          antitheticVariate, false, requiredSamples, requiredTolerance,
          maxSamples, seed, nCalibrationSamples),
      model_(model), oas_(oas), discountCurve_(discountCurve),
      generateProxy_(generateProxy) {

    if (!oas_.empty())
        this->registerWith(oas_);
    if (!discountCurve_.empty())
        this->registerWith(discountCurve_);
}

template <class RNG, class S>
void McGaussian1dNonstandardSwaptionEngine<RNG, S>::calculate() const {
    // a lazy object is not thread safe, neither is the caching
    // in gsrprocess. therefore we trigger computations here such
    // that neither lazy object recalculation nor write access
    // during caching occurs in the parallized loop in MonteCarloModel
#ifdef _OPENMP
    boost::shared_ptr<LongstaffSchwartzPathPricer<Path> > tmp =
        this->lsmPathPricer();
    tmp->calibrate();
    model_->numeraire(1.0);
    tmp->operator()(this->pathGenerator()->next().value);
#endif
    // continue with usual calculations
    base_class::calculate();
    // transform the deflated values into plain ones
    this->results_.value *= model_->numeraire(0.0, 0.0, discountCurve_);
    this->results_.errorEstimate *= model_->numeraire(0.0, 0.0, discountCurve_);
    if (generateProxy_) {
        // generate proxy
        this->proxy_ = boost::make_shared<NonstandardSwaption::Proxy>();
        Date today = Settings::instance().evaluationDate();
        // original evaluation date
        // open expiry dates
        std::vector<Date>::const_iterator start =
            std::upper_bound(this->arguments_.exercise->dates().begin(),
                             this->arguments_.exercise->dates().end(), today);
        for (std::vector<Date>::const_iterator i = start;
             i != this->arguments_.exercise->dates().end(); ++i) {
            this->proxy_->expiryDates.push_back(*i);
        }
        // model
        this->proxy_->model = model_;
        // discount curve (may be empty)
        this->proxy_->discount = discountCurve_;
        // oas (may be empty)
        this->proxy_->oas = oas_;
        // regression functions for values
        for (int i = 0; i < static_cast<int>(this->proxy_->expiryDates.size());
             ++i) {
            boost::shared_ptr<LsFunction> lsTmp =
                boost::make_shared<LsFunction>();
            boost::shared_ptr<LongstaffSchwartzProxyPathPricer> pathPricer =
                boost::dynamic_pointer_cast<LongstaffSchwartzProxyPathPricer>(
                    this->pathPricer_);
            lsTmp->coeffItm = pathPricer->coefficientsItm()[i];
            lsTmp->coeffOtm = pathPricer->coefficientsOtm()[i];
            lsTmp->cutoff = pathPricer->cutoff();
            lsTmp->v = pathPricer->basisSystem();
            this->proxy_->regression.push_back(lsTmp);
        }
        this->results_.proxy = this->proxy_;
    }
}

template <class RNG, class S>
void McGaussian1dNonstandardSwaptionEngine<RNG, S>::reset() const {
    base_class::reset();
    this->proxy_ = boost::shared_ptr<NonstandardSwaption::Proxy>();
}

template <class RNG, class S>
boost::shared_ptr<LongstaffSchwartzPathPricer<Path> >
McGaussian1dNonstandardSwaptionEngine<RNG, S>::lsmPathPricer() const {

    TimeGrid grid = this->timeGrid();
    // reduced grid which only contains the exercise times and 0
    std::vector<Real> exerciseTimes;
    exerciseTimes.push_back(0.0);
    for (Size i = 0; i < this->arguments_.exercise->dates().size(); ++i) {
        exerciseTimes.push_back(
            model_->stateProcess()->time(this->arguments_.exercise->date(i)));
    }
    TimeGrid exerciseGrid =
        TimeGrid(exerciseTimes.begin(), exerciseTimes.end());
    boost::shared_ptr<Gaussian1dNonstandardSwaptionPathPricer>
        earlyExercisePricer =
            boost::make_shared<Gaussian1dNonstandardSwaptionPathPricer>(
                model_, &this->arguments_, discountCurve_, oas_);
    // we work with non deflated npvs produced in the early exercise pricer
    // so we pass a dummyCurve, which produces discount factors of 1.0 always
    boost::shared_ptr<YieldTermStructure> dummyCurve =
        boost::make_shared<FlatForward>(0, NullCalendar(), 0.0,
                                        Actual365Fixed());
    if (generateProxy_) {
        return boost::make_shared<LongstaffSchwartzProxyPathPricer>(
            exerciseGrid, earlyExercisePricer, dummyCurve);
    } else {
        return boost::make_shared<LongstaffSchwartzPathPricer<Path> >(
            exerciseGrid, earlyExercisePricer, dummyCurve);
    }
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::
    MakeMcGaussian1dNonstandardSwaptionEngine(
        const boost::shared_ptr<Gaussian1dModel> &model)
    : model_(model), oas_(Handle<Quote>()),
      discount_(Handle<YieldTermStructure>()), brownianBridge_(false),
      antithetic_(false), steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), calibrationSamples_(Null<Size>()),
      maxSamples_(Null<Size>()), tolerance_(Null<Real>()), seed_(0),
      generateProxy_(false) {}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S> &
MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::withSteps(Size steps) {
    steps_ = steps;
    return *this;
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S> &
MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::withStepsPerYear(
    Size steps) {
    stepsPerYear_ = steps;
    return *this;
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S> &
MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::withBrownianBridge(
    bool brownianBridge) {
    brownianBridge_ = brownianBridge;
    return *this;
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S> &
MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::withAntitheticVariate(
    bool b) {
    antithetic_ = b;
    return *this;
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S> &
MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::withSamples(Size samples) {
    QL_REQUIRE(tolerance_ == Null<Real>(), "tolerance already set");
    samples_ = samples;
    return *this;
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S> &
MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::withCalibrationSamples(
    Size samples) {
    calibrationSamples_ = samples;
    return *this;
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S> &
MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::withAbsoluteTolerance(
    Real tolerance) {
    QL_REQUIRE(samples_ == Null<Size>(), "number of samples already set");
    QL_REQUIRE(RNG::allowsErrorEstimate, "chosen random generator policy "
                                         "does not allow an error estimate");
    tolerance_ = tolerance;
    return *this;
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S> &
MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::withMaxSamples(
    Size samples) {
    maxSamples_ = samples;
    return *this;
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S> &
MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::withSeed(BigNatural seed) {
    seed_ = seed;
    return *this;
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S> &
MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::withOas(
    const Handle<Quote> &oas) {
    oas_ = oas;
    return *this;
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S> &
MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::withDiscount(
    const Handle<YieldTermStructure> &discount) {
    discount_ = discount;
    return *this;
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S> &
MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::withProxy(bool b) {
    generateProxy_ = b;
    return *this;
}

template <class RNG, class S>
inline MakeMcGaussian1dNonstandardSwaptionEngine<RNG, S>::
operator boost::shared_ptr<PricingEngine>() const {
    QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
               "number of steps not given");
    QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
               "number of steps overspecified");
    return boost::shared_ptr<PricingEngine>(
        new McGaussian1dNonstandardSwaptionEngine<RNG, S>(
            model_, oas_, discount_, steps_, stepsPerYear_, brownianBridge_,
            antithetic_, samples_, tolerance_, maxSamples_, seed_,
            calibrationSamples_, generateProxy_));
}

} // namespace QuantLib

#endif
