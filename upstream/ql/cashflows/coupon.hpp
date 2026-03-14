/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file coupon.hpp
    \brief Coupon accruing over a fixed period
*/

#ifndef quantlib_coupon_hpp
#define quantlib_coupon_hpp

#include <ql/cashflow.hpp>

namespace QuantLib {

    class DayCounter;

    //! %coupon accruing over a fixed period
    /*! This class implements part of the CashFlow interface but it is
        still abstract and provides derived classes with methods for
        accrual period calculations.
    */
    class Coupon : public CashFlow {
      public:
        /*! \warning the coupon does not adjust the payment date which
                     must already be a business day.
        */
        Coupon(const Date& paymentDate,
               Real nominal,
               const Date& accrualStartDate,
               const Date& accrualEndDate,
               const Date& refPeriodStart = Date(),
               const Date& refPeriodEnd = Date(),
               const Date& exCouponDate = Date());
        //! \name Event interface
        //@{
        Date date() const override { return paymentDate_; }
        //@}
        //! \name CashFlow interface
        //@{
        Date exCouponDate() const override { return exCouponDate_; }
        //@}
        //! \name Inspectors
        //@{
        virtual Real nominal() const;
        //! start of the accrual period
        const Date& accrualStartDate() const;
        //! end of the accrual period
        const Date& accrualEndDate() const;
        //! start date of the reference period
        const Date& referencePeriodStart() const;
        //! end date of the reference period
        const Date& referencePeriodEnd() const;
        //! accrual period as fraction of year
        Time accrualPeriod() const;
        //! accrual period in days
        Date::serial_type accrualDays() const;
        //! accrued rate
        virtual Rate rate() const = 0;
        //! day counter for accrual calculation
        virtual DayCounter dayCounter() const = 0;
        //! accrued period as fraction of year at the given date
        Time accruedPeriod(const Date&) const;
        //! accrued days at the given date
        Date::serial_type accruedDays(const Date&) const;
        //! accrued amount at the given date
        virtual Real accruedAmount(const Date&) const = 0;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        Date paymentDate_;
        Real nominal_;
        Date accrualStartDate_,accrualEndDate_, refPeriodStart_,refPeriodEnd_;
        Date exCouponDate_;
        mutable Real accrualPeriod_;
    };


    // inline definitions

    inline Real Coupon::nominal() const {
        return nominal_;
    }

    inline const Date& Coupon::accrualStartDate() const {
        return accrualStartDate_;
    }

    inline const Date& Coupon::accrualEndDate() const {
        return accrualEndDate_;
    }

    inline const Date& Coupon::referencePeriodStart() const {
        return refPeriodStart_;
    }

    inline const Date& Coupon::referencePeriodEnd() const {
        return refPeriodEnd_;
    }

}

#endif
