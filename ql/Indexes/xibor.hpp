
/*
 Copyright (C) 2000-2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file xibor.hpp
    \brief base class for libor indexes
*/

#ifndef quantlib_xibor_hpp
#define quantlib_xibor_hpp

#include <ql/index.hpp>
#include <ql/termstructure.hpp>

namespace QuantLib {

    //! base class for libor indexes
    class Xibor : public Index, public Observer {
      public:
        Xibor(const std::string& familyName,
              Integer n, TimeUnit units, Integer settlementDays,
              CurrencyTag currency,
              const Calendar& calendar,
              BusinessDayConvention convention,
              const DayCounter& dayCounter, 
              const RelinkableHandle<TermStructure>& h)
        : familyName_(familyName), n_(n), units_(units),
          settlementDays_(settlementDays),
          currency_(currency), calendar_(calendar),
          convention_(convention),
          dayCounter_(dayCounter), termStructure_(h) {
            registerWith(termStructure_);
        }
#ifndef QL_DISABLE_DEPRECATED
        /*! \deprecated use the constructor without <tt>isAdjusted</tt>
                        argument; <tt>isAdjusted = false</tt> can be 
                        replicated by <tt>convention = Unadjusted</tt>.
        */
        Xibor(const std::string& familyName,
              Integer n, TimeUnit units, Integer settlementDays,
              CurrencyTag currency,
              const Calendar& calendar, bool isAdjusted,
              BusinessDayConvention convention,
              const DayCounter& dayCounter, 
              const RelinkableHandle<TermStructure>& h)
        : familyName_(familyName), n_(n), units_(units),
          settlementDays_(settlementDays),
          currency_(currency), calendar_(calendar),
          dayCounter_(dayCounter), termStructure_(h) {
            registerWith(termStructure_);
            convention_ = isAdjusted ? convention : Unadjusted;
        }
#endif
        //! \name Index interface
        //@{
        Rate fixing(const Date& fixingDate) const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Inspectors
        //@{
        std::string name() const;
        Period tenor() const;
        Frequency frequency() const;
        Integer settlementDays() const;
        CurrencyTag currency() const;
        Calendar calendar() const;
        bool isAdjusted() const;
        BusinessDayConvention businessDayConvention() const;
#ifndef QL_DISABLE_DEPRECATED
        //! \deprecated renamed to businessDayConvention()
        BusinessDayConvention rollingConvention() const;
#endif
        DayCounter dayCounter() const;
        boost::shared_ptr<TermStructure> termStructure() const;
        //@}
      private:
        std::string familyName_;
        Integer n_;
        TimeUnit units_;
        Integer settlementDays_;
        CurrencyTag currency_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        DayCounter dayCounter_;
        RelinkableHandle<TermStructure> termStructure_;
    };


    // inline definitions

    inline void Xibor::update() {
        notifyObservers();
    }

    inline Period Xibor::tenor() const { 
        return Period(n_,units_); 
    }

    inline Integer Xibor::settlementDays() const { 
        return settlementDays_; 
    }

    inline CurrencyTag Xibor::currency() const { 
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

#ifndef QL_DISABLE_DEPRECATED
    inline BusinessDayConvention Xibor::rollingConvention() const {
        return convention_; 
    }
#endif

    inline DayCounter Xibor::dayCounter() const { 
        return dayCounter_; 
    }

    inline boost::shared_ptr<TermStructure> Xibor::termStructure() const {
        return termStructure_.currentLink();
    }

}


#endif
