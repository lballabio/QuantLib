/*
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2004 M-Dimension Consulting Inc.
 Copyright (C) 2005 StatPro Italia srl

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

/*! \file bond.hpp
    \brief concrete bond class
*/

#ifndef quantlib_bond_hpp
#define quantlib_bond_hpp

#include <ql/instrument.hpp>
#include <ql/cashflow.hpp>
#include <ql/calendar.hpp>
#include <ql/daycounter.hpp>
#include <ql/interestrate.hpp>
#include <ql/yieldtermstructure.hpp>
#include <vector>

namespace QuantLib {

    //! Base bond class
    /*! Derived classes must fill the unitialized data members.

        \warning Most methods assume that the cashflows are stored
                 sorted by date

        \ingroup instruments

        \test
        - price/yield calculations are cross-checked for consistency.
        - price/yield calculations are checked against known good
          values.
    */
    class Bond : public Instrument {
      protected:
        Bond(const DayCounter& dayCount,
             const Calendar& calendar,
             BusinessDayConvention businessDayConvention,
             Integer settlementDays,
             const Handle<YieldTermStructure>& discountCurve
                                              = Handle<YieldTermStructure>());
      public:
        //! \name Inspectors
        //@{
        Date settlementDate() const;
        const std::vector<boost::shared_ptr<CashFlow> >& cashflows() const;
        const boost::shared_ptr<CashFlow>& redemption() const;
        const Calendar& calendar() const;
        BusinessDayConvention businessDayConvention() const;
        const DayCounter& dayCounter() const;
        Frequency frequency() const;
        boost::shared_ptr<YieldTermStructure> discountCurve() const;
        //@}
        //! \name Calculations
        //@{
        //! theoretical clean price
        /*! The default bond settlement is used for calculation. */
        Real cleanPrice() const;
        //! theoretical dirty price
        /*! The default bond settlement is used for calculation. */
        Real dirtyPrice() const;
        //! theoretical bond yield
        /*! The default bond settlement and theoretical price are used
            for calculation.
        */
        Real yield(Compounding compounding,
                   Real accuracy = 1.0e-8,
                   Size maxEvaluations = 100) const;
        //! clean price given a yield and settlement date
        /*! The default bond settlement is used if no date is given. */
        Real cleanPrice(Rate yield,
                        Compounding compounding,
                        Date settlementDate = Date()) const;
        //! dirty price given a yield and settlement date
        /*! The default bond settlement is used if no date is given. */
        Real dirtyPrice(Rate yield,
                        Compounding compounding,
                        Date settlementDate = Date()) const;
        //! yield given a (clean) price and settlement date
        /*! The default bond settlement is used if no date is given. */
        Real yield(Real cleanPrice,
                   Compounding compounding,
                   Date settlementDate = Date(),
                   Real accuracy = 1.0e-8,
                   Size maxEvaluations = 100) const;
        //! accrued amount at a given date
        /*! The default bond settlement is used if no date is given. */
        Real accruedAmount(Date d = Date()) const;
        bool isExpired() const;
        //@}
      protected:
        void performCalculations() const;
        Integer settlementDays_;
        Calendar calendar_;
        BusinessDayConvention businessDayConvention_;
		DayCounter dayCount_;

        Date issueDate_, datedDate_, maturityDate_;
        Frequency frequency_;
        std::vector<boost::shared_ptr<CashFlow> > cashFlows_;
        boost::shared_ptr<CashFlow> redemption_;
        Handle<YieldTermStructure> discountCurve_;
    };


    // inline definitions

    inline
    const std::vector<boost::shared_ptr<CashFlow> >& Bond::cashflows() const {
        return cashFlows_;
    }

    inline
    const boost::shared_ptr<CashFlow>& Bond::redemption() const {
        return redemption_;
    }

    inline const Calendar& Bond::calendar() const {
        return calendar_;
    }

    inline BusinessDayConvention Bond::businessDayConvention() const {
        return businessDayConvention_;
    }

    inline Frequency Bond::frequency() const {
        return frequency_;
    }

    inline boost::shared_ptr<YieldTermStructure> Bond::discountCurve() const {
        return discountCurve_.currentLink();
    }

}


#endif
