/*
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 StatPro Italia srl


 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file cmscoupon.hpp
    \brief Cms coupon
*/

#ifndef quantlib_cms_coupon_hpp
#define quantlib_cms_coupon_hpp

#include <ql/CashFlows/iborcoupon.hpp>
#include <ql/Indexes/swapindex.hpp>
#include <ql/swaptionvolstructure.hpp>

namespace QuantLib {

    //! Cms coupon class
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    class CmsCoupon : public FloatingRateCoupon {
      public:
        CmsCoupon(const Date& paymentDate,
                  const Real nominal,
                  const Date& startDate, 
                  const Date& endDate,
                  const Integer fixingDays,
                  const boost::shared_ptr<SwapIndex>& index,
                  const Real gearing = 1.0,
                  const Spread spread= 0.0,
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const DayCounter& dayCounter = DayCounter(),
                  bool isInArrears = false);

        //! \name Inspectors
        //@{
        const boost::shared_ptr<SwapIndex>& swapIndex() const {
            return swapIndex_;
        } //da eliminare dovrebbere convergere nel metodo index di float

        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        boost::shared_ptr<SwapIndex> swapIndex_;
    };

}

#endif
