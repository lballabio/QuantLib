/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
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

/*! \file swaptionvolmatrix.hpp
    \brief Swaption at-the-money volatility matrix
*/

#ifndef quantlib_swaption_volatility_matrix_h
#define quantlib_swaption_volatility_matrix_h

#include <ql/swaptionvolstructure.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Math/bilinearinterpolation.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

namespace QuantLib {

    //! At-the-money swaption-volatility matrix
    /*! This class provides the at-the-money volatility for a given
        swaption by interpolating a volatility matrix whose elements
        are the market volatilities of a set of swaption with given
        exercise date and length.

        The volatility matrix <tt>M</tt> must be defined so that:
        - the number of rows equals the number of exercise dates;
        - the number of columns equals the number of swap tenors;
        - <tt>M[i][j]</tt> contains the volatility corresponding
          to the <tt>i</tt>-th exercise and <tt>j</tt>-th tenor.
    */
    class SwaptionVolatilityMatrix : public SwaptionVolatilityStructure,
                                     private boost::noncopyable {
      public:
        SwaptionVolatilityMatrix(const Date& referenceDate,
                                 const std::vector<Date>& exerciseDates,
                                 const std::vector<Period>& lengths,
                                 const Matrix& volatilities,
                                 const DayCounter& dayCounter);
        SwaptionVolatilityMatrix(const std::vector<Date>& exerciseDates,
                                 const std::vector<Period>& tenors,
                                 const Matrix& volatilities,
                                 const DayCounter& dayCounter);
        SwaptionVolatilityMatrix(const std::vector<Period>& expiries,
                                 const Calendar& calendar,
                                 const BusinessDayConvention bdc,
                                 const std::vector<Period>& tenors,
                                 const Matrix& volatilities,
                                 const DayCounter& dayCounter);
        // inspectors
        const std::vector<Date>& exerciseDates() const;
        const std::vector<Time>& exerciseTimes() const;
        const std::vector<Period>& lengths() const;
        const std::vector<Time>& timeLengths() const;

        //! \name RateHelper interface
        //@{
        DayCounter dayCounter() const { return dayCounter_; }
        //@}
        //! \name SwaptionVolatilityStructure interface
        //@{
        Date maxStartDate() const;
        Time maxStartTime() const;
        Period maxLength() const;
        Time maxTimeLength() const;
        Rate minStrike() const;
        Rate maxStrike() const;
        //! return trivial smile section
        virtual VarianceSmileSection smileSection(Time start, Time length) const;
        //! implements the conversion between dates and times
        std::pair<Time,Time> convertDates(const Date& exerciseDate,
                                          const Period& length) const;
        //@}
        //! \name Other inspectors
        //@{
        //! returns the lower indexes of sourrounding volatility matrix corners
        std::pair<Size,Size> locate(const Date& exerciseDate,
                                    const Period& length) const {
            std::pair<Time,Time> times = convertDates(exerciseDate,length);
            return locate(times.first, times.second);
        }
        //! returns the lower indexes of sourrounding volatility matrix corners
        std::pair<Size,Size> locate(Time exerciseTime,
                                    Time length) const {
            return std::make_pair(interpolation_.locateY(exerciseTime),
                                  interpolation_.locateX(length));
        }
        //@}
    private:
        DayCounter dayCounter_;
        std::vector<Date> exerciseDates_;
        std::vector<Time> exerciseTimes_;
        std::vector<Period> lengths_;
        std::vector<Time> timeLengths_;
        Matrix volatilities_;
        Interpolation2D interpolation_;
        Volatility volatilityImpl(Time exerciseTime,
                                  Time length,
                                  Rate strike) const;
    };


    // inline definitions

    inline const std::vector<Date>&
    SwaptionVolatilityMatrix::exerciseDates() const {
        return exerciseDates_;
    }

    inline const std::vector<Time>&
    SwaptionVolatilityMatrix::exerciseTimes() const {
        return exerciseTimes_;
    }

     inline const std::vector<Period>&
     SwaptionVolatilityMatrix::lengths() const {
         return lengths_;
     }
 
    inline const std::vector<Time>&
    SwaptionVolatilityMatrix::timeLengths() const {
        return timeLengths_;
    }

    inline Date SwaptionVolatilityMatrix::maxStartDate() const {
        return exerciseDates_.back();
    }

    inline Time SwaptionVolatilityMatrix::maxStartTime() const {
        return exerciseTimes_.back();
    }

    inline Period SwaptionVolatilityMatrix::maxLength() const {
        return lengths_.back();
    }

    inline Time SwaptionVolatilityMatrix::maxTimeLength() const {
        return timeLengths_.back();
    }

    inline Rate SwaptionVolatilityMatrix::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Rate SwaptionVolatilityMatrix::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline Volatility SwaptionVolatilityMatrix::volatilityImpl(
                                        Time exerciseTime, Time length, Rate) const {
        return interpolation_(length,exerciseTime,true);
    }

}

#endif
