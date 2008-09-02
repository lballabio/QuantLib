/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file default.hpp
    \brief Classes for default-event handling.
*/

#ifndef quantlib_default_hpp
#define quantlib_default_hpp

#include <ql/event.hpp>

#ifdef SEC
#undef SEC
#endif

namespace QuantLib {

    //! Seniority of a default event
    enum Seniority { SEN, SUB, SEC, TI0, TI1, PCL, AnySeniority };

    //! Restructuring of a default event
    enum Restructuring { NR, MR, MM, RR, AnyRestructuring };

    //! Credit-default event
    class DefaultEvent : public Event {
      public:
        DefaultEvent(const Date& date,
                     Real recoveryRatio,
                     Seniority seniority = AnySeniority,
                     Restructuring restructuring = AnyRestructuring);
        Date date() const;
        Real recoveryRatio() const;
        Seniority seniority() const;
        Restructuring restructuring() const;
      private:
        Date defaultDate_;
        Rate recoveryRatio_;
        Seniority seniority_;
        Restructuring restructuring_;
        void accept(AcyclicVisitor&);
    };

    //! information on a default-protection contract
    struct Protection {
        enum Side { Buyer, Seller };
    };

}


#endif
