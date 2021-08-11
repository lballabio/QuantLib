/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Roland Lichters

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

/*! \file bmaindex.hpp
    \brief Bond Market Association index
*/

#ifndef quantlib_bma_index_hpp
#define quantlib_bma_index_hpp

#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/indexes/interestrateindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! Bond Market Association index
    /*! The BMA index is the short-term tax-exempt reference index of
        the Bond Market Association.  It has tenor one week, is fixed
        weekly on Wednesdays and is applied with a one-day's fixing
        gap from Thursdays on for one week.  It is the tax-exempt
        correspondent of the 1M USD-Libor.
    */
    class BMAIndex : public InterestRateIndex {
      public:
        explicit BMAIndex(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
        //! \name Index interface
        //@{
        /*! BMA is fixed weekly on Wednesdays.
        */
        bool isValidFixingDate(const Date& fixingDate) const override;
        //@}
        //! \name Inspectors
        //@{
        Handle<YieldTermStructure> forwardingTermStructure() const;
        //@}
        //! \name Date calculations
        //@{
        Date maturityDate(const Date& valueDate) const override;
        /*! This method returns a schedule of fixing dates between
            start and end.
        */
        Schedule fixingSchedule(const Date& start,
                                const Date& end);
        // @}
      protected:
        Rate forecastFixing(const Date& fixingDate) const override;
        Handle<YieldTermStructure> termStructure_;
    };

}

#endif
