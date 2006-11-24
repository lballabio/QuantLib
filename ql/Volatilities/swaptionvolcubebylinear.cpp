/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni

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


#include <ql/Volatilities/swaptionvolcubebylinear.hpp>
#include <ql/Math/bilinearinterpolation.hpp>

namespace QuantLib {

    SwaptionVolatilityCubeByLinear::SwaptionVolatilityCubeByLinear(
        const Handle<SwaptionVolatilityStructure>& atmVolStructure,
        const std::vector<Period>& expiries,
        const std::vector<Period>& lengths,
        const std::vector<Spread>& strikeSpreads,
        const std::vector<std::vector<Handle<Quote> > >& volSpreads,
        const boost::shared_ptr<SwapIndex>& swapIndexBase,
        bool vegaWeightedSmileFit)
    : SwaptionVolatilityCube(atmVolStructure, expiries, lengths,
                             strikeSpreads, volSpreads, swapIndexBase,
                             vegaWeightedSmileFit),
      volSpreadsInterpolator_(nStrikes_),
      volSpreadsMatrix_(nStrikes_, Matrix(expiries.size(), lengths.size(), 0.0)) {
    }

    void SwaptionVolatilityCubeByLinear::performCalculations() const{
        //! set volSpreadsMatrix_ by volSpreads_ quotes
        for (Size i=0; i<nStrikes_; i++) 
            for (Size j=0; j<nExercise_; j++)
                for (Size k=0; k<nlengths_; k++) {
                    volSpreadsMatrix_[i][j][k] =
                        volSpreads_[j*nlengths_+k][i]->value();
                }
        //! create volSpreadsInterpolator_ 
        for (Size i=0; i<nStrikes_; i++) {
            volSpreadsInterpolator_[i] = BilinearInterpolation(
                timeLengths_.begin(), timeLengths_.end(),
                exerciseTimes_.begin(), exerciseTimes_.end(),
                volSpreadsMatrix_[i]);
            volSpreadsInterpolator_[i].enableExtrapolation();
        }
    }

    boost::shared_ptr<SmileSectionInterface>
    SwaptionVolatilityCubeByLinear::smileSection(Time start,
                                                 Time length) const {

        Date exerciseDate = Date(static_cast<BigInteger>(
            exerciseInterpolator_(start)));
        Rounding rounder(0);
        Period swapTenor(static_cast<Integer>(rounder(length/12.0)), Months);
        return smileSection(exerciseDate, swapTenor);
    }

    boost::shared_ptr<SmileSectionInterface>
    SwaptionVolatilityCubeByLinear::smileSection(const Date& exerciseDate,
                                                 const Period& length) const {
        calculate();
        const Rate atmForward = atmStrike(exerciseDate, length);
        const Volatility atmVol = atmVol_->volatility(exerciseDate, length,
                                                      atmForward);
        std::vector<Real> strikes, volatilities;
        const std::pair<Time, Time> p = convertDates(exerciseDate, length);
        for (Size i=0; i<nStrikes_; i++) {
            strikes.push_back(atmForward + strikeSpreads_[i]);
            volatilities.push_back(
                      atmVol + volSpreadsInterpolator_[i](p.second, p.first));
        }
        return boost::shared_ptr<SmileSectionInterface>(new
            InterpolatedSmileSection(p.first, strikes, volatilities));
    }
}
