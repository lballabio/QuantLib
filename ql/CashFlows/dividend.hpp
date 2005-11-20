/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file dividend.hpp
    \brief A stock dividend
*/

#ifndef quantlib_dividend_hpp
#define quantlib_dividend_hpp

#include <ql/cashflow.hpp>

namespace QuantLib {

    //! Predetermined cash flow
    /*! This cash flow pays a predetermined amount at a given date. */
    class Dividend : public CashFlow {
      public:
        Dividend(Real amount, const Date& date)
        : amount_(amount), date_(date) {}
        //! \name CashFlow interface
        //@{
        Real amount() const { return amount_; }
        Date date() const { return date_; }
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        Real amount_;
        Date date_;
    };


    // inline definitions

    inline void Dividend::accept(AcyclicVisitor& v) {
        Visitor<Dividend>* v1 =
            dynamic_cast<Visitor<Dividend>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            CashFlow::accept(v);
    }

}


#endif
