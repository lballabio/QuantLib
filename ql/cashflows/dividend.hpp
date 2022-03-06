/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file dividend.hpp
    \brief A stock dividend
*/

#ifndef quantlib_dividend_hpp
#define quantlib_dividend_hpp

#include <ql/cashflow.hpp>
#include <ql/utilities/null.hpp>
#include <vector>

namespace QuantLib {

    //! Predetermined cash flow
    /*! This cash flow pays a predetermined amount at a given date. */
    class Dividend : public CashFlow {
      public:
        Dividend(const Date& date)
        : date_(date) {}
        //! \name Event interface
        //@{
        Date date() const override { return date_; }
        //@}
        //! \name CashFlow interface
        //@{
        Real amount() const override = 0;
        //@}
        virtual Real amount(Real underlying) const = 0;
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
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
        //! \name Dividend interface
        //@{
        Real amount() const override { return amount_; }
        Real amount(Real) const override { return amount_; }
        //@}
      protected:
        Real amount_;
    };

    //! Predetermined cash flow
    /*! This cash flow pays a fractional amount at a given date. */
    class FractionalDividend : public Dividend {
      public:
        FractionalDividend(Real rate, const Date& date)
        : Dividend(date), rate_(rate), nominal_(Null<Real>()) {}

        FractionalDividend(Real rate, Real nominal, const Date& date)
        : Dividend(date), rate_(rate), nominal_(nominal) {}
        //! \name Dividend interface
        //@{
        Real amount() const override {
            QL_REQUIRE(nominal_ != Null<Real>(), "no nominal given");
            return rate_ * nominal_;
        }
        Real amount(Real underlying) const override { return rate_ * underlying; }
        //@}
        //! \name Inspectors
        //@{
        Real rate() const { return rate_; }
        Real nominal() const { return nominal_; }
        //@}
      protected:
        Real rate_;
        Real nominal_;
    };


    //! helper function building a sequence of fixed dividends
    std::vector<ext::shared_ptr<Dividend> >
    DividendVector(const std::vector<Date>& dividendDates,
                   const std::vector<Real>& dividends);

}


#endif


#ifndef id_8e0f4f0323a07cff5fc69ed9e356cd1a
#define id_8e0f4f0323a07cff5fc69ed9e356cd1a
inline bool test_8e0f4f0323a07cff5fc69ed9e356cd1a(int* i) { return i != 0; }
#endif
