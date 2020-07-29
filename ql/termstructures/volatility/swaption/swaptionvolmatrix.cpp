/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2008 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
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

#include <ql/termstructures/volatility/swaption/swaptionvolmatrix.hpp>
#include <ql/termstructures/volatility/flatsmilesection.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/math/interpolations/flatextrapolation2d.hpp>


namespace QuantLib {

    // floating reference date, floating market data
    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                    const Calendar& cal,
                    BusinessDayConvention bdc,
                    const std::vector<Period>& optionT,
                    const std::vector<Period>& swapT,
                    const std::vector<std::vector<Handle<Quote> > >& vols,
                    const DayCounter& dc,
                    const bool flatExtrapolation,
                    const VolatilityType type,
                    const std::vector<std::vector<Real> >& shifts)
    : SwaptionVolatilityDiscrete(optionT, swapT, 0, cal, bdc, dc),
      volHandles_(vols), shiftValues_(shifts),
      volatilities_(vols.size(), vols.front().size()),
      shifts_(vols.size(), vols.front().size(), 0.0), volatilityType_(type) {
        checkInputs(volatilities_.rows(), volatilities_.columns(), shifts.size(),
                    shifts.empty() ? 0 : shifts.front().size());
        registerWithMarketData();
        if (flatExtrapolation) {
            interpolation_ =
                FlatExtrapolator2D(ext::make_shared<BilinearInterpolation>(
                    swapLengths_.begin(), swapLengths_.end(),
                    optionTimes_.begin(), optionTimes_.end(), volatilities_));
            interpolationShifts_ =
                FlatExtrapolator2D(ext::make_shared<BilinearInterpolation>(
                    swapLengths_.begin(), swapLengths_.end(),
                    optionTimes_.begin(), optionTimes_.end(), shifts_));
        } else {
            interpolation_ = BilinearInterpolation(
                swapLengths_.begin(), swapLengths_.end(), optionTimes_.begin(),
                optionTimes_.end(), volatilities_);
            interpolationShifts_ = BilinearInterpolation(
                swapLengths_.begin(), swapLengths_.end(), optionTimes_.begin(),
                optionTimes_.end(), shifts_);
        }
   }

    // fixed reference date, floating market data
    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                    const Date& refDate,
                    const Calendar& cal,
                    BusinessDayConvention bdc,
                    const std::vector<Period>& optionT,
                    const std::vector<Period>& swapT,
                    const std::vector<std::vector<Handle<Quote> > >& vols,
                    const DayCounter& dc,
                    const bool flatExtrapolation,
                    const VolatilityType type,
                    const std::vector<std::vector<Real> >& shifts)
    : SwaptionVolatilityDiscrete(optionT, swapT, refDate, cal, bdc, dc),
      volHandles_(vols), shiftValues_(shifts),
      volatilities_(vols.size(), vols.front().size()),
      shifts_(vols.size(), vols.front().size(), 0.0), volatilityType_(type) {
        checkInputs(volatilities_.rows(), volatilities_.columns(), shifts.size(),
                    shifts.empty() ? 0 : shifts.front().size());
        registerWithMarketData();
        if (flatExtrapolation) {
            interpolation_ =
                FlatExtrapolator2D(ext::make_shared<BilinearInterpolation>(
                    swapLengths_.begin(), swapLengths_.end(),
                    optionTimes_.begin(), optionTimes_.end(), volatilities_));
            interpolationShifts_ =
                FlatExtrapolator2D(ext::make_shared<BilinearInterpolation>(
                    swapLengths_.begin(), swapLengths_.end(),
                    optionTimes_.begin(), optionTimes_.end(), shifts_));
        } else {
            interpolation_ = BilinearInterpolation(
                swapLengths_.begin(), swapLengths_.end(), optionTimes_.begin(),
                optionTimes_.end(), volatilities_);
            interpolationShifts_ = BilinearInterpolation(
                swapLengths_.begin(), swapLengths_.end(), optionTimes_.begin(),
                optionTimes_.end(), shifts_);
        }
    }

    // floating reference date, fixed market data
    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                        const Calendar& cal,
                        BusinessDayConvention bdc,
                        const std::vector<Period>& optionT,
                        const std::vector<Period>& swapT,
                        const Matrix& vols,
                        const DayCounter& dc,
                        const bool flatExtrapolation,
                        const VolatilityType type,
                        const Matrix& shifts)
    : SwaptionVolatilityDiscrete(optionT, swapT, 0, cal, bdc, dc),
      volHandles_(vols.rows()), shiftValues_(vols.rows()),
      volatilities_(vols.rows(), vols.columns()),
      shifts_(vols.rows(), vols.columns(), 0.0), volatilityType_(type) {

        checkInputs(vols.rows(), vols.columns(), shifts.rows(), shifts.columns());

        // fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<vols.rows(); ++i) {
            volHandles_[i].resize(vols.columns());
            shiftValues_[i].resize(vols.columns());
            for (Size j=0; j<vols.columns(); ++j) {
                volHandles_[i][j] = Handle<Quote>(ext::shared_ptr<Quote>(new
                    SimpleQuote(vols[i][j])));
                shiftValues_[i][j] = shifts.rows() > 0 ? shifts[i][j] : 0.0;
            }
        }
        if (flatExtrapolation) {
            interpolation_ =
                FlatExtrapolator2D(ext::make_shared<BilinearInterpolation>(
                    swapLengths_.begin(), swapLengths_.end(),
                    optionTimes_.begin(), optionTimes_.end(), volatilities_));
            interpolationShifts_ =
                FlatExtrapolator2D(ext::make_shared<BilinearInterpolation>(
                    swapLengths_.begin(), swapLengths_.end(),
                    optionTimes_.begin(), optionTimes_.end(), shifts_));
        } else {
            interpolation_ = BilinearInterpolation(
                swapLengths_.begin(), swapLengths_.end(), optionTimes_.begin(),
                optionTimes_.end(), volatilities_);
            interpolationShifts_ = BilinearInterpolation(
                swapLengths_.begin(), swapLengths_.end(), optionTimes_.begin(),
                optionTimes_.end(), shifts_);
        }
    }

    // fixed reference date, fixed market data
    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                        const Date& refDate,
                        const Calendar& cal,
                        BusinessDayConvention bdc,
                        const std::vector<Period>& optionT,
                        const std::vector<Period>& swapT,
                        const Matrix& vols,
                        const DayCounter& dc,
                        const bool flatExtrapolation,
                        const VolatilityType type,
                        const Matrix& shifts)
    : SwaptionVolatilityDiscrete(optionT, swapT, refDate, cal, bdc, dc),
      volHandles_(vols.rows()), shiftValues_(vols.rows()),
      volatilities_(vols.rows(), vols.columns()),
      shifts_(shifts.rows(), shifts.columns(), 0.0), volatilityType_(type) {

        checkInputs(vols.rows(), vols.columns(), shifts.rows(), shifts.columns());

        // fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<vols.rows(); ++i) {
            volHandles_[i].resize(vols.columns());
            shiftValues_[i].resize(vols.columns());
            for (Size j=0; j<vols.columns(); ++j) {
                volHandles_[i][j] = Handle<Quote>(ext::shared_ptr<Quote>(new
                    SimpleQuote(vols[i][j])));
                shiftValues_[i][j] = shifts.rows() > 0 ? shifts[i][j] : 0.0;
            }
        }
        if (flatExtrapolation) {
            interpolation_ =
                FlatExtrapolator2D(ext::make_shared<BilinearInterpolation>(
                    swapLengths_.begin(), swapLengths_.end(),
                    optionTimes_.begin(), optionTimes_.end(), volatilities_));
            interpolationShifts_ =
                FlatExtrapolator2D(ext::make_shared<BilinearInterpolation>(
                    swapLengths_.begin(), swapLengths_.end(),
                    optionTimes_.begin(), optionTimes_.end(), shifts_));
        } else {
            interpolation_ = BilinearInterpolation(
                swapLengths_.begin(), swapLengths_.end(), optionTimes_.begin(),
                optionTimes_.end(), volatilities_);
            interpolationShifts_ = BilinearInterpolation(
                swapLengths_.begin(), swapLengths_.end(), optionTimes_.begin(),
                optionTimes_.end(), shifts_);
        }
    }

    // fixed reference date and fixed market data, option dates
    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                    const Date& today,
                    const Calendar& calendar,
                    BusinessDayConvention bdc,
                    const std::vector<Date>& optionDates,
                    const std::vector<Period>& swapT,
                    const Matrix& vols,
                    const DayCounter& dc,
                    const bool flatExtrapolation,
                    const VolatilityType type,
                    const Matrix& shifts)
    : SwaptionVolatilityDiscrete(optionDates, swapT, today, calendar, bdc, dc),
      volHandles_(vols.rows()), shiftValues_(vols.rows()),
      volatilities_(vols.rows(), vols.columns()),
      shifts_(shifts.rows(),shifts.columns(),0.0), volatilityType_(type) {

        checkInputs(vols.rows(), vols.columns(), shifts.rows(), shifts.columns());

        // fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<vols.rows(); ++i) {
            volHandles_[i].resize(vols.columns());
            shiftValues_[i].resize(vols.columns());
            for (Size j=0; j<vols.columns(); ++j) {
                volHandles_[i][j] = Handle<Quote>(ext::shared_ptr<Quote>(new
                    SimpleQuote(vols[i][j])));
                shiftValues_[i][j] = shifts.rows() > 0 ? shifts[i][j] : 0.0;
            }
        }
        if (flatExtrapolation) {
            interpolation_ =
                FlatExtrapolator2D(ext::make_shared<BilinearInterpolation>(
                    swapLengths_.begin(), swapLengths_.end(),
                    optionTimes_.begin(), optionTimes_.end(), volatilities_));
            interpolationShifts_ =
                FlatExtrapolator2D(ext::make_shared<BilinearInterpolation>(
                    swapLengths_.begin(), swapLengths_.end(),
                    optionTimes_.begin(), optionTimes_.end(), shifts_));
        } else {
            interpolation_ = BilinearInterpolation(
                swapLengths_.begin(), swapLengths_.end(), optionTimes_.begin(),
                optionTimes_.end(), volatilities_);
            interpolationShifts_ = BilinearInterpolation(
                swapLengths_.begin(), swapLengths_.end(), optionTimes_.begin(),
                optionTimes_.end(), shifts_);
        }
    }


    void SwaptionVolatilityMatrix::checkInputs(Size volRows,
                                               Size volsColumns,
                                               Size shiftRows,
                                               Size shiftsColumns) const {
        QL_REQUIRE(nOptionTenors_==volRows,
                   "mismatch between number of option dates (" <<
                   nOptionTenors_ << ") and number of rows (" << volRows <<
                   ") in the vol matrix");
        QL_REQUIRE(nSwapTenors_==volsColumns,
                   "mismatch between number of swap tenors (" <<
                   nSwapTenors_ << ") and number of columns (" << volsColumns <<
                   ") in the vol matrix");

        if(shiftRows==0 && shiftsColumns==0) {
            shifts_ = Matrix(volRows, volsColumns, 0.0);
            shiftRows = volRows;
            shiftsColumns = volsColumns;
        }

        QL_REQUIRE(nOptionTenors_==shiftRows,
                   "mismatch between number of option dates (" <<
                   nOptionTenors_ << ") and number of rows (" << shiftRows <<
                   ") in the shift matrix");
        QL_REQUIRE(nSwapTenors_==shiftsColumns,
                   "mismatch between number of swap tenors (" <<
                   nSwapTenors_ << ") and number of columns (" << shiftsColumns <<
                   ") in the shift matrix");

    }

    void SwaptionVolatilityMatrix::registerWithMarketData()
    {
        for (Size i=0; i<volHandles_.size(); ++i)
            for (Size j=0; j<volHandles_.front().size(); ++j)
                registerWith(volHandles_[i][j]);
    }

    void SwaptionVolatilityMatrix::performCalculations() const {

        SwaptionVolatilityDiscrete::performCalculations();

        // we might use iterators here...
        for (Size i=0; i<volatilities_.rows(); ++i) {
            for (Size j=0; j<volatilities_.columns(); ++j) {
                volatilities_[i][j] = volHandles_[i][j]->value();
                if (!shiftValues_.empty())
                    shifts_[i][j] = shiftValues_[i][j];
            }
        }
    }

    //ext::shared_ptr<SmileSection>
    //SwaptionVolatilityMatrix::smileSectionImpl(const Date& d,
    //                                           const Period& swapTenor) const {
    //    Time optionTime = timeFromReference(d);
    //    Time swapLength = convertSwapTenor(swapTenor);
    //    // dummy strike
    //    Volatility atmVol = volatilityImpl(optionTime, swapLength, 0.05);
    //    return ext::shared_ptr<SmileSection>(new
    //        FlatSmileSection(d, atmVol, dayCounter(), referenceDate()));
    //}

    ext::shared_ptr<SmileSection>
    SwaptionVolatilityMatrix::smileSectionImpl(Time optionTime,
                                               Time swapLength) const {
        // dummy strike
        Volatility atmVol = volatilityImpl(optionTime, swapLength, 0.05);
        return ext::shared_ptr<SmileSection>(new FlatSmileSection(
            optionTime, atmVol, dayCounter(), Null<Real>(), volatilityType(),
            shift(optionTime, swapLength, true)));
    }

}
