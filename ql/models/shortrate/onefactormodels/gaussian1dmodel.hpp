/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2015 Peter Caspers

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

/*! \file gaussian1dmodel.hpp
    \brief basic interface for one factor interest rate models

    TODO as it turns out it is not optimal for all implementations to work
    with the normalized state variable y instead of the original x (e.g.
    in the monte carlo swaption engine or in the lgm model), revisit this

    TODO check if registration with the evaluation date is needed, shouldn't
    it be enough to register with the termstructure (as it is done in derived
    classes)
*/

// uncomment to enable NTL support (see below for more details and references)
// #define GAUSS1D_ENABLE_NTL

#ifndef quantlib_gaussian1dmodel_hpp
#define quantlib_gaussian1dmodel_hpp

#include <ql/models/model.hpp>
#include <ql/models/parameter.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/time/date.hpp>
#include <ql/time/period.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/stochasticprocess.hpp>
#include <ql/utilities/null.hpp>
#include <ql/patterns/lazyobject.hpp>

#ifdef GAUSS1D_ENABLE_NTL
#include <boost/math/bindings/rr.hpp>
#endif

#if defined(__GNUC__) &&                                                       \
    (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/math/special_functions/erf.hpp>
#include <boost/unordered_map.hpp>
#if defined(__GNUC__) &&                                                       \
    (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif

namespace QuantLib {

/*! One factor interest rate model interface class
    The only methods that must be implemented by subclasses
    are the numeraire and zerobond methods for an input array
    of state variable values. The variable $y$ is understood
    to be the standardized (zero mean, unit variance) version
    of the model's original state variable $x$.

    NTL support may be enabled by defining GAUSS1D_ENABLE_NTL in this
    file. For details on NTL see
             http://www.shoup.net/ntl/

    \warning the variance of the state process conditional on
    $x(t)=x$ must be independent of the value of $x$

*/

class Gaussian1dModel : public TermStructureConsistentModel, public LazyObject {
  public:
    const boost::shared_ptr<StochasticProcess1D> stateProcess() const;

    const Real numeraire(const Time t, const Real y = 0.0,
                         const Handle<YieldTermStructure> &yts =
                             Handle<YieldTermStructure>()) const;

    const Real zerobond(
        const Time T, const Time t = 0.0, const Real y = 0.0,
        const Handle<YieldTermStructure> &yts = Handle<YieldTermStructure>(),
        const bool adjusted = false) const;

    const Real deflatedZerobond(
        const Time T, const Time t = 0.0, const Real y = 0.0,
        const Handle<YieldTermStructure> &yts = Handle<YieldTermStructure>(),
        const Handle<YieldTermStructure> &ytsNumeraire =
            Handle<YieldTermStructure>(),
        const bool adjusted = false) const;

    const Real numeraire(const Date &referenceDate, const Real y = 0.0,
                         const Handle<YieldTermStructure> &yts =
                             Handle<YieldTermStructure>()) const;

    const Real zerobond(
        const Date &maturity, const Date &referenceDate = Null<Date>(),
        const Real y = 0.0,
        const Handle<YieldTermStructure> &yts = Handle<YieldTermStructure>(),
        const bool adjusted = false) const;

    const Real deflatedZerobond(
        const Date &maturity, const Date &referenceDate = Null<Date>(),
        const Real y = 0.0,
        const Handle<YieldTermStructure> &yts = Handle<YieldTermStructure>(),
        const Handle<YieldTermStructure> &ytsNumeraire =
            Handle<YieldTermStructure>(),
        const bool adjusted = false) const;

    const Real zerobondOption(
        const Option::Type &type, const Date &expiry, const Date &valueDate,
        const Date &maturity, const Rate strike,
        const Date &referenceDate = Null<Date>(), const Real y = 0.0,
        const Handle<YieldTermStructure> &yts = Handle<YieldTermStructure>(),
        const Real yStdDevs = 7.0, const Size yGridPoints = 64,
        const bool extrapolatePayoff = true,
        const bool flatPayoffExtrapolation = false,
        const bool adjusted = false) const;

    const Real forwardRate(
        const Date &fixing, const Date &referenceDate = Null<Date>(),
        const Real y = 0.0,
        boost::shared_ptr<IborIndex> iborIdx = boost::shared_ptr<IborIndex>(),
        const bool adjusted = false) const;

    const Real swapRate(
        const Date &fixing, const Period &tenor,
        const Date &referenceDate = Null<Date>(), const Real y = 0.0,
        boost::shared_ptr<SwapIndex> swapIdx = boost::shared_ptr<SwapIndex>(),
        const bool adjusted = false) const;

    const Real swapAnnuity(
        const Date &fixing, const Period &tenor,
        const Date &referenceDate = Null<Date>(), const Real y = 0.0,
        boost::shared_ptr<SwapIndex> swapIdx = boost::shared_ptr<SwapIndex>(),
        const bool adjusted = false) const;

    const Real deflatedSwapAnnuity(
        const Date &fixing, const Period &tenor,
        const Date &referenceDate = Null<Date>(), const Real y = 0.0,
        boost::shared_ptr<SwapIndex> swapIdx = boost::shared_ptr<SwapIndex>(),
        const bool adjusted = false,
        const Handle<YieldTermStructure> &ytsNumeraire =
            Handle<YieldTermStructure>()) const;

    /*! Computes the integral
    \f[ {2\pi}^{-0.5} \int_{a}^{b} p(x) \exp{-0.5*x*x} \mathrm{d}x \f]
    with
    \f[ p(x) = ax^4+bx^3+cx^2+dx+e \f].
    */
    const static Real gaussianPolynomialIntegral(const Real a, const Real b,
                                                 const Real c, const Real d,
                                                 const Real e, const Real x0,
                                                 const Real x1);

    /*! Computes the integral
    \f[ {2\pi}^{-0.5} \int_{a}^{b} p(x) \exp{-0.5*x*x} \mathrm{d}x \f]
    with
    \f[ p(x) = a(x-h)^4+b(x-h)^3+c(x-h)^2+d(x-h)+e \f].
    */
    const static Real
    gaussianShiftedPolynomialIntegral(const Real a, const Real b, const Real c,
                                      const Real d, const Real e, const Real h,
                                      const Real x0, const Real x1);

    /*! Generates a grid of values for the standardized state variable $y$
       at time $T$
        conditional on $y(t)=y$, covering yStdDevs standard deviations
       consisting of
        2*gridPoints+1 points */

    const Disposable<Array> yGrid(const Real yStdDevs, const int gridPoints,
                                  const Real T = 1.0, const Real t = 0,
                                  const Real y = 0) const;

    /*! Computes the standardized model state from the original one
        We use that the standard deviation is independent of $x$ here ! */
    const Real y(const Real x, const Time t) {
        return (x - stateProcess_->expectation(0.0, 0.0, t)) /
               stateProcess_->stdDeviation(0.0, 0.0, t);
    }

  private:
    const Real
    swapAnnuityImpl(const Date &fixing, const Period &tenor,
                    const Date &referenceDate, const Real y,
                    boost::shared_ptr<SwapIndex> swapIdx, const bool adjusted,
                    const bool deflated,
                    const Handle<YieldTermStructure> &ytsNumeraire) const;

    // It is of great importance for performance reasons to cache underlying
    // swaps generated from indexes. In addition the indexes may only be given
    // as templates for the conventions with the tenor replaced by the actual
    // one later on.

    struct CachedSwapKey {
        const boost::shared_ptr<SwapIndex> index;
        const Date fixing;
        const Period tenor;
        const bool operator==(const CachedSwapKey &o) const {
            return index->name() == o.index->name() && fixing == o.fixing &&
                   tenor == o.tenor;
        }
    };

    struct CachedSwapKeyHasher
        : std::unary_function<CachedSwapKey, std::size_t> {
        std::size_t operator()(CachedSwapKey const &x) const {
            std::size_t seed = 0;
            boost::hash_combine(seed, x.index->name());
            boost::hash_combine(seed, x.fixing.serialNumber());
            boost::hash_combine(seed, x.tenor.length());
            boost::hash_combine(seed, x.tenor.units());
            return seed;
        }
    };

    typedef boost::unordered_map<CachedSwapKey, boost::shared_ptr<VanillaSwap>,
                                 CachedSwapKeyHasher> CacheType;

    mutable CacheType swapCache_;

  protected:
    // we let derived classes register with the termstructure
    Gaussian1dModel(const Handle<YieldTermStructure> &yieldTermStructure)
        : TermStructureConsistentModel(yieldTermStructure) {
        registerWith(Settings::instance().evaluationDate());
    }

    virtual ~Gaussian1dModel() {}

    virtual const Real
    numeraireImpl(const Time t, const Real y,
                  const Handle<YieldTermStructure> &yts) const = 0;

    virtual const Real zerobondImpl(const Time T, const Time t, const Real y,
                                    const Handle<YieldTermStructure> &yts,
                                    const bool adjusted) const = 0;

    /* a default implementation is given, but in some cases (as e.g.
       for the LGM model) it may be more efficient to provide an
       own implementation */
    virtual const Real
    deflatedZerobondImpl(const Time T, const Time t, const Real y,
                         const Handle<YieldTermStructure> &yts,
                         const Handle<YieldTermStructure> &ytsNumeraire,
                         const bool adjusted) const;

    /* return true in implementations if deflatedZerobond is computed
       more efficiently than zerobond */
    virtual bool preferDeflatedZerobond() const {
        return false;
    }

    void performCalculations() const {
        evaluationDate_ = Settings::instance().evaluationDate();
        enforcesTodaysHistoricFixings_ =
            Settings::instance().enforcesTodaysHistoricFixings();
    }

    void generateArguments() {
        calculate();
        notifyObservers();
    }

    // retrieve underlying swap from cache if possible, otherwise
    // create it and store it in the cache
    boost::shared_ptr<VanillaSwap>
    underlyingSwap(const boost::shared_ptr<SwapIndex> &index,
                   const Date &expiry, const Period &tenor) const {

        CachedSwapKey k = {index, expiry, tenor};
        CacheType::iterator i = swapCache_.find(k);
        if (i == swapCache_.end()) {
            boost::shared_ptr<VanillaSwap> underlying =
                index->clone(tenor)->underlyingSwap(expiry);
            swapCache_.insert(std::make_pair(k, underlying));
            return underlying;
        }
        return i->second;
    }

    boost::shared_ptr<StochasticProcess1D> stateProcess_;
    mutable Date evaluationDate_;
    mutable bool enforcesTodaysHistoricFixings_;
};

inline const boost::shared_ptr<StochasticProcess1D>
Gaussian1dModel::stateProcess() const {

    QL_REQUIRE(stateProcess_ != NULL, "state process not set");
    return stateProcess_;
}

inline const Real
Gaussian1dModel::numeraire(const Time t, const Real y,
                           const Handle<YieldTermStructure> &yts) const {

    return numeraireImpl(t, y, yts);
}

inline const Real
Gaussian1dModel::zerobond(const Time T, const Time t, const Real y,
                          const Handle<YieldTermStructure> &yts,
                          const bool adjusted) const {
    return zerobondImpl(T, t, y, yts, adjusted);
}

inline const Real
Gaussian1dModel::deflatedZerobond(const Time T, const Time t, const Real y,
                                  const Handle<YieldTermStructure> &yts,
                                  const Handle<YieldTermStructure> &ytsNumeraire,
                                  const bool adjusted) const {
    return deflatedZerobondImpl(T, t, y, yts, ytsNumeraire, adjusted);
}

inline const Real
Gaussian1dModel::deflatedZerobondImpl(const Time T, const Time t, const Real y,
                                      const Handle<YieldTermStructure> &yts,
                                      const Handle<YieldTermStructure> &ytsNumeraire,
                                      const bool adjusted) const {
    return zerobondImpl(T, t, y, yts, adjusted) / numeraire(t, y, ytsNumeraire);
}

inline const Real
Gaussian1dModel::numeraire(const Date &referenceDate, const Real y,
                           const Handle<YieldTermStructure> &yts) const {

    return numeraire(termStructure()->timeFromReference(referenceDate), y, yts);
}

inline const Real
Gaussian1dModel::zerobond(const Date &maturity, const Date &referenceDate,
                          const Real y, const Handle<YieldTermStructure> &yts,
                          const bool adjusted) const {

    return zerobond(termStructure()->timeFromReference(maturity),
                    referenceDate != Null<Date>()
                        ? termStructure()->timeFromReference(referenceDate)
                        : 0.0,
                    y, yts, adjusted);
}

inline const Real Gaussian1dModel::deflatedZerobond(
    const Date &maturity, const Date &referenceDate, const Real y,
    const Handle<YieldTermStructure> &yts,
    const Handle<YieldTermStructure> &ytsNumeraire,
    const bool adjusted) const {

    return deflatedZerobond(
        termStructure()->timeFromReference(maturity),
        referenceDate != Null<Date>()
            ? termStructure()->timeFromReference(referenceDate)
            : 0.0,
        y, yts, ytsNumeraire, adjusted);
}
} // namespace QuantLib

#endif
