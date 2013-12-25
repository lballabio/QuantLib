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

/*! \file gaussian1dmodel.hpp
    \brief basic interface for one factor interest rate models
*/

// uncomment to enable NTL support (see below for more details and references)
// #define GAUSS1D_ENABLE_NTL 

#ifndef quantlib_gaussian1dmodel_hpp
#define quantlib_gaussian1dmodel_hpp

#include <ql/models/model.hpp>
#include <ql/models/parameter.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
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

#include <boost/math/special_functions/erf.hpp>

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

    class Gaussian1dModel : public TermStructureConsistentModel,
                            public LazyObject {

      public:

        const boost::shared_ptr<StochasticProcess1D> stateProcess() const;

        const Real numeraire(const Time t, const Real y = 0.0,
                             const Handle<YieldTermStructure> &yts =
                                 Handle<YieldTermStructure>()) const;

        const Real zerobond(const Time T, const Time t = 0.0,
                            const Real y = 0.0,
                            const Handle<YieldTermStructure> &yts =
                                Handle<YieldTermStructure>()) const;

        const Real numeraire(const Date &referenceDate, const Real y = 0.0,
                             const Handle<YieldTermStructure> &yts =
                                 Handle<YieldTermStructure>()) const;

        const Real zerobond(const Date &maturity,
                            const Date &referenceDate = Null<Date>(),
                            const Real y = 0.0,
                            const Handle<YieldTermStructure> &yts =
                                Handle<YieldTermStructure>()) const;

        const Real zerobondOption(
            const Option::Type &type, const Date &expiry, const Date &valueDate,
            const Date &maturity, const Rate strike,
            const Date &referenceDate = Null<Date>(), const Real y = 0.0,
            const Handle<YieldTermStructure> &yts =
                Handle<YieldTermStructure>(),
            const Real yStdDevs = 7.0, const Size yGridPoints = 64,
            const bool extrapolatePayoff = true,
            const bool flatPayoffExtrapolation = false) const;

        const Real forwardRate(const Date &fixing,
                               const Date &referenceDate = Null<Date>(),
                               const Real y = 0.0,
                               boost::shared_ptr<IborIndex> iborIdx =
                                   boost::shared_ptr<IborIndex>()) const;

        const Real swapRate(const Date &fixing, const Period &tenor,
                            const Date &referenceDate = Null<Date>(),
                            const Real y = 0.0,
                            boost::shared_ptr<SwapIndex> swapIdx =
                                boost::shared_ptr<SwapIndex>()) const;

        const Real swapAnnuity(const Date &fixing, const Period &tenor,
                               const Date &referenceDate = Null<Date>(),
                               const Real y = 0.0,
                               boost::shared_ptr<SwapIndex> swapIdx =
                                   boost::shared_ptr<SwapIndex>()) const;

        /*! Computes the integral
        \f[ {2\pi}^{-0.5} \int_{a}^{b} p(x) \exp{-0.5*x*x} \mathrm{d}x \f]
        with
        \f[ p(x) = ax^4+bx^3+cx^2+dx+e \f].
        */
        const static Real gaussianPolynomialIntegral(const Real a, const Real b,
                                                     const Real c, const Real d,
                                                     const Real e,
                                                     const Real x0,
                                                     const Real x1);

        /*! Computes the integral
        \f[ {2\pi}^{-0.5} \int_{a}^{b} p(x) \exp{-0.5*x*x} \mathrm{d}x \f]
        with
        \f[ p(x) = a(x-h)^4+b(x-h)^3+c(x-h)^2+d(x-h)+e \f].
        */
        const static Real gaussianShiftedPolynomialIntegral(
            const Real a, const Real b, const Real c, const Real d,
            const Real e, const Real h, const Real x0, const Real x1);

        /*! Generates a grid of values for the standardized state variable $y$
           at time $T$
            conditional on $y(t)=y$, covering yStdDevs standard deviations
           consisting of
            2*gridPoints+1 points */

        const Disposable<Array> yGrid(const Real yStdDevs, const int gridPoints,
                                      const Real T = 1.0, const Real t = 0,
                                      const Real y = 0) const;

      protected:

        // we let derived classes register with the termstructure
        Gaussian1dModel(const Handle<YieldTermStructure> &yieldTermStructure)
            : TermStructureConsistentModel(yieldTermStructure) {}

        virtual ~Gaussian1dModel() {}

        virtual const Real
        numeraireImpl(const Time t, const Real y,
                      const Handle<YieldTermStructure> &yts) const = 0;

        virtual const Real
        zerobondImpl(const Time T, const Time t, const Real y,
                     const Handle<YieldTermStructure> &yts) const = 0;

        void performCalculations() const {}

        void generateArguments() {
            calculate();
            notifyObservers();
        }

        boost::shared_ptr<StochasticProcess1D> stateProcess_;
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
                              const Handle<YieldTermStructure> &yts) const {

        return zerobondImpl(T, t, y, yts);
    }

    inline const Real
    Gaussian1dModel::numeraire(const Date &referenceDate, const Real y,
                               const Handle<YieldTermStructure> &yts) const {

        return numeraire(termStructure()->timeFromReference(referenceDate), y,
                         yts);
    }

    inline const Real
    Gaussian1dModel::zerobond(const Date &maturity, const Date &referenceDate,
                              const Real y,
                              const Handle<YieldTermStructure> &yts) const {

        return zerobond(termStructure()->timeFromReference(maturity),
                        referenceDate != Null<Date>()
                            ? termStructure()->timeFromReference(referenceDate)
                            : 0.0,
                        y, yts);
    }

}

#endif
