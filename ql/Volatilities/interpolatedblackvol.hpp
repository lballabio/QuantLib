
/*
 Copyright (C) 2002 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file interpolatedblackvol.hpp
    \brief Black volatility term structure

  \fullpath
    ql/Volatilities/%interpolatedblackvol.hpp
*/

// $Id$

#ifndef quantlib_interpolatedblackvol_hpp
#define quantlib_interpolatedblackvol_hpp

#include <ql/Math/matrix.hpp>
#include <ql/voltermstructure.hpp>
#include <vector>

namespace QuantLib {

    namespace VolTermStructures {

        //! Black volatility term structure interpolation based
        /*! This class calculates interpolated Black volatilies based on
            a matrix of Black volatilities observed in the market

            The interpolation is performed on the variance.
        */
        template<class Interpolator2D>
        class InterpolatedBlackVolStructure : public VolTermStructure {
          public:
            InterpolatedBlackVolStructure(
                                const Date& referenceDate,
                                const std::vector<Date>& dates,
                                const std::vector<double>& strikes,
                                const QuantLib::Math::Matrix& blackVolMatrix,
                                const DayCounter& dayCounter,
                                const std::string& underlying = "");
            double blackVol(const Date& maturity,
                            double strike,
                            bool extrapolate = false) const;
            double blackVol(Time maturity,
                            double strike,
                            bool extrapolate = false) const;
            Date referenceDate() const { return referenceDate_; }
            DayCounter dayCounter() const { return dayCounter_; }
            Date maxDate() const { return dates_.back(); }
            Time maxTime() const { return times_.back(); }
            std::string underlying() const { return underlying_; }
          private:
              Date referenceDate_;
              std::vector<Date> dates_;
              DayCounter dayCounter_;
              std::string underlying_;
              std::vector<Time> times_;
              Handle < Interpolator2D> varianceSurface_;
        };


        template<class Interpolator2D>
        InterpolatedBlackVolStructure<Interpolator2D
                                              >::InterpolatedBlackVolStructure(
            const Date& referenceDate,
            const std::vector<Date>& dates,
            const std::vector<double>& moneyStrikes,
            const QuantLib::Math::Matrix& blackVolMatrix,
            const DayCounter& dayCounter,
            const std::string& underlying)
        : referenceDate_(referenceDate), dates_(dates),
          dayCounter_(dayCounter), underlying_(underlying) {

            QL_REQUIRE(dates.size()==blackVolMatrix.columns(),
                "mismatch between date vector and vol matrix colums");
            QL_REQUIRE(moneyStrikes.size()==blackVolMatrix.rows(),
                "mismatch between money-strike vector and vol matrix rows");

            Size j, i;
            QuantLib::Math::Matrix variances(i,j);
            times_ = std::vector<Time>(j);
            times_[0] = dayCounter_.yearFraction(referenceDate, dates[0]);
            QL_REQUIRE(times_[0]>=0.0,
                "minimum date previous than reference date");
            for (i=0; i<blackVolMatrix.rows(); i++) {
                variances[i][0] = times_[0] *
                    blackVolMatrix[i][0]*blackVolMatrix[i][0];
            }

            for (j=1; j<blackVolMatrix.columns(); j++) {
                times_[j] = dayCounter_.yearFraction(referenceDate, dates[j]);
                QL_REQUIRE(times_[j]>=times_[j-1],
                    "dates not sorted!");
                for (i=0; i<blackVolMatrix.rows(); i++) {
                    variances[i][j] = times_[j] *
                        blackVolMatrix[i][j]*blackVolMatrix[i][j];
                }
            }
            varianceSurface_ = Handle<Interpolator2D> (new
                Interpolator2D(times_.begin(), times_.end(),
                moneyStrikes.begin(), moneyStrikes.end(), variances));
        }


        template<class Interpolator2D>
        double InterpolatedBlackVolStructure<Interpolator2D>::blackVol(
            const Date& evaluationDate,double strike,bool extrapolate) const {

                return blackVol(dayCounter_.yearFraction(referenceDate_,
                    evaluationDate), strike, extrapolate);
        }

        template<class Interpolator2D>
        double InterpolatedBlackVolStructure<Interpolator2D>::blackVol(
            Time evaluationTime, double strike, bool extrapolate) const {

            QL_REQUIRE(evaluationTime>=0.0,
                "InterpolatedBlackVolStructure<Interpolator2D>::blackVol : "
                "negative time not allowed");

            double variance = varianceSurface_->(evaluationTime, strike,
                extrapolate);
            
            return QL_SQRT(variance/evaluationTime);
        }

    }

}


#endif
