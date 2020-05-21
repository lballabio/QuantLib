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

namespace QuantLib {

    /*
    Future on a compounded overnight index investment. Compatable with
    SOFR futures and Sonia futures available on CME and ICE exchanges.
    */
    class OvernightIndexFuture : public Forward {
      public:
        enum NettingType { Averaging, Compounding };

        OvernightIndexFuture(const ext::shared_ptr<OvernightIndex>& overnightIndex,
                             const ext::shared_ptr<Payoff>& payoff,
                             const Date& valueDate,
                             const Date& maturityDate,
                             const Handle<YieldTermStructure>& discountCurve,
                             const Handle<Quote>& convexityAdjustment = Handle<Quote>(),
                             NettingType subPeriodsNettingType = Compounding);

        //! returns spot value/price of an underlying financial instrument
        virtual Real spotValue() const;

        //! NPV of income/dividends/storage-costs etc. of underlying instrument
        virtual Real spotIncome(const Handle<YieldTermStructure>&) const;

        virtual Real forwardValue() const;

        Real convexityAdjustment() const;

      private:
        Real averagedSpotValue() const;
        Real compoundedSpotValue() const;
        ext::shared_ptr<OvernightIndex> overnightIndex_;
        Handle<Quote> convexityAdjustment_;
        NettingType subPeriodsNettingType_;
    };

}

#endif
