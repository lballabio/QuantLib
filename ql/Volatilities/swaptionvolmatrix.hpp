
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file swaptionvolmatrix.hpp
    \brief Swaption at-the-money volatility matrix
*/

#ifndef quantlib_swaption_volatility_matrix_h
#define quantlib_swaption_volatility_matrix_h

#include <ql/swaptionvolstructure.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Math/interpolationtraits.hpp>
#include <vector>

namespace QuantLib {

    //! At-the-money swaption-volatility matrix
    /*! This class provides the at-the-money volatility for a given
        swaption by interpolating a volatility matrix whose elements
        are the market volatilities of a set of swaption with given
        exercise date and length.

        \todo either add correct copy behavior or inhibit copy. Right
              now, a copied instance would end up with its own copy of
              the exercise date and length vector but an interpolation
              pointing to the original ones.
    */
    class SwaptionVolatilityMatrix : public SwaptionVolatilityStructure {
      public:
        SwaptionVolatilityMatrix(const Date& referenceDate,
                                 const std::vector<Date>& exerciseDates,
                                 const std::vector<Period>& lengths,
                                 const Matrix& volatilities,
                                 const DayCounter& dayCounter);
        // inspectors
        DayCounter dayCounter() const { return dayCounter_; }
        const std::vector<Date>& exerciseDates() const;
        const std::vector<Period>& lengths() const;
      private:
        DayCounter dayCounter_;
        std::vector<Date> exerciseDates_;
        std::vector<Time> exerciseTimes_;
        std::vector<Period> lengths_;
        std::vector<Time> timeLengths_;
        Matrix volatilities_;
        Interpolation2D interpolation_;
        Volatility volatilityImpl(Time start, Time length, Rate strike) const;
        std::pair<Time,Time> convertDates(const Date& start,
                                          const Period& length) const;
    };


    // inline definitions

    inline SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                       const Date& today, const std::vector<Date>& dates,
                       const std::vector<Period>& lengths, const Matrix& vols,
                       const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(today),
      dayCounter_(dayCounter), exerciseDates_(dates),
      lengths_(lengths), volatilities_(vols) {
        exerciseTimes_.resize(exerciseDates_.size());
        timeLengths_.resize(lengths_.size());
        Size i;
        for (i=0; i<exerciseDates_.size(); i++) {
            exerciseTimes_[i] = timeFromReference(exerciseDates_[i]);
        }
        for (i=0; i<lengths_.size(); i++) {
            Date startDate = exerciseDates_[0]; // as good as any
            Date endDate = startDate + lengths_[i];
            timeLengths_[i] = dayCounter_.yearFraction(startDate,endDate);
        }
        interpolation_ =
            Linear::make_interpolation(exerciseTimes_.begin(),
                                       exerciseTimes_.end(),
                                       timeLengths_.begin(),
                                       timeLengths_.end(),
                                       volatilities_);
    }

    inline const std::vector<Date>&
    SwaptionVolatilityMatrix::exerciseDates() const {
        return exerciseDates_;
    }

    inline const std::vector<Period>&
    SwaptionVolatilityMatrix::lengths() const {
        return lengths_;
    }

    inline Volatility SwaptionVolatilityMatrix::volatilityImpl(
                                        Time start, Time length, Rate) const {
        return interpolation_(start,length,false);
    }

    inline std::pair<Time,Time> SwaptionVolatilityMatrix::convertDates(
                              const Date& start, const Period& length) const {
        Time exerciseTime = timeFromReference(start);
        Date startDate = exerciseDates_[0]; // for consistency
        Date endDate = startDate + length;
        Time timeLength = dayCounter_.yearFraction(startDate,endDate);
        return std::make_pair(exerciseTime,timeLength);
    }

}


#endif

