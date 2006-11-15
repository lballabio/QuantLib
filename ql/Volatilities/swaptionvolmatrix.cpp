/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#include <ql/Calendars/nullcalendar.hpp>
#include <ql/Utilities/dataformatters.hpp>
#include <ql/Volatilities/swaptionvolmatrix.hpp>
#include <ql/Volatilities/smilesection.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED
    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                    const std::vector<Period>& expiries,
                    const Calendar& calendar,
                    const BusinessDayConvention bdc,
                    const std::vector<Period>& tenors,
                    const std::vector<std::vector<Handle<Quote> > >& vols,
                    const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(0, calendar),
      swapTenors_(tenors), volatilities_(expiries.size(),tenors.size()),
      dayCounter_(dayCounter) {
        QL_REQUIRE(!vols.empty(), "empty vol matrix");
        QL_REQUIRE(expiries.size()==vols.size(),
            "mismatch between number of exercise dates ("
            << expiries.size() << ") and number of rows ("
            << vols.size() << ") in the vol matrix");
        Size i;
        for (i=0; i<expiries.size(); i++) {
            QL_REQUIRE(tenors.size()==vols[i].size(),
                "mismatch between number of tenors ("
                << tenors.size() << ") and number of columns ("
                << vols[i].size() << ") in the "
                << io::ordinal(i) << " row of the vol matrix");
            for (Size j=0; j<tenors.size(); j++) {
                volatilities_[i][j]=vols[i][j]->value();
                registerWith(vols[i][j]);
            }
        }

        optionTimes_.resize(expiries.size());
        optionDates_.resize(expiries.size());
        swapLengths_.resize(swapTenors_.size());
        for (i=0; i<expiries.size(); i++) {
            optionDates_[i] = calendar.advance(referenceDate(),
                                                 expiries[i],
                                                 bdc); // FIXME
            optionTimes_[i] = timeFromReference(optionDates_[i]);
        }

        Date startDate = optionDates_[0]; // as good as any
        for (i=0; i<swapTenors_.size(); i++) {
            Date endDate = startDate + swapTenors_[i];
            swapLengths_[i] = dayCounter_.yearFraction(startDate,endDate);
        }
        interpolation_ =
            BilinearInterpolation(swapLengths_.begin(),
                                  swapLengths_.end(),
                                  optionTimes_.begin(),
                                  optionTimes_.end(),
                                  volatilities_);
    }

    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                        const std::vector<Period>& expiries,
                        const Calendar& calendar,
                        const BusinessDayConvention bdc,
                        const std::vector<Period>& lengths,
                        const Matrix& vols,
                        const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(0, calendar),
      swapTenors_(lengths), volatilities_(vols),  dayCounter_(dayCounter) {
        QL_REQUIRE(expiries.size()==vols.rows(),
            "mismatch between number of expiries ("
            << expiries.size() << ") and number of rows ("
            << vols.rows() << ") in the vol matrix");
        QL_REQUIRE(lengths.size()==vols.columns(),
            "mismatch between number of tenors ("
            << lengths.size() << ") and number of rows ("
            << vols.columns() << ") in the vol matrix");

        optionTimes_.resize(expiries.size());
        optionDates_.resize(expiries.size());
        swapLengths_.resize(swapTenors_.size());
        Size i;
        for (i=0; i<expiries.size(); i++) {
            optionDates_[i] = calendar.advance(referenceDate(),
                                                 expiries[i],
                                                 bdc); // FIXME
            optionTimes_[i] = timeFromReference(optionDates_[i]);
        }

        Date startDate = optionDates_[0]; // as good as any
        for (i=0; i<swapTenors_.size(); i++) {
            Date endDate = startDate + swapTenors_[i];
            swapLengths_[i] = dayCounter_.yearFraction(startDate,endDate);
        }
        interpolation_ =
            BilinearInterpolation(swapLengths_.begin(),
                                  swapLengths_.end(),
                                  optionTimes_.begin(),
                                  optionTimes_.end(),
                                  volatilities_);
    }

    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                       const std::vector<Date>& dates,
                       const std::vector<Period>& lengths,
                       const Matrix& vols,
                       const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(0, NullCalendar()), // FIXME
      optionDates_(dates), swapTenors_(lengths), volatilities_(vols),
      dayCounter_(dayCounter) {
        QL_REQUIRE(dates.size()==vols.rows(),
            "mismatch between number of exercise dates ("
            << dates.size() << ") and number of rows ("
            << vols.rows() << ") in the vol matrix");
        QL_REQUIRE(lengths.size()==vols.columns(),
            "mismatch between number of tenors ("
            << lengths.size() << ") and number of rows ("
            << vols.columns() << ") in the vol matrix");
        optionTimes_.resize(optionDates_.size());
        swapLengths_.resize(swapTenors_.size());
        Size i;
        for (i=0; i<optionDates_.size(); i++) {
            optionTimes_[i] = timeFromReference(optionDates_[i]);
        }

        Date startDate = optionDates_[0]; // as good as any
        for (i=0; i<swapTenors_.size(); i++) {
            Date endDate = startDate + swapTenors_[i];
            swapLengths_[i] = dayCounter_.yearFraction(startDate,endDate);
        }
        interpolation_ =
            BilinearInterpolation(swapLengths_.begin(),
                                  swapLengths_.end(),
                                  optionTimes_.begin(),
                                  optionTimes_.end(),
                                  volatilities_);
    }
    #endif

    // floating reference date, floating market data
    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                    const Calendar& calendar,
                    const std::vector<Period>& optionTenors,
                    const std::vector<Period>& swapTenors,
                    const std::vector<std::vector<Handle<Quote> > >& vols,
                    const DayCounter& dayCounter,
                    const BusinessDayConvention bdc)
    : SwaptionVolatilityStructure(0, calendar),
      optionTenors_(optionTenors), swapTenors_(swapTenors), 
      volHandles_(vols),
      volatilities_(vols.size(), vols.front().size()),
      dayCounter_(dayCounter), bdc_(bdc) {
        checkInputs(optionTenors.size(), swapTenors.size(), 
                    volatilities_.rows(), volatilities_.columns());
        initializeOptionDatesAndTimes();
        registerWithMarketData();
        interpolation_ = BilinearInterpolation(
                                    swapLengths_.begin(), swapLengths_.end(),
                                    optionTimes_.begin(), optionTimes_.end(),
                                    volatilities_);
   }

    // fixed reference date, floating market data
    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                    const Date& referenceDate,
                    const Calendar& calendar,
                    const std::vector<Period>& optionTenors,
                    const std::vector<Period>& swapTenors,
                    const std::vector<std::vector<Handle<Quote> > >& vols,
                    const DayCounter& dayCounter,
                    const BusinessDayConvention bdc)
    : SwaptionVolatilityStructure(referenceDate, calendar), 
      optionTenors_(optionTenors), swapTenors_(swapTenors), 
      volHandles_(vols),
      volatilities_(vols.size(), vols.front().size()),
      dayCounter_(dayCounter), bdc_(bdc) {
        checkInputs(optionTenors.size(), swapTenors.size(), 
                    volatilities_.rows(), volatilities_.columns());
        initializeOptionDatesAndTimes();
        registerWithMarketData();
        interpolation_ = BilinearInterpolation(
                                    swapLengths_.begin(), swapLengths_.end(),
                                    optionTimes_.begin(), optionTimes_.end(),
                                    volatilities_);
    }


    // floating reference date, fixed market data
    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                        const Calendar& calendar,
                        const std::vector<Period>& optionTenors,
                        const std::vector<Period>& swapTenors,
                        const Matrix& vols,
                        const DayCounter& dayCounter,
                        const BusinessDayConvention bdc)
    : SwaptionVolatilityStructure(0, calendar),
      optionTenors_(optionTenors), swapTenors_(swapTenors),
      volHandles_(vols.rows(), vols.columns()),
      volatilities_(vols.rows(), vols.columns()),
      dayCounter_(dayCounter), bdc_(bdc) {

        checkInputs(optionTenors.size(), swapTenors.size(), 
                    vols.rows(), vols.columns());
        initializeOptionDatesAndTimes();

        // fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<vols.rows(); i++){
            for (Size j=0; j<vols.columns(); j++){
                volHandles_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(
                    new SimpleQuote(vols[i][j])));
            }
        }   
        registerWithMarketData();
        interpolation_ = BilinearInterpolation(
                                    swapLengths_.begin(), swapLengths_.end(),
                                    optionTimes_.begin(), optionTimes_.end(),
                                    volatilities_);
    }

    // fixed reference date, fixed market data
    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                        const Date& referenceDate,
                        const Calendar& calendar,
                        const std::vector<Period>& optionTenors,
                        const std::vector<Period>& swapTenors,
                        const Matrix& vols,
                        const DayCounter& dayCounter,
                        const BusinessDayConvention bdc)
    : SwaptionVolatilityStructure(referenceDate, calendar),
      optionTenors_(optionTenors), swapTenors_(swapTenors),
      volHandles_(vols.rows(), vols.columns()),
      volatilities_(vols.rows(), vols.columns()),
      dayCounter_(dayCounter), bdc_(bdc) {
        checkInputs(optionTenors.size(), swapTenors.size(), 
                    vols.rows(), vols.columns());
        initializeOptionDatesAndTimes();

        // fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<vols.rows(); i++){
            for (Size j=0; j<vols.columns(); j++){
                volHandles_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(
                    new SimpleQuote(vols[i][j])));
            }
        }   
        registerWithMarketData();
        interpolation_ = BilinearInterpolation(
                                    swapLengths_.begin(), swapLengths_.end(),
                                    optionTimes_.begin(), optionTimes_.end(),
                                    volatilities_);
    }

    // fixed reference date and fixed market data, option dates 
    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                    const Date& today,
                    const std::vector<Date>& optionDates,
                    const std::vector<Period>& swapTenors,
                    const Matrix& vols,
                    const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(today),
      optionDates_(optionDates), swapTenors_(swapTenors),
      volHandles_(vols.rows(), vols.columns()),
      volatilities_(vols.rows(), vols.columns()),
      dayCounter_(dayCounter) {
        checkInputs(optionDates.size(), swapTenors.size(),
                    vols.rows(), vols.columns());
        // initialize times only (no date)
        initializeTimes();

        // fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<vols.rows(); i++){
            for (Size j=0; j<vols.columns(); j++){
                volHandles_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(
                    new SimpleQuote(vols[i][j])));
            }
        }   
        registerWithMarketData();
        interpolation_ = BilinearInterpolation(
                                    swapLengths_.begin(), swapLengths_.end(),
                                    optionTimes_.begin(), optionTimes_.end(),
                                    volatilities_);
    }

    void SwaptionVolatilityMatrix::performCalculations() const {
        
        if (moving_) // check if date recalculation could be avoided
            initializeOptionDatesAndTimes(); 

        // we might use iterators here...
        for (Size i=0; i<volatilities_.rows(); ++i)
            for (Size j=0; j<volatilities_.columns(); ++j)
                volatilities_[i][j] = volHandles_[i][j]->value();
    }
  
    void SwaptionVolatilityMatrix::registerWithMarketData()
    {
        for (Size i=0; i<volHandles_.size(); ++i)
            for (Size j=0; j<volHandles_.front().size(); ++j)
                registerWith(volHandles_[i][j]);
    }

    boost::shared_ptr<SmileSectionInterface> SwaptionVolatilityMatrix::smileSection(
                                             Time start, Time length) const {

        // dummy strike
        const Volatility atmVol = volatility(start, length, 0.05);

        std::vector<Real> strikes, volatilities(2, atmVol);

        strikes.push_back(0.0);
        strikes.push_back(1.0);

        return boost::shared_ptr<SmileSectionInterface>(
                              new InterpolatedSmileSection(start, strikes, volatilities));
    }

    boost::shared_ptr<SmileSectionInterface>
    SwaptionVolatilityMatrix::smileSection(const Date& exerciseDate,
                                           const Period& length) const {

        // dummy strike
        const Volatility atmVol = volatility(exerciseDate, length, 0.05);

        std::vector<Real> strikes, volatilities(2, atmVol);

        strikes.push_back(0.0);
        strikes.push_back(1.0);

        return boost::shared_ptr<SmileSectionInterface>(new
            InterpolatedSmileSection(timeFromReference(exerciseDate),
                         strikes, volatilities));
    }
    
    void SwaptionVolatilityMatrix::checkInputs(
        Size optionsNb, Size SwapNb, Size volRows, Size volsColumns) const {
        QL_REQUIRE(optionsNb==volRows,
            "mismatch between number of exercise dates ("
            << optionsNb << ") and number of rows ("
            << volRows << ") in the vol matrix");
        QL_REQUIRE(SwapNb==volsColumns,
            "mismatch between number of tenors ("
            << SwapNb << ") and number of rows ("
            << volsColumns << ") in the vol matrix");
        }

    void SwaptionVolatilityMatrix::initializeOptionDatesAndTimes() const {
        optionDates_.resize(optionTenors_.size());
        for (Size i=0; i<optionTenors_.size(); i++)
            optionDates_[i] = calendar().advance(referenceDate(),
                                                 optionTenors_[i],
                                                 bdc_); // FIXME
        initializeTimes();
    }

    void SwaptionVolatilityMatrix::initializeTimes() const {
        // Option times initialization
        optionTimes_.resize(optionDates_.size());
        for (Size i=0; i<optionTimes_.size(); i++)
            optionTimes_[i] = timeFromReference(optionDates_[i]);
        // Swap times initialization
        swapLengths_.resize(swapTenors_.size());
        Date startDate = optionDates_.front(); // as good as any
        for (Size i=0; i<swapLengths_.size(); i++) {
            Date endDate = startDate + swapTenors_[i];
            swapLengths_[i] = dayCounter_.yearFraction(startDate, endDate);
        }
    }

    std::pair<Time,Time> SwaptionVolatilityMatrix::convertDates(
                      const Date& exerciseDate, const Period& length) const {
        Time exerciseTime = timeFromReference(exerciseDate);
        Date startDate = optionDates_[0]; // for consistency
        Date endDate = startDate + length;
        Time timeLength = dayCounter_.yearFraction(startDate,endDate);
        return std::make_pair(exerciseTime,timeLength);
    }

}
