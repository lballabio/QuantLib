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
        Dividend(const Date& date)
        : date_(date) {}
        //! \name CashFlow interface
        //@{
        virtual Date date() const { return date_; }
        virtual Real amount() const = 0;
        //@}
        virtual Real amount(Real underlying) const = 0;
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
    protected:
        Date date_;
    };

    //! Predetermined cash flow
    /*! This cash flow pays a predetermined amount at a given date. */
    class FixedDividend : public Dividend {
      public:
        FixedDividend(Real amount, const Date& date)
        : Dividend(date), amount_(amount) {}
        //! \name CashFlow interface
        //@{
        virtual Real amount() const { return amount_; }
        virtual Real amount(Real underlying) const { return amount_;}
        //@}
    protected:
        Real amount_;
    };

    //! Predetermined cash flow
    /*! This cash flow pays a predetermined amount at a given date. */
    class FractionalDividend : public Dividend {
      public:
        FractionalDividend(Real rate, const Date& date)
        : Dividend(date), rate_(rate), nominal_(0.0) {}

        FractionalDividend(Real rate, Real nominal, const Date& date)
        : Dividend(date), rate_(rate), nominal_(nominal) {}
        //! \name CashFlow interface
        //@{
        virtual Real amount() const { return rate_ * nominal_; }
        virtual Real amount(Real underlying) const 
             { return rate_ * underlying;}
        virtual Real rate() const { return rate_; }
        virtual Real nominal() const { return nominal_;}
        //@}
    protected:
        Real rate_;
        Real nominal_;
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
