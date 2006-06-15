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

#include <ql/index.hpp>
#include <ql/yieldtermstructure.hpp>

namespace QuantLib {

    //! base class for LIBOR-like indexes
    /*! \todo add methods returning InterestRate */
    class Xibor : public Index, public Observer {
      public:
        Xibor(const std::string& familyName,
              Integer n, TimeUnit units,
              Integer settlementDays,
              const Currency& currency,
              const Calendar& calendar,
              BusinessDayConvention convention,
              const DayCounter& dayCounter,
              const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
        Xibor(const std::string& familyName,
              const Period& tenor,
              Integer settlementDays,
              const Currency& currency,
              const Calendar& calendar,
              BusinessDayConvention convention,
              const DayCounter& dayCounter,
              const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
        //! \name Index interface
        //@{
        Rate fixing(const Date& fixingDate,
                    bool forecastTodaysFixing = false) const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Inspectors
        //@{
        std::string name() const;
        std::string familyName() const;
        Period tenor() const;
        Frequency frequency() const;
        Integer settlementDays() const;
        const Currency& currency() const;
        Calendar calendar() const;
        bool isAdjusted() const;
        BusinessDayConvention businessDayConvention() const;
        DayCounter dayCounter() const;
        boost::shared_ptr<YieldTermStructure> termStructure() const;
        //@}
        /*| \name Date calculations

            These methods can be overridden to implement particular
            conventions

            @{
        */
        virtual Date valueDate(const Date& fixingDate) const;
        virtual Date maturityDate(const Date& valueDate) const;
        //@}
      protected:
        std::string familyName_;
        Period tenor_;
        Integer settlementDays_;
        Currency currency_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        DayCounter dayCounter_;
        Handle<YieldTermStructure> termStructure_;
    };


    // inline definitions

    inline void Xibor::update() {
        notifyObservers();
    }

    inline std::string Xibor::familyName() const {
        return familyName_;
    }

    inline Period Xibor::tenor() const {
        return tenor_;
    }

    inline Integer Xibor::settlementDays() const {
        return settlementDays_;
    }

    inline const Currency& Xibor::currency() const {
        return currency_;
    }

    inline Calendar Xibor::calendar() const {
        return calendar_;
    }

    inline bool Xibor::isAdjusted() const {
        return (convention_ != Unadjusted);
    }

    inline BusinessDayConvention Xibor::businessDayConvention() const {
        return convention_;
    }

    inline DayCounter Xibor::dayCounter() const {
        return dayCounter_;
    }

    inline boost::shared_ptr<YieldTermStructure> Xibor::termStructure() const {
        return termStructure_.currentLink();
    }

}


#endif
