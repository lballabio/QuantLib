/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

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


#include <ql/Volatilities/swaptionvolcube.hpp>
#include <ql/Math/sabrinterpolation.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/cubicspline.hpp>

namespace QuantLib {

    SwaptionVolatilityCube::SwaptionVolatilityCube(
        const Handle<SwaptionVolatilityStructure>& atmVolStructure,
        const std::vector<Period>& expiries,
        const std::vector<Period>& lengths,
        const std::vector<Spread>& strikeSpreads,
        const Matrix& volSpreads)
    : atmVolStructure_(atmVolStructure),
      exerciseDates_(expiries.size()), exerciseTimes_(expiries.size()),
      lengths_(lengths), timeLengths_(lengths.size()), 
      nStrikes_(strikeSpreads.size()), strikeSpreads_(strikeSpreads),
      volSpreads_(nStrikes_, Matrix(expiries.size(), lengths.size(), 0.0)),
      volSpreadsInterpolator_(nStrikes_),
      localStrikes_(nStrikes_), localSmile_(nStrikes_)
    {
        Size i, nExercise = expiries.size();
        exerciseDates_[0] = calendar().advance(referenceDate(),
                                               expiries[0],
                                               Unadjusted); //FIXME
        exerciseTimes_[0] = timeFromReference(exerciseDates_[0]);
        QL_REQUIRE(0.0<exerciseTimes_[0],
            "first exercise time is negative");
        for (i=1; i<nExercise; i++) {
            exerciseDates_[i] = calendar().advance(referenceDate(),
                                                   expiries[i],
                                                   Unadjusted); //FIXME
            exerciseTimes_[i] = timeFromReference(exerciseDates_[i]);
            QL_REQUIRE(exerciseTimes_[i-1]<exerciseTimes_[i],
                "non increasing exercise times");
        }

        Size nlengths = lengths_.size();
        Date startDate = exerciseDates_[0]; // as good as any
        Date endDate = startDate + lengths_[0];
        timeLengths_[0] = dayCounter().yearFraction(startDate,endDate);
        QL_REQUIRE(0.0<timeLengths_[0],
            "first time length is negative");
        for (i=1; i<nlengths; i++) {
            Date endDate = startDate + lengths_[i];
            timeLengths_[i] = dayCounter().yearFraction(startDate,endDate);
            QL_REQUIRE(timeLengths_[i-1]<timeLengths_[i],
                "non increasing time length");
        }

        QL_REQUIRE(nStrikes_>1, "too few strikes (" << nStrikes_ << ")");
        for (i=1; i<nStrikes_; i++) {
            QL_REQUIRE(strikeSpreads_[i-1]<strikeSpreads_[i],
                "non increasing strike spreads");
        }


        QL_REQUIRE(nStrikes_==volSpreads.columns(),
            "nStrikes_!=volSpreads.columns()");
        QL_REQUIRE(nExercise*nlengths==volSpreads.rows(), 
            "nExercise*nlengths!=volSpreads.rows()");
        for (i=0; i<nStrikes_; i++) {


            Matrix temp(nExercise, nlengths);
            for (Size j=0; j<nExercise; j++) {
                for (Size k=0; k<nlengths; k++) {
                    temp[j][k]=volSpreads[j*nlengths+k][i];
                }
            }

            volSpreads_[i] = temp;
            volSpreadsInterpolator_[i] = BilinearInterpolation(
                timeLengths_.begin(), timeLengths_.end(),
                exerciseTimes_.begin(), exerciseTimes_.end(),
                volSpreads_[i]);
        }

        //registerWith(atmVolMatrix_);
    }


    boost::shared_ptr<Interpolation>
    SwaptionVolatilityCube::smile(Time start, Time length) const
    {
        Rate atmForward = 0.05; //FIXME
        Volatility atmVol = atmVolStructure_->volatility(start, length, atmForward);
        for (Size i=0; i<nStrikes_; i++) {
            localStrikes_[i] = atmForward + strikeSpreads_[i];
            localSmile_[i]   = atmVol     + volSpreadsInterpolator_[i](length, start, true);
        }
        return boost::shared_ptr<Interpolation>(new
            //SABRInterpolation(localStrikes_.begin(), localStrikes_.end(), localSmile_.begin(), start, atmForward, Null<Real>(), Null<Real>(), Null<Real>(), Null<Real>())
            LinearInterpolation(localStrikes_.begin(), localStrikes_.end(), localSmile_.begin())
            //NaturalCubicSpline(localStrikes_.begin(), localStrikes_.end(), localSmile_.begin())
            );
    }

    Volatility SwaptionVolatilityCube::volatilityImpl(Time start,
                                                      Time length,
                                                      Rate strike) const {
        return smile(start, length)->operator()(strike);
    }

}
