
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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
/*! \file blackvariancesurface.hpp
    \brief Black volatility surface modelled as variance surface

  \fullpath
    ql/Volatilities/%blackvariancesurface.hpp
*/

// $Id$

#ifndef quantlib_blackvariancesurface_hpp
#define quantlib_blackvariancesurface_hpp

#include <ql/Math/matrix.hpp>
#include <ql/voltermstructure.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    namespace VolTermStructures {

        //! Black volatility surface modelled as variance surface
        /*! This class calculates time/strike dependant Black volatilities
            using as input a matrix of Black volatilities
            observed in the market.

            The calculation is performed interpolating on the variance surface.
        */
        template<class Interpolator2D>
        class BlackVarianceSurface : public BlackVarianceTermStructure,
                                     public Patterns::Observer {
          public:
            enum Extrapolation { ConstantExtrapolation,
                                 InterpolatorDefaultExtrapolation };
            BlackVarianceSurface(const Date& referenceDate,
                                 const std::vector<Date>& dates,
                                 const std::vector<double>& strikes,
                                 const QuantLib::Math::Matrix& blackVolMatrix,
                                 Extrapolation lowerExtrapolation =
                                     InterpolatorDefaultExtrapolation,
                                 Extrapolation upperExtrapolation =
                                     InterpolatorDefaultExtrapolation,
                                 const DayCounter& dayCounter =
                                     DayCounters::Actual365());
            Date referenceDate() const { return referenceDate_; }
            DayCounter dayCounter() const { return dayCounter_; }
            Date maxDate() const { return maxDate_; }
            // Observer interface
            void update();
          protected:
            virtual double blackVarianceImpl(Time t,
                                             double strike,
                                             bool extrapolate = false) const;
          private:
            Date referenceDate_;
            DayCounter dayCounter_;
            Date maxDate_;
            std::vector<double> strikes_;
            std::vector<Time> times_;
            QuantLib::Math::Matrix variances_;
            Handle < Interpolator2D> varianceSurface_;
            Extrapolation lowerExtrapolation_, upperExtrapolation_;
        };


        template<class Interpolator2D>
        BlackVarianceSurface<Interpolator2D>::BlackVarianceSurface(
            const Date& referenceDate,
            const std::vector<Date>& dates,
            const std::vector<double>& strikes,
            const QuantLib::Math::Matrix& blackVolMatrix,
            BlackVarianceSurface<Interpolator2D>::Extrapolation lowerEx,
            BlackVarianceSurface<Interpolator2D>::Extrapolation upperEx,
            const DayCounter& dayCounter)
        : referenceDate_(referenceDate), dayCounter_(dayCounter),
          maxDate_(dates.back()), strikes_(strikes),
          lowerExtrapolation_(lowerEx), upperExtrapolation_(upperEx) {

            QL_REQUIRE(dates.size()==blackVolMatrix.columns(),
                "BlackVarianceSurface::BlackVarianceSurface : "
                "mismatch between date vector and vol matrix colums");
            QL_REQUIRE(strikes_.size()==blackVolMatrix.rows(),
                "BlackVarianceSurface::BlackVarianceSurface : "
                "mismatch between money-strike vector and vol matrix rows");

            QL_REQUIRE(dates[0]>=referenceDate,
                "BlackVarianceSurface::BlackVarianceSurface : "
                "cannot have dates[0]<=referenceDate");

            variances_ = QuantLib::Math::Matrix(strikes_.size(), dates.size());
            times_ = std::vector<Time>(dates.size());
            Size j, i;
            for (j=0; j<blackVolMatrix.columns(); j++) {
                times_[j] = dayCounter_.yearFraction(referenceDate, dates[j]);
                QL_REQUIRE(j==0 || times_[j]>times_[j-1],
                    "BlackVarianceSurface::BlackVarianceSurface : "
                    "dates must be sorted unique!");
                for (i=0; i<blackVolMatrix.rows(); i++) {
                    variances_[i][j] = times_[j] *
                        blackVolMatrix[i][j]*blackVolMatrix[i][j];
                    if (j==0) {
                        QL_REQUIRE(variances_[i][0]>0.0 || times_[0]==0.0,
                            "BlackVarianceCurve::BlackVarianceCurve : "
                            "variance must be positive");
                    } else {
                        QL_REQUIRE(variances_[i][j]>=variances_[i][j-1],
                            "BlackVarianceCurve::BlackVarianceCurve : "
                            "variance must be non-decreasing");
                    }
                }
            }
            varianceSurface_ = Handle<Interpolator2D> (new
                Interpolator2D(times_.begin(), times_.end(),
                               strikes_.begin(), strikes_.end(),
                               variances_));
        }


        template<class Interpolator2D>
        void BlackVarianceSurface<Interpolator2D>::update() {
            notifyObservers();
        }

        template<class Interpolator2D>
        double BlackVarianceSurface<Interpolator2D>::
            blackVarianceImpl(Time t, double strike, bool extrapolate) const {

            // it doesn't check if extrapolation is performed/allowed
            if (t==0.0) return 0.0;

            // enforce constant extrapolation when required
            if (strike < strikes_.front() && strike < strikes_.back()
                && extrapolate
                && lowerExtrapolation_ == ConstantExtrapolation)
                strike = strikes_.front();
            if (strike > strikes_.back() && strike > strikes_.front()
                && extrapolate
                && upperExtrapolation_ == ConstantExtrapolation)
                strike = strikes_.back();

            QL_REQUIRE(t>=0.0,
                "BlackVarianceSurface::blackVarianceImpl : "
                "negative time (" + DoubleFormatter::toString(t) +
                ") not allowed");
            if (t<=times_[0])
                return (*varianceSurface_)(times_[0], strike, extrapolate)*
                    t/times_[0];
            else if (t<=times_.back())
                return (*varianceSurface_)(t, strike, extrapolate);
            else // t>times_.back() || extrapolate
                QL_REQUIRE(extrapolate,
                    "ConstantVol::blackVolImpl : "
                    "time (" + DoubleFormatter::toString(t) +
                    ") greater than max time (" +
                    DoubleFormatter::toString(times_.back()) +
                    ")");
                return (*varianceSurface_)(times_.back(), strike, extrapolate)*
                    t/times_.back();
        }

    }

}


#endif
