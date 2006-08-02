/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
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

/*! \file interestrateindex.hpp
    \brief base class for interest rate indexes
*/

#ifndef quantlib_interestrateindex_hpp
#define quantlib_interestrateindex_hpp

#include <ql/index.hpp>
#include <ql/yieldtermstructure.hpp>

namespace QuantLib {

    //! base class for interest rate indexes
    /*! \todo add methods returning InterestRate */
    class InterestRateIndex : public Index, public Observer {
      public:
        InterestRateIndex(
              const std::string& familyName,
              const Period& tenor,
              Integer settlementDays,
              const Currency& currency,
              const Calendar& calendar,
              const DayCounter& dayCounter);
        //! \name Index interface
        //@{
        std::string name() const;
        Rate fixing(const Date& fixingDate,
                    bool forecastTodaysFixing = false) const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Inspectors
        //@{
        std::string familyName() const;
        Period tenor() const;
        Integer settlementDays() const;
        const Currency& currency() const;
        Calendar calendar() const;
        const DayCounter& dayCounter() const;
        virtual Rate forecastFixing(const Date& fixingDate) const = 0;
        virtual boost::shared_ptr<YieldTermStructure> termStructure() const = 0;
        //@}
        /*! \name Date calculations

            These methods can be overridden to implement particular
            conventions

            @{
        */
        virtual Date valueDate(const Date& fixingDate) const;
        virtual Date maturityDate(const Date& valueDate) const;
        // @}
      protected:
        std::string familyName_;
        Period tenor_;
        Integer settlementDays_;
        Currency currency_;
        Calendar calendar_;
        DayCounter dayCounter_;
    };


    // inline definitions

    inline void InterestRateIndex::update() {
        notifyObservers();
    }

    inline std::string InterestRateIndex::familyName() const {
        return familyName_;
    }

    inline Period InterestRateIndex::tenor() const {
        return tenor_;
    }

    inline Integer InterestRateIndex::settlementDays() const {
        return settlementDays_;
    }

    inline const Currency& InterestRateIndex::currency() const {
        return currency_;
    }

    inline Calendar InterestRateIndex::calendar() const {
        return calendar_;
    }

    inline const DayCounter& InterestRateIndex::dayCounter() const {
        return dayCounter_;
    }

}

#endif
