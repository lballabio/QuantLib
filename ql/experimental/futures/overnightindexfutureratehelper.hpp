/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Roy Zywina

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

/*! \file overnightindexfutureratehelper.hpp
    \brief Overnight Index Future bootstrap helper
*/

#ifndef quantlib_overnightindexfutureratehelper_hpp
#define quantlib_overnightindexfutureratehelper_hpp

#include <ql/experimental/futures/overnightindexfuture.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>

namespace QuantLib {

    //! RateHelper for bootstrapping over overnight compounding futures
    class OvernightIndexFutureRateHelper : public RateHelper {
    public:
        OvernightIndexFutureRateHelper(
          const Handle<Quote>& price,
          // first day of reference period
          const Date& valueDate,
          // delivery date
          const Date& maturityDate,
          const ext::shared_ptr<OvernightIndex>& overnightIndex,
          const Handle<Quote>& convexityAdjustment = Handle<Quote>());

        //! \name RateHelper interface
        //@{
        Real impliedQuote() const;
        void setTermStructure(YieldTermStructure*);
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
        Real convexityAdjustment() const;
    private:
        ext::shared_ptr<OvernightIndexFuture> future_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };

    /*!
    RateHelper for bootstrapping over CME SOFR futures

    Compounds with overnight SOFR rates from the third Wednesday of the
    reference month/year (inclusive) to the third Wednesday of the month
    one Month/Quarter later (exclusive).

    Requires index history populated when reference period starts in the past.
    */
    class SofrFutureRateHelper : public OvernightIndexFutureRateHelper {
    public:
        SofrFutureRateHelper(
            const Handle<Quote>& price,
            Month referenceMonth,
            Year referenceYear,
            Frequency referenceFreq,
            const ext::shared_ptr<OvernightIndex>& overnightIndex,
            const Handle<Quote>& convexityAdjustment = Handle<Quote>());
        SofrFutureRateHelper(
            Real price,
            Month referenceMonth,
            Year referenceYear,
            Frequency referenceFreq,
            const ext::shared_ptr<OvernightIndex>& overnightIndex,
            Real convexityAdjustment = 0);
    };

}

#endif
