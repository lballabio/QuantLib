/*
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
    \brief CMS coupon
*/

#ifndef quantlib_cms_coupon_hpp
#define quantlib_cms_coupon_hpp

#include <ql/Indexes/swapindex.hpp>
#include <ql/CashFlows/all.hpp>
#include <ql/swaptionvolstructure.hpp>

namespace QuantLib {

    //! CovexityAdjustemPricer
    /*! 
		
	*/
	class ConvexityAdjustmentPricer {
      public:
        enum Type {ConundrumByBlack,
                   ConundrumByNumericalIntegration };
		virtual ~ConvexityAdjustmentPricer() {};
		virtual Real price() const = 0;
        virtual Real rate() const = 0;
	};

    //! CMS coupon class
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */


    class CMSCoupon : public FloatingRateCoupon,
                      public Observer {
      public:
        CMSCoupon(const Real nominal,
                  const Date& paymentDate,
                  const boost::shared_ptr<SwapIndex>& index,
                  const Date& startDate, const Date& endDate,
                  Integer fixingDays,
                  const DayCounter& dayCounter,
                  ConvexityAdjustmentPricer::Type typeOfConvexityAdjustment,
                  Real gearing,
                  Rate spread,
                  Rate cap = Null<Rate>(),
                  Rate floor = Null<Rate>(),
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date());
        //! \name CashFlow interface
        //@{
        Real amount() const;
        //@}
        //! \name Coupon interface
        //@{
        Rate rate() const;
        Rate rate1() const;
        DayCounter dayCounter() const { return dayCounter_; }
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        Rate fixing() const;
        Date fixingDate() const;
        Rate indexFixing() const;
        //@}
        //! \name Inspectors
        //@{
        const boost::shared_ptr<SwapIndex>& index() const { return index_; }
        Rate cap() const { return cap_; }
        Rate floor() const { return floor_; }
        //Real multiplier() const { return multiplier_; }
        //@}
        //! \name Modifiers
        //@{
        void setSwaptionVolatility(
                const Handle<SwaptionVolatilityStructure>&);
        Handle<SwaptionVolatilityStructure> CMSCoupon::swaptionVolatility() const;
        //@}
        //! \name Observer interface
        //@{
        void update() { notifyObservers(); }
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        boost::shared_ptr<SwapIndex> index_;
        DayCounter dayCounter_;
        Rate baseRate_, cap_, floor_;
        Real multiplier_;
        Handle<SwaptionVolatilityStructure> swaptionVol_;
        ConvexityAdjustmentPricer::Type typeOfConvexityAdjustment_;
    };


    std::vector<boost::shared_ptr<CashFlow> >
    CMSCouponVector(const Schedule& schedule,
                    BusinessDayConvention paymentAdjustment,
                    const std::vector<Real>& nominals,
                    const boost::shared_ptr<SwapIndex>& index,
                    Integer fixingDays,
                    const DayCounter& dayCounter,
                    const std::vector<Real>& baseRate,
                    const std::vector<Real>& fractions,
                    const std::vector<Real>& caps,
                    const std::vector<Real>& floors,
                    const Handle<SwaptionVolatilityStructure>& vol =
                                Handle<SwaptionVolatilityStructure>(),
                    ConvexityAdjustmentPricer::Type typeOfConvexityAdjustment =
                                ConvexityAdjustmentPricer::ConundrumByBlack);

}

#endif
