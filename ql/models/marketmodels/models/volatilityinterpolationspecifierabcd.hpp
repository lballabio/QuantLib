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
#include <ql/shared_ptr.hpp>
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

        ~VolatilityInterpolationSpecifierabcd() override = default;
        void setScalingFactors(const std::vector<Real>& scales) override;
        void setLastCapletVol(Real vol) override;


        const std::vector<ext::shared_ptr<PiecewiseConstantVariance> >&
        interpolatedVariances() const override;
        const std::vector<ext::shared_ptr<PiecewiseConstantVariance> >&
        originalVariances() const override;

        Size getPeriod() const override;
        Size getOffset() const override;
        Size getNoBigRates() const override;
        Size getNoSmallRates() const override;

      private:
        Size period_;
        Size offset_;

         std::vector<ext::shared_ptr<PiecewiseConstantVariance> > interpolatedVariances_;
         std::vector<ext::shared_ptr<PiecewiseConstantVariance> > originalVariances_;
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


#ifndef id_1f2d01736d399731b5c7cd771650c67d
#define id_1f2d01736d399731b5c7cd771650c67d
inline bool test_1f2d01736d399731b5c7cd771650c67d(const int* i) {
    return i != nullptr;
}
#endif
