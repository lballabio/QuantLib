/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Roy Zywina
 Copyright (C) 2019 Eisuke Tani

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

/*! \file overnightindexfuture.hpp
    \brief Overnight Index Future
*/

#ifndef quantlib_overnightindexfuture_hpp
#define quantlib_overnightindexfuture_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/forward.hpp>
#include <ql/cashflows/rateaveraging.hpp>

namespace QuantLib {

    /*! Future on a compounded overnight index investment.

        Compatible with SOFR futures and Sonia futures available on
        CME and ICE exchanges.
    */
    class OvernightIndexFuture : public Instrument {
      public:
        OvernightIndexFuture(
            ext::shared_ptr<OvernightIndex> overnightIndex,
            const Date& valueDate,
            const Date& maturityDate,
            Handle<Quote> convexityAdjustment = Handle<Quote>(),
            RateAveraging::Type averagingMethod = RateAveraging::Compound);

        Real convexityAdjustment() const;
        bool isExpired() const override;
        const ext::shared_ptr<OvernightIndex>& overnightIndex() const { return overnightIndex_; }
        Date valueDate() const { return valueDate_; }
        Date maturityDate() const { return maturityDate_; }
      private:
        void performCalculations() const override;
        Real rate() const;
        Real averagedRate() const;
        Real compoundedRate() const;
        ext::shared_ptr<OvernightIndex> overnightIndex_;
        Date valueDate_, maturityDate_;
        Handle<Quote> convexityAdjustment_;
        RateAveraging::Type averagingMethod_;
    };

}

#endif


#ifndef id_fe2369cd781c0e3f8a324d8e9f7fe08b
#define id_fe2369cd781c0e3f8a324d8e9f7fe08b
inline bool test_fe2369cd781c0e3f8a324d8e9f7fe08b(int* i) { return i != 0; }
#endif
