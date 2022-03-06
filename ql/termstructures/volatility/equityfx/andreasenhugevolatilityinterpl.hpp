/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017, 2018 Klaus Spanderen

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

/*! \file andreasenhugevolatilityinterpl.hpp
    \brief Andreasen-Huge local volatility calibration and interpolation
*/

#ifndef quantlib_andreasen_huge_local_volatility_hpp
#define quantlib_andreasen_huge_local_volatility_hpp

#include <ql/quote.hpp>
#include <ql/handle.hpp>
#include <ql/option.hpp>
#include <ql/math/matrix.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

#include <ql/tuple.hpp>
#include <utility>

namespace QuantLib {

    class VanillaOption;
    class YieldTermStructure;
    class FdmMesherComposite;
    class AndreasenHugeCostFunction;

    //! Calibration of a local volatility surface to a sparse grid of options

    /*! References:

        Andreasen J., Huge B., 2010. Volatility Interpolation
        https://ssrn.com/abstract=1694972
    */

    class AndreasenHugeVolatilityInterpl : public LazyObject {

      public:
        enum InterpolationType {PiecewiseConstant, Linear, CubicSpline};
        enum CalibrationType {
            Call = Option::Call, Put = Option::Put, CallPut};

        typedef std::vector<std::pair<
            ext::shared_ptr<VanillaOption>, ext::shared_ptr<Quote> > >
          CalibrationSet;

        AndreasenHugeVolatilityInterpl(
            const CalibrationSet& calibrationSet,
            Handle<Quote> spot,
            Handle<YieldTermStructure> rTS,
            Handle<YieldTermStructure> qTS,
            InterpolationType interpolationType = CubicSpline,
            CalibrationType calibrationType = Call,
            Size nGridPoints = 500,
            Real minStrike = Null<Real>(),
            Real maxStrike = Null<Real>(),
            ext::shared_ptr<OptimizationMethod> optimizationMethod =
                ext::shared_ptr<OptimizationMethod>(new LevenbergMarquardt),
            const EndCriteria& endCriteria = EndCriteria(500, 100, 1e-12, 1e-10, 1e-10));

        Date maxDate() const;
        Real minStrike() const;
        Real maxStrike() const;

        Real fwd(Time t) const;
        const Handle<YieldTermStructure>& riskFreeRate() const;

        // returns min, max and average error in volatility units
        ext::tuple<Real, Real, Real> calibrationError() const;

        // returns the option price of the calibration type. In case
        // of CallPut it return the call option price
        Real optionPrice(Time t, Real strike, Option::Type optionType) const;

        Volatility localVol(Time t, Real strike) const;

      protected:
        void performCalculations() const override;

      private:
        typedef std::map<Time,
            ext::tuple<
                Real,
                ext::shared_ptr<Array>,
                ext::shared_ptr<Interpolation> > > TimeValueCacheType;

        struct SingleStepCalibrationResult {
            Array putNPVs, callNPVs, sigmas;
            ext::shared_ptr<AndreasenHugeCostFunction> costFunction;
        };

        ext::shared_ptr<AndreasenHugeCostFunction> buildCostFunction(
            Size iExpiry, Option::Type optionType,
            const Array& previousNPVs) const;

        Size getExerciseTimeIdx(Time t) const;

        Real getCacheValue(
            Real strike, const TimeValueCacheType::const_iterator& f) const;

        Disposable<Array>
            getPriceSlice(Time t, Option::Type optionType) const;

        Disposable<Array>
            getLocalVolSlice(Time t, Option::Type optionType) const;

        CalibrationSet calibrationSet_;
        const Handle<Quote> spot_;
        const Handle<YieldTermStructure> rTS_;
        const Handle<YieldTermStructure> qTS_;
        const InterpolationType interpolationType_;
        const CalibrationType calibrationType_;

        const Size nGridPoints_;
        const Real minStrike_, maxStrike_;

        const ext::shared_ptr<OptimizationMethod> optimizationMethod_;
        const EndCriteria endCriteria_;

        std::vector<Real> strikes_;
        std::vector<Date> expiries_;
        mutable std::vector<Time> expiryTimes_, dT_;

        std::vector<std::vector<Size> > calibrationMatrix_;
        mutable Real avgError_, minError_, maxError_;

        mutable ext::shared_ptr<FdmMesherComposite> mesher_;
        mutable Array gridPoints_, gridInFwd_;

        mutable std::vector<SingleStepCalibrationResult> calibrationResults_;

        mutable TimeValueCacheType localVolCache_, priceCache_;
    };

}

#endif


#ifndef id_1a13c61166fdd9c32d4d14334e7f7e43
#define id_1a13c61166fdd9c32d4d14334e7f7e43
inline bool test_1a13c61166fdd9c32d4d14334e7f7e43(const int* i) {
    return i != nullptr;
}
#endif
