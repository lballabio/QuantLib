/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file xibor.hpp
    \brief base class for LIBOR-like indexes
*/

#ifndef quantlib_xibor_hpp
#define quantlib_xibor_hpp

#include <ql/Indexes/interestrateindex.hpp>

namespace QuantLib {

    //! base class for LIBOR-like indexes
    /*! \todo add methods returning InterestRate */
    class Xibor : public InterestRateIndex {
      public:
        Xibor(const std::string& familyName,
              const Period& p,
              Integer settlementDays,
              const Currency& currency,
              const Calendar& calendar,
              BusinessDayConvention convention,
              const DayCounter& dayCounter,
              const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
        //! \name InterestRateIndex interface
        //@{
        Rate forecastFixing(const Date& fixingDate) const;
        boost::shared_ptr<YieldTermStructure> termStructure() const;
        //@}
        //! \name Inspectors
        //@{
        /*! \note this method does not always apply. Use tenor() if
                  possible.
        */
        Frequency frequency() const;
        bool isAdjusted() const;
        BusinessDayConvention businessDayConvention() const;
        //@}
        //! \name Date calculations
        //@{
        virtual Date maturityDate(const Date& valueDate) const;
        // @}
      protected:
        BusinessDayConvention convention_;
        Handle<YieldTermStructure> termStructure_;
    };

    #ifndef QL_DISABLE_DEPRECATED
    typedef IborIndex Xibor;
    #endif
    
    // inline definitions

    inline bool Xibor::isAdjusted() const {
        return (convention_ != Unadjusted);
    }

    inline BusinessDayConvention Xibor::businessDayConvention() const {
        return convention_;
    }

   inline boost::shared_ptr<YieldTermStructure> Xibor::termStructure() const {
       return termStructure_.currentLink();
   }

}

#endif
