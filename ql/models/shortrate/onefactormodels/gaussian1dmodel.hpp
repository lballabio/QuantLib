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

#if BOOST_VERSION < 106700
#include <boost/functional/hash.hpp>
#else
#include <boost/container_hash/hash.hpp>
#endif

#include <boost/math/special_functions/erf.hpp>

#include <unordered_map>

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
    ext::shared_ptr<StochasticProcess1D> stateProcess() const;

    Real numeraire(Time t,
                   Real y = 0.0,
                   const Handle<YieldTermStructure>& yts = Handle<YieldTermStructure>()) const;

    Real zerobond(Time T,
                  Time t = 0.0,
                  Real y = 0.0,
                  const Handle<YieldTermStructure>& yts = Handle<YieldTermStructure>()) const;

    Real numeraire(const Date& referenceDate,
                   Real y = 0.0,
                   const Handle<YieldTermStructure>& yts = Handle<YieldTermStructure>()) const;

    Real zerobond(const Date& maturity,
                  const Date& referenceDate = Null<Date>(),
                  Real y = 0.0,
                  const Handle<YieldTermStructure>& yts = Handle<YieldTermStructure>()) const;

    Real zerobondOption(const Option::Type& type,
                        const Date& expiry,
                        const Date& valueDate,
                        const Date& maturity,
                        Rate strike,
                        const Date& referenceDate = Null<Date>(),
                        Real y = 0.0,
                        const Handle<YieldTermStructure>& yts = Handle<YieldTermStructure>(),
                        Real yStdDevs = 7.0,
                        Size yGridPoints = 64,
                        bool extrapolatePayoff = true,
                        bool flatPayoffExtrapolation = false) const;

    Real
    forwardRate(const Date& fixing,
                const Date& referenceDate = Null<Date>(),
                Real y = 0.0,
                const ext::shared_ptr<IborIndex>& iborIdx = ext::shared_ptr<IborIndex>()) const;

    Real swapRate(const Date& fixing,
                  const Period& tenor,
                  const Date& referenceDate = Null<Date>(),
                  Real y = 0.0,
                  const ext::shared_ptr<SwapIndex>& swapIdx = ext::shared_ptr<SwapIndex>()) const;

    Real
    swapAnnuity(const Date& fixing,
                const Period& tenor,
                const Date& referenceDate = Null<Date>(),
                Real y = 0.0,
                const ext::shared_ptr<SwapIndex>& swapIdx = ext::shared_ptr<SwapIndex>()) const;

    /*! Computes the integral
    \f[ {2\pi}^{-0.5} \int_{a}^{b} p(x) \exp{-0.5*x*x} \mathrm{d}x \f]
    with
    \f[ p(x) = ax^4+bx^3+cx^2+dx+e \f].
    */
    static Real
    gaussianPolynomialIntegral(Real a, Real b, Real c, Real d, Real e, Real x0, Real x1);

    /*! Computes the integral
    \f[ {2\pi}^{-0.5} \int_{a}^{b} p(x) \exp{-0.5*x*x} \mathrm{d}x \f]
    with
    \f[ p(x) = a(x-h)^4+b(x-h)^3+c(x-h)^2+d(x-h)+e \f].
    */
    static Real gaussianShiftedPolynomialIntegral(
        Real a, Real b, Real c, Real d, Real e, Real h, Real x0, Real x1);

    /*! Generates a grid of values for the standardized state variable $y$
       at time $T$
        conditional on $y(t)=y$, covering yStdDevs standard deviations
       consisting of
        2*gridPoints+1 points */

    Array yGrid(Real yStdDevs, int gridPoints, Real T = 1.0, Real t = 0, Real y = 0) const;

  private:
    // It is of great importance for performance reasons to cache underlying
    // swaps generated from indexes. In addition the indexes may only be given
    // as templates for the conventions with the tenor replaced by the actual
    // one later on.

    struct CachedSwapKey {
        const ext::shared_ptr<SwapIndex> index;
        const Date fixing;
        const Period tenor;
        bool operator==(const CachedSwapKey &o) const {
            return index->name() == o.index->name() && fixing == o.fixing &&
                   tenor == o.tenor;
        }
    };

    struct CachedSwapKeyHasher {
        std::size_t operator()(CachedSwapKey const &x) const {
            std::size_t seed = 0;
            boost::hash_combine(seed, x.index->name());
            boost::hash_combine(seed, x.fixing.serialNumber());
            boost::hash_combine(seed, x.tenor.length());
            boost::hash_combine(seed, x.tenor.units());
            return seed;
        }
    };

    mutable std::unordered_map<CachedSwapKey, ext::shared_ptr<VanillaSwap>, CachedSwapKeyHasher> swapCache_;

  protected:
    // we let derived classes register with the termstructure
    Gaussian1dModel(const Handle<YieldTermStructure> &yieldTermStructure)
        : TermStructureConsistentModel(yieldTermStructure) {
        registerWith(Settings::instance().evaluationDate());
    }

    virtual Real numeraireImpl(Time t, Real y, const Handle<YieldTermStructure>& yts) const = 0;

    virtual Real
    zerobondImpl(Time T, Time t, Real y, const Handle<YieldTermStructure>& yts) const = 0;

    void performCalculations() const override {
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
    ext::shared_ptr<VanillaSwap>
    underlyingSwap(const ext::shared_ptr<SwapIndex> &index,
                   const Date &expiry, const Period &tenor) const {

        CachedSwapKey k = {index, expiry, tenor};
        auto i = swapCache_.find(k);
        if (i == swapCache_.end()) {
            ext::shared_ptr<VanillaSwap> underlying =
                index->clone(tenor)->underlyingSwap(expiry);
            swapCache_.insert(std::make_pair(k, underlying));
            return underlying;
        }
        return i->second;
    }

    ext::shared_ptr<StochasticProcess1D> stateProcess_;
    mutable Date evaluationDate_;
    mutable bool enforcesTodaysHistoricFixings_;
};

inline ext::shared_ptr<StochasticProcess1D> Gaussian1dModel::stateProcess() const {

    QL_REQUIRE(stateProcess_ != nullptr, "state process not set");
    return stateProcess_;
}

inline Real
Gaussian1dModel::numeraire(const Time t, const Real y,
                           const Handle<YieldTermStructure> &yts) const {

    return numeraireImpl(t, y, yts);
}

inline Real
Gaussian1dModel::zerobond(const Time T, const Time t, const Real y,
                          const Handle<YieldTermStructure> &yts) const {
    return zerobondImpl(T, t, y, yts);
}

inline Real
Gaussian1dModel::numeraire(const Date &referenceDate, const Real y,
                           const Handle<YieldTermStructure> &yts) const {

    return numeraire(termStructure()->timeFromReference(referenceDate), y, yts);
}

inline Real
Gaussian1dModel::zerobond(const Date &maturity, const Date &referenceDate,
                          const Real y, const Handle<YieldTermStructure> &yts) const {

    return zerobond(termStructure()->timeFromReference(maturity),
                    referenceDate != Null<Date>()
                        ? termStructure()->timeFromReference(referenceDate)
                        : 0.0,
                    y, yts);
}
}

#endif
