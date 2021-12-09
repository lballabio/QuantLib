/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Chris Kenyon

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


#include <ql/experimental/inflation/cpicapfloortermpricesurface.hpp>
#include <utility>


namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

    CPICapFloorTermPriceSurface::CPICapFloorTermPriceSurface(
        Real nominal,
        Real baseRate, // avoids an uncontrolled crash if index has no TS
        const Period& observationLag,
        const Calendar& cal, // calendar in index may not be useful
        const BusinessDayConvention& bdc,
        const DayCounter& dc,
        const Handle<ZeroInflationIndex>& zii,
        Handle<YieldTermStructure> yts,
        const std::vector<Rate>& cStrikes,
        const std::vector<Rate>& fStrikes,
        const std::vector<Period>& cfMaturities,
        const Matrix& cPrice,
        const Matrix& fPrice)
    : InflationTermStructure(
          0, cal, baseRate, observationLag, zii->frequency(), zii->interpolated(), dc),
      zii_(zii), nominalTS_(std::move(yts)), cStrikes_(cStrikes), fStrikes_(fStrikes),
      cfMaturities_(cfMaturities), cPrice_(cPrice), fPrice_(fPrice), nominal_(nominal), bdc_(bdc) {

        // does the index have a TS?
        QL_REQUIRE(!zii_->zeroInflationTermStructure().empty(), "ZITS missing from index");
        QL_REQUIRE(!nominalTS_.empty(), "nominal TS missing");
              
        // data consistency checking, enough data?
        QL_REQUIRE(fStrikes_.size() > 1, "not enough floor strikes");
        QL_REQUIRE(cStrikes_.size() > 1, "not enough cap strikes");
        QL_REQUIRE(cfMaturities_.size() > 1, "not enough maturities");
        QL_REQUIRE(fStrikes_.size() == fPrice.rows(),
                   "floor strikes vs floor price rows not equal");
        QL_REQUIRE(cStrikes_.size() == cPrice.rows(),
                   "cap strikes vs cap price rows not equal");
        QL_REQUIRE(cfMaturities_.size() == fPrice.columns(),
                   "maturities vs floor price columns not equal");
        QL_REQUIRE(cfMaturities_.size() == cPrice.columns(),
                   "maturities vs cap price columns not equal");

        // data has correct properties (positive, monotonic)?
        for(Size j = 0; j <cfMaturities_.size(); j++) {
            QL_REQUIRE( cfMaturities[j] > Period(0,Days), "non-positive maturities");
            if(j>0) {
                QL_REQUIRE( cfMaturities[j] > cfMaturities[j-1],
                            "non-increasing maturities");
            }
            for(Size i = 0; i <fPrice_.rows(); i++) {
                QL_REQUIRE( fPrice_[i][j] > 0.0,
                            "non-positive floor price: " << fPrice_[i][j] );
                if(i>0) {
                    QL_REQUIRE( fPrice_[i][j] >= fPrice_[i-1][j],
                                "non-increasing floor prices");
                }
            }
            for(Size i = 0; i <cPrice_.rows(); i++) {
                QL_REQUIRE( cPrice_[i][j] > 0.0,
                            "non-positive cap price: " << cPrice_[i][j] );
                if(i>0) {
                    QL_REQUIRE( cPrice_[i][j] <= cPrice_[i-1][j],
                                "non-decreasing cap prices: " 
                               << cPrice_[i][j] << " then " << cPrice_[i-1][j]);
                }
            }
        }


        // Get the set of strikes, noting that repeats, overlaps are
        // expected between caps and floors but that no overlap in the
        // output is allowed so no repeats or overlaps are used
        cfStrikes_ = std::vector<Rate>();
        for(Size i = 0; i <fStrikes_.size(); i++)
            cfStrikes_.push_back( fStrikes[i] );
        Real eps = 0.0000001;
        Rate maxFstrike = fStrikes_.back();
        for(Size i = 0; i < cStrikes_.size(); i++) {
            Rate k = cStrikes[i];
            if (k > maxFstrike + eps) cfStrikes_.push_back(k);
        }

        // final consistency checking
        QL_REQUIRE(cfStrikes_.size() > 2, "overall not enough strikes");
        for (Size i = 1; i < cfStrikes_.size(); i++)
            QL_REQUIRE( cfStrikes_[i] > cfStrikes_[i-1],
                        "cfStrikes not increasing");
    }

    QL_DEPRECATED_ENABLE_WARNING


    Date CPICapFloorTermPriceSurface::cpiOptionDateFromTenor(const Period& p) const
    {
        return calendar().adjust(referenceDate() + p, businessDayConvention());
    }

    
    Real CPICapFloorTermPriceSurface::price(const Period &d, Rate k) const {
        return this->price(cpiOptionDateFromTenor(d), k);
    }
    

    Real CPICapFloorTermPriceSurface::capPrice(const Period &d, Rate k) const {
        return this->capPrice(cpiOptionDateFromTenor(d), k);
    }
    

    Real CPICapFloorTermPriceSurface::floorPrice(const Period &d, Rate k) const {
        return this->floorPrice(cpiOptionDateFromTenor(d), k);
    }
    
    
    

}

