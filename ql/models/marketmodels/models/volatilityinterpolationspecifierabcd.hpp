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

#ifndef volatility_interpolation_specifier_abcd_hpp
#define volatility_interpolation_specifier_abcd_hpp

#include <ql/models/marketmodels/models/volatilityinterpolationspecifier.hpp>
#include <ql/models/marketmodels/models/piecewiseconstantabcdvariance.hpp>
#include <ql/types.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>


namespace QuantLib
{
    class VolatilityInterpolationSpecifierabcd : public VolatilityInterpolationSpecifier
    {
    public:
        VolatilityInterpolationSpecifierabcd(Size period,
                                                                   Size offset,
                                                                   const std::vector< PiecewiseConstantAbcdVariance>& originalVariances, // these should be associated with the long rates
                                                                   const std::vector<Time>& timesForSmallRates, // these should be associated with the shorter rates
                                                                   Real lastCapletVol=0.0
                                                                   );

        virtual ~VolatilityInterpolationSpecifierabcd();
        virtual void setScalingFactors(const std::vector<Real>& scales);
        virtual void setLastCapletVol(Real vol);


        virtual const std::vector<boost::shared_ptr<PiecewiseConstantVariance> >& interpolatedVariances() const;
        virtual const std::vector<boost::shared_ptr<PiecewiseConstantVariance> >& originalVariances() const;

        virtual Size getPeriod() const;
        virtual Size getOffset() const;
        virtual Size getNoBigRates() const;
        virtual Size getNoSmallRates() const;

    private:
        Size period_;
        Size offset_;

         std::vector<boost::shared_ptr<PiecewiseConstantVariance> > interpolatedVariances_;
         std::vector<boost::shared_ptr<PiecewiseConstantVariance> > originalVariances_;
         std::vector< PiecewiseConstantAbcdVariance> originalABCDVariances_;
         std::vector< PiecewiseConstantAbcdVariance> originalABCDVariancesScaled_;
         Real lastCapletVol_;
         std::vector<Time> timesForSmallRates_;
         std::vector<Real> scalingFactors_;

         Size noBigRates_;
         Size noSmallRates_;

         void recompute();

    };
}

#endif
