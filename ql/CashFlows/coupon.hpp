
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file coupon.hpp
    \brief Coupon accruing over a fixed period
*/

#ifndef quantlib_coupon_hpp
#define quantlib_coupon_hpp

#include <ql/cashflow.hpp>
#include <ql/calendar.hpp>
#include <ql/daycounter.hpp>

namespace QuantLib {

    //! %coupon accruing over a fixed period
    /*! This class implements part of the CashFlow interface but it
      is still abstract and provides derived classes with methods for
      accrual period calculations.
    */
    class Coupon : public CashFlow {
      public:
        /*! \warning the coupon does not roll the payment date
          which must already be a business day.
        */
        Coupon(Real nominal,
               const Date& paymentDate, 
               const Date& accrualStartDate, 
               const Date& accrualEndDate,
               const Date& refPeriodStart = Date(),
               const Date& refPeriodEnd = Date());
        //! \name Partial CashFlow interface
        //@{
        Date date() const;
        //@}
        //! \name Inspectors
        //@{
        Real nominal() const;
        //! start of the accrual period
        const Date& accrualStartDate() const;
        //! end of the accrual period
        const Date& accrualEndDate() const;
        //! accrual period as fraction of year
        Time accrualPeriod() const;
        //! accrual period in days
        Integer accrualDays() const;
        //! day counter for accrual calculation
        virtual DayCounter dayCounter() const = 0;
        //! accrued amount at the given date
        virtual Real accruedAmount(const Date&) const = 0;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        Real nominal_;
        Date paymentDate_, accrualStartDate_, accrualEndDate_, 
             refPeriodStart_, refPeriodEnd_;
    };


    // inline definitions

    inline Coupon::Coupon(Real nominal,
                          const Date& paymentDate, 
                          const Date& accrualStartDate, 
                          const Date& accrualEndDate,
                          const Date& refPeriodStart, 
                          const Date& refPeriodEnd)
    : nominal_(nominal), paymentDate_(paymentDate), 
      accrualStartDate_(accrualStartDate), accrualEndDate_(accrualEndDate),
      refPeriodStart_(refPeriodStart), refPeriodEnd_(refPeriodEnd) {
        if (refPeriodStart_ == Date())
            refPeriodStart_ = accrualStartDate_;
        if (refPeriodEnd_ == Date())
            refPeriodEnd_ = accrualEndDate_;
    }

    inline Date Coupon::date() const {
        return paymentDate_;
    }

    inline Real Coupon::nominal() const {
        return nominal_;
    }

    inline const Date& Coupon::accrualStartDate() const {
        return accrualStartDate_;
    }

    inline const Date& Coupon::accrualEndDate() const {
        return accrualEndDate_;
    }

    inline Time Coupon::accrualPeriod() const {
        return dayCounter().yearFraction(accrualStartDate_,
                                         accrualEndDate_,
                                         refPeriodStart_,
                                         refPeriodEnd_);
    }

    inline Integer Coupon::accrualDays() const {
        return dayCounter().dayCount(accrualStartDate_,
                                     accrualEndDate_);
    }

    inline void Coupon::accept(AcyclicVisitor& v) {
        Visitor<Coupon>* v1 = dynamic_cast<Visitor<Coupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            CashFlow::accept(v);
    }

}


#endif
