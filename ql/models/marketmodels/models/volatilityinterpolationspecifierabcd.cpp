/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

Copyright (C) 2007 Mark Joshi

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
/*!
implementation specifies how to decide volatility structure for additional
synthetic rates which are interleaved

here we work with abcd curves and interpolate the a, b, c and d

*/

#include <ql/models/marketmodels/models/volatilityinterpolationspecifierabcd.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <memory>
#include <vector>

namespace QuantLib
{

    VolatilityInterpolationSpecifierabcd::VolatilityInterpolationSpecifierabcd(Size period,
        Size offset,
        const std::vector< PiecewiseConstantAbcdVariance>& originalVariances, // these should be associated with the long rates
        const std::vector<Time>& timesForSmallRates, // these should be associated with the shorter rates
        Real lastCapletVol
        )
        :
    period_(period),
        offset_(offset),
        interpolatedVariances_(timesForSmallRates.size()-1),
        originalVariances_(originalVariances.size()),
        originalABCDVariances_(originalVariances),
        originalABCDVariancesScaled_(originalVariances),
        lastCapletVol_(lastCapletVol),
        timesForSmallRates_(timesForSmallRates),
        scalingFactors_(originalVariances.size(),1.0),
        noBigRates_(originalVariances.size()),
        noSmallRates_(timesForSmallRates.size()-1)
    {
        QL_REQUIRE( (noSmallRates_ - offset) /period == noBigRates_, "size mismatch in VolatilityInterpolationSpecifierabcd");

        for (Size i=0; i < noBigRates_; ++i)
            for (Size j=0; j < originalVariances[i].rateTimes().size(); ++j)
                QL_REQUIRE( originalVariances[i].rateTimes()[j] == timesForSmallRates[offset+j*period],"rate times in variances passed in don't match small times in VolatilityInterpolationSpecifierabcd");

        if (lastCapletVol_ == 0.0)
            lastCapletVol_ =  originalVariances[noBigRates_-1].totalVolatility(noBigRates_-1);

        // change type of array to PiecewiseConstantVariance for client, from PiecewiseConstantAbcdVariance
        for (Size i=0; i < noBigRates_; ++i)
            originalVariances_[i] = std::shared_ptr<PiecewiseConstantVariance>(new PiecewiseConstantAbcdVariance(originalVariances[i]));

        recompute();

    }

    void VolatilityInterpolationSpecifierabcd::setScalingFactors(const std::vector<Real>& scales)
    {
        QL_REQUIRE(scalingFactors_.size() == scales.size(), "inappropriate number of scales passed in to VolatilityInterpolationSpecifierabcd::setScalingFactors ");
        scalingFactors_= scales;
        recompute();
    }

    void VolatilityInterpolationSpecifierabcd::setLastCapletVol(Real vol)
    {
        lastCapletVol_ = vol;
        recompute();
    }


    const std::vector<std::shared_ptr<PiecewiseConstantVariance> >& VolatilityInterpolationSpecifierabcd::interpolatedVariances() const
    {
        return interpolatedVariances_;
    }

    const std::vector<std::shared_ptr<PiecewiseConstantVariance> >& VolatilityInterpolationSpecifierabcd::originalVariances() const
    {
        return originalVariances_;
    }

    Size VolatilityInterpolationSpecifierabcd::getPeriod() const
    {
        return period_;
    }

    Size VolatilityInterpolationSpecifierabcd::getOffset() const
    {
        return offset_;
    }

    Size VolatilityInterpolationSpecifierabcd::getNoBigRates() const
    {
        return noBigRates_;
    }
    Size VolatilityInterpolationSpecifierabcd::getNoSmallRates() const
    {
        return noSmallRates_;
    }


    void VolatilityInterpolationSpecifierabcd::recompute()
    {
        //     PiecewiseConstantAbcdVariance(Real a, Real b, Real c, Real d,
        //                                    Size resetIndex,
        //                                  const std::vector<Time>& rateTimes);


        for (Size i=0; i < noBigRates_; ++i)
        {
            Real a,b,c,d;
            originalABCDVariances_[i].getABCD(a,b,c,d);
            a*=scalingFactors_[i];
            b*=scalingFactors_[i];
            // c is not scaled
            d*=scalingFactors_[i];

            originalABCDVariancesScaled_[i] = PiecewiseConstantAbcdVariance(a,b,c,d, i, originalABCDVariances_[i].rateTimes());

        }

        // three cases:
        //before offset,
        // between offset and last big rate,
        // and after last big rate

        // before offset

        {
            Real a,b,c,d;
            originalABCDVariancesScaled_[0].getABCD(a,b,c,d);

            for (Size i=0; i < offset_; ++i)
                interpolatedVariances_[i] = std::shared_ptr<PiecewiseConstantVariance>(
                new PiecewiseConstantAbcdVariance(a,b,c,d,i,timesForSmallRates_));
        }


        // in between rates

        for (Size j=0; j < noBigRates_-1; ++j)
        {
            Real a,b,c,d;
            Real a0,b0,c0,d0;
            Real a1,b1,c1,d1;
            originalABCDVariancesScaled_[j].getABCD(a0,b0,c0,d0);
            originalABCDVariancesScaled_[j+1].getABCD(a1,b1,c1,d1);
            a= 0.5*(a0+a1);
            b= 0.5*(b0+b1);
            c= 0.5*(c0+c1);
            d= 0.5*(d0+d1);

            for (Size i=0; i < period_; ++i)
                interpolatedVariances_[i+j*period_+offset_] =  std::shared_ptr<PiecewiseConstantVariance>(
                new PiecewiseConstantAbcdVariance(a,b,c,d,i+j*period_,timesForSmallRates_));

        }


       {
            Real a,b,c,d;
            originalABCDVariancesScaled_[noBigRates_-1].getABCD(a,b,c,d);

            for (Size i=offset_+(noBigRates_-1)*period_; i < noSmallRates_; ++i)
                interpolatedVariances_[i] = std::shared_ptr<PiecewiseConstantVariance>(
                                                                         new PiecewiseConstantAbcdVariance(a,b,c,d,i,timesForSmallRates_));

            // very last rate is special as we must match the caplet vol
             Real vol = interpolatedVariances_[noSmallRates_-1]->totalVolatility(noSmallRates_-1);

             Real scale = lastCapletVol_/vol;
             a*=scale;
             b*=scale;
             d*=scale;
             interpolatedVariances_[noSmallRates_-1] = std::shared_ptr<PiecewiseConstantVariance>(
                                                                         new PiecewiseConstantAbcdVariance(a,b,c,d,noSmallRates_-1,timesForSmallRates_));

       }
    }

}
