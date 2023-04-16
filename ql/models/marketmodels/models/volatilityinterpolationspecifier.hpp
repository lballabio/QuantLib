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
/*! abstract base class
implementation specifies how to decide volatility structure for additional
synthetic rates which are interleaved

*/

#ifndef volatility_interpolation_specifier_hpp
#define  volatility_interpolation_specifier_hpp



#include <ql/models/marketmodels/models/piecewiseconstantvariance.hpp>
#include <ql/types.hpp>
#include <memory>
#include <vector>


namespace QuantLib
{
    class VolatilityInterpolationSpecifier
    {
      public:
        VolatilityInterpolationSpecifier() = default;
        virtual ~VolatilityInterpolationSpecifier() = default;
        virtual void setScalingFactors(const std::vector<Real>& scales)=0;
        virtual void setLastCapletVol(Real vol)=0;


        virtual const std::vector<std::shared_ptr<PiecewiseConstantVariance> >& interpolatedVariances() const=0;
        virtual const std::vector<std::shared_ptr<PiecewiseConstantVariance> >& originalVariances() const=0;

        virtual Size getPeriod() const=0;
        virtual Size getOffset() const=0;
        virtual Size getNoBigRates() const=0;
        virtual Size getNoSmallRates() const=0;
    };
}

#endif
