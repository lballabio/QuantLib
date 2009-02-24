/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

/*! \file riskybond.hpp
    \brief Defaultable bonds
*/

#ifndef quantlib_riskybond_hpp
#define quantlib_riskybond_hpp

#include <ql/instrument.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/default.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/cashflow.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/experimental/credit/pool.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>

namespace QuantLib {
    /*! Base class for default risky bonds
      \ingroup credit
    */
    class RiskyBond : public Instrument {
    public:
        RiskyBond(std::string name,
                  Currency ccy,
                  Handle<DefaultProbabilityTermStructure> probability,
                  Real recoveryRate,
                  Handle<YieldTermStructure> yieldTS)
            : name_(name), ccy_(ccy), probability_(probability),
              recoveryRate_(recoveryRate), yieldTS_(yieldTS) {
            registerWith (yieldTS_);
            registerWith (probability_);
        }
        virtual ~RiskyBond() {}
        virtual std::vector<boost::shared_ptr<CashFlow> > cashflows() const = 0;
        std::vector<boost::shared_ptr<CashFlow> > expectedCashflows();
        virtual Real notional(Date date = Date::minDate()) const = 0;
        virtual Date effectiveDate() const = 0;
        virtual Date maturityDate() const = 0;
        Real riskfreeNPV() const;
        Real totalFutureFlows(Date date) const;
        std::string name() const;
        Currency ccy() const;
        Handle<YieldTermStructure> yieldTS() const;
        //! \name Instrument interface
        //@{
        bool isExpired() const;
        //@}
    protected:
        void setupExpired() const;
        void performCalculations() const;
    private:
        std::string name_;
        Currency ccy_;
        Handle<DefaultProbabilityTermStructure> probability_;
        Real recoveryRate_;
        Handle<YieldTermStructure> yieldTS_;
    };

    inline std::string RiskyBond::name() const {
        return name_;
    }

    inline Currency RiskyBond::ccy() const {
        return ccy_;
    }

    inline Handle<YieldTermStructure> RiskyBond::yieldTS() const {
        return yieldTS_;
    }

    /*! Default risky fixed bond
      \ingroup credit
    */
    class RiskyFixedBond : public RiskyBond {
    public:
        RiskyFixedBond(std::string name,
                       Currency ccy,
                       Handle<DefaultProbabilityTermStructure> probability,
                       Real recoveryRate,
                       Schedule schedule,
                       Real rate,
                       DayCounter dayCounter,
                       BusinessDayConvention paymentConvention,
                       std::vector<Real> notionals,
                       Handle<YieldTermStructure> yieldTS);
        std::vector<boost::shared_ptr<CashFlow> > cashflows() const;
        Real notional(Date date = Date::minDate()) const;
        Date effectiveDate() const;
        Date maturityDate() const;
    private:
        Schedule schedule_;
        Real rate_;
        DayCounter dayCounter_;
        BusinessDayConvention paymentConvention_;
        std::vector<Real> notionals_;
        std::vector<boost::shared_ptr<CashFlow> > leg_;
    };


    /*! Default risky floating bonds
      \ingroup credit
    */
    class RiskyFloatingBond : public RiskyBond {
    public:
        RiskyFloatingBond(std::string name,
                          Currency ccy,
                          Handle<DefaultProbabilityTermStructure> probability,
                          Real recoveryRate,
                          Schedule schedule,
                          boost::shared_ptr<IborIndex> index,
                          Integer fixingDays,
                          Real spread,
                          std::vector<Real> notionals,
                          Handle<YieldTermStructure> yieldTS);
        std::vector<boost::shared_ptr<CashFlow> > cashflows() const;
        Real notional(Date date = Date::minDate()) const;
        Date effectiveDate() const;
        Date maturityDate() const;
    private:
        Schedule schedule_;
        boost::shared_ptr<IborIndex> index_;
        DayCounter dayCounter_;
        Integer fixingDays_;
        Real spread_;
        BusinessDayConvention paymentConvention_;
        std::vector<Real> notionals_;
        std::vector<boost::shared_ptr<CashFlow> > leg_;
    };

}


#endif
