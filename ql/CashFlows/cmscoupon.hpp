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

    //! VanillaCMSCouponPricer
    /*!

    */
    class CMSCoupon;

    class VanillaCMSCouponPricer {
      public:
        virtual ~VanillaCMSCouponPricer() {}
        virtual Real price() const = 0;
        virtual Real rate() const = 0;
        virtual void initialize(const CMSCoupon& coupon) = 0;
    };

    //! CMS coupon class
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    class CMSCoupon : public FloatingRateCoupon {
      public:
        CMSCoupon(const Real nominal,
                  const Date& paymentDate,
                  const boost::shared_ptr<SwapIndex>& index,
                  const Date& startDate, const Date& endDate,
                  Integer fixingDays,
                  const DayCounter& dayCounter,
                  const boost::shared_ptr<VanillaCMSCouponPricer>& Pricer,
                  Real gearing,
                  Rate spread,
                  Rate cap = Null<Rate>(),
                  Rate floor = Null<Rate>(),
                  Real meanReversion = 0.,
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  bool isInArrears = false);
        //! \name Coupon interface
        //@{
		Real price(const Handle<YieldTermStructure>& discountingCurve) const;
        Rate rate() const;
        // legacy code (analytical integration) to be removed later
        Rate rate1() const;
        //@}
        //! \name Inspectors
        //@{
        const boost::shared_ptr<SwapIndex>& swapIndex() const {
            return swapIndex_;
        }
        Rate cap() const { return cap_; }
        Rate floor() const { return floor_; }
        Real meanReversion() const { return meanReversion_; }
        //! fixing date
        virtual Date fixingDate() const;
        //@}
        //! \name Modifiers
        //@{
        void setSwaptionVolatility(
                const Handle<SwaptionVolatilityStructure>&);
        Handle<SwaptionVolatilityStructure> swaptionVolatility() const;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        Rate convexityAdjustmentImpl(Rate f) const {
            return (gearing() == 0.0 ? 0.0 : (rate()-spread())/gearing() - f);
        }
        boost::shared_ptr<SwapIndex> swapIndex_;
        Rate cap_, floor_;
        bool isInArrears_;
        Real meanReversion_;
        Handle<SwaptionVolatilityStructure> swaptionVol_;
        boost::shared_ptr<VanillaCMSCouponPricer> Pricer_;
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
                    const std::vector<Real>& meanReversions,
                    const boost::shared_ptr<VanillaCMSCouponPricer>& pricer,
                    const Handle<SwaptionVolatilityStructure>& vol =
                                Handle<SwaptionVolatilityStructure>());

    std::vector<boost::shared_ptr<CashFlow> >
    CMSZeroCouponVector(const Schedule& schedule,
                    BusinessDayConvention paymentAdjustment,
                    const std::vector<Real>& nominals,
                    const boost::shared_ptr<SwapIndex>& index,
                    Integer fixingDays,
                    const DayCounter& dayCounter,
                    const std::vector<Real>& baseRate,
                    const std::vector<Real>& fractions,
                    const std::vector<Real>& caps,
                    const std::vector<Real>& floors,
                    const std::vector<Real>& meanReversions,
                    const boost::shared_ptr<VanillaCMSCouponPricer>& pricer,
                    const Handle<SwaptionVolatilityStructure>& vol =
                                Handle<SwaptionVolatilityStructure>());

    std::vector<boost::shared_ptr<CashFlow> >
    CMSInArrearsCouponVector(const Schedule& schedule,
                    BusinessDayConvention paymentAdjustment,
                    const std::vector<Real>& nominals,
                    const boost::shared_ptr<SwapIndex>& index,
                    Integer fixingDays,
                    const DayCounter& dayCounter,
                    const std::vector<Real>& baseRate,
                    const std::vector<Real>& fractions,
                    const std::vector<Real>& caps,
                    const std::vector<Real>& floors,
                    const std::vector<Real>& meanReversions,
                    const boost::shared_ptr<VanillaCMSCouponPricer>& pricer,
                    const Handle<SwaptionVolatilityStructure>& vol =
                                Handle<SwaptionVolatilityStructure>());

}

#endif
