

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file floatingratecoupon.hpp
    \brief Coupon at par on a term structure

    \fullpath
    ql/CashFlows/%floatingratecoupon.hpp
*/

// $Id$

#ifndef quantlib_floating_rate_coupon_hpp
#define quantlib_floating_rate_coupon_hpp

#include <ql/CashFlows/coupon.hpp>
#include <ql/Indexes/xibor.hpp>

namespace QuantLib {

    namespace CashFlows {

        //! %coupon at par on a term structure
        /*! \warning This class does not perform any date adjustment,
            i.e., the start and end date passed upon construction
            should be already rolled to a business day.
        */
        class FloatingRateCoupon : public Coupon,
                                   public Patterns::Observer {
          public:
            FloatingRateCoupon(double nominal,
                const Handle<Indexes::Xibor>& index,
                const RelinkableHandle<TermStructure>& termStructure,
                const Date& startDate, const Date& endDate,
                int fixingDays,
                Spread spread = 0.0,
                const Date& refPeriodStart = Date(),
                const Date& refPeriodEnd = Date());
            //! \name CashFlow interface
            //@{
            double amount() const;
            //@}
            //! \name Coupon interface
            //@{
            double accruedAmount(const Date&) const;
            //@}
            //! \name Inspectors
            //@{
            const Handle<Indexes::Xibor>& index() const;
            int fixingDays() const;
            Rate fixing() const;
            Spread spread() const;
            //@}
            //! \name Observer interface
            //@{
            void update();
            //@}
          private:
            RelinkableHandle<TermStructure> termStructure_;
            Handle<Indexes::Xibor> index_;
            int fixingDays_;
            Spread spread_;
        };


        // inline definitions

        inline const Handle<Indexes::Xibor>&
        FloatingRateCoupon::index() const {
            return index_;
        }

        inline int FloatingRateCoupon::fixingDays() const {
            return fixingDays_;
        }

        inline Rate FloatingRateCoupon::fixing() const {
            return amount()/(nominal()*accrualPeriod());
        }

        inline Spread FloatingRateCoupon::spread() const {
            return spread_;
        }

        inline void FloatingRateCoupon::update() {
            notifyObservers();
        }

        inline double FloatingRateCoupon::accruedAmount(const Date& d) const {
            if (d <= startDate_ || d >= endDate_) {
                return 0.0;
            } else {
                return nominal()*fixing()*
                    dayCounter_.yearFraction(startDate_,d,
                        refPeriodStart_,refPeriodEnd_);
            }
        }
        
    }

}


#endif
