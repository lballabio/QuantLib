/*
 Copyright (C) 2014 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file swapspreadindex.hpp
    \brief swap-rate spread indexes
*/

#ifndef quantlib_swapspreadindex_hpp
#define quantlib_swapspreadindex_hpp

#include <ql/indexes/swapindex.hpp>

namespace QuantLib {

    //! class for swap-rate spread indexes
    class SwapSpreadIndex : public InterestRateIndex {
      public:
        SwapSpreadIndex(const std::string& familyName,
                        const ext::shared_ptr<SwapIndex>& swapIndex1,
                        ext::shared_ptr<SwapIndex> swapIndex2,
                        Real gearing1 = 1.0,
                        Real gearing2 = -1.0);

        //! \name InterestRateIndex interface
        //@{
        Date maturityDate(const Date& valueDate) const override {
            QL_FAIL("SwapSpreadIndex does not provide a single maturity date");
        }
        Rate forecastFixing(const Date& fixingDate) const override;
        Rate pastFixing(const Date& fixingDate) const override;
        bool allowsNativeFixings() override { return false; }
        //@}

        //! \name Inspectors
        //@{
        ext::shared_ptr<SwapIndex> swapIndex1() { return swapIndex1_; }
        ext::shared_ptr<SwapIndex> swapIndex2() { return swapIndex2_; }
        Real gearing1() const { return gearing1_; }
        Real gearing2() const { return gearing2_; }
        //@}


    private:
        ext::shared_ptr<SwapIndex> swapIndex1_, swapIndex2_;
        Real gearing1_, gearing2_;
    };


    inline Rate SwapSpreadIndex::forecastFixing(const Date& fixingDate) const {
        // this also handles the case when one of indices has
        // a historic fixing on the evaluation date
        return gearing1_ * swapIndex1_->fixing(fixingDate,false) +
            gearing2_ * swapIndex2_->fixing(fixingDate,false);

    }

    inline Rate SwapSpreadIndex::pastFixing(const Date& fixingDate) const {

        Real f1 = swapIndex1_->pastFixing(fixingDate);
        Real f2 = swapIndex2_->pastFixing(fixingDate);
        // if one of the fixings is missing we return null, indicating
        // a missing fixing for the spread index
        if(f1 == Null<Real>() || f2 == Null<Real>())
            return Null<Real>();
        else
            return gearing1_ * f1 + gearing2_ * f2;
    }

}

#endif


#ifndef id_ddb04e076d7bf17050700c2b31b18dca
#define id_ddb04e076d7bf17050700c2b31b18dca
inline bool test_ddb04e076d7bf17050700c2b31b18dca(const int* i) {
    return i != nullptr;
}
#endif
