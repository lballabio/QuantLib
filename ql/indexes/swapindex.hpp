/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006, 2007, 2009 StatPro Italia srl

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

/*! \file swapindex.hpp
    \brief swap-rate indexes
*/

#ifndef quantlib_swapindex_hpp
#define quantlib_swapindex_hpp

#include <ql/indexes/interestrateindex.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    class IborIndex;
    class Schedule;
    class VanillaSwap;
    class YieldTermStructure;

    //! base class for swap-rate indexes
    class SwapIndex : public InterestRateIndex {
      public:
        SwapIndex(const std::string& familyName,
                  const Period& tenor,
                  Natural settlementDays,
                  Currency currency,
                  const Calendar& calendar,
                  const Period& fixedLegTenor,
                  BusinessDayConvention fixedLegConvention,
                  const DayCounter& fixedLegDayCounter,
                  const boost::shared_ptr<IborIndex>& iborIndex);
        //! \name InterestRateIndex interface
        //@{
        Date maturityDate(const Date& valueDate) const;
        //@}
        //! \name Inspectors
        //@{
        Period fixedLegTenor() const { return fixedLegTenor_; }
        BusinessDayConvention fixedLegConvention() const;
        boost::shared_ptr<IborIndex> iborIndex() const;
        Handle<YieldTermStructure> forwardingTermStructure() const;
        /*! \warning Relinking the term structure underlying the index will
                     not have effect on the returned swap.
        */
        boost::shared_ptr<VanillaSwap> underlyingSwap(
                                                const Date& fixingDate) const;
        //@}
      protected:
        Rate forecastFixing(const Date& fixingDate) const;
        Period tenor_;
        boost::shared_ptr<IborIndex> iborIndex_;
        Period fixedLegTenor_;
        BusinessDayConvention fixedLegConvention_;
    };


    // inline definitions

    inline BusinessDayConvention SwapIndex::fixedLegConvention() const {
        return fixedLegConvention_;
    }

    inline boost::shared_ptr<IborIndex> SwapIndex::iborIndex() const {
        return iborIndex_;
    }

}

#endif
