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
#include <ql/experimental/credit/issuer.hpp>
#include <ql/default.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/cashflow.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/experimental/credit/pool.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/currency.hpp>

namespace QuantLib {

    /*! Base class for default risky bonds
      \ingroup credit
    */
    class RiskyBond : public Instrument {
    public:
        /*! The value is contingent to survival, i.e., the knockout
            probability is considered.  To compute the npv given that
            the issuer has survived, divide the npv by
            \f[(1-P_{def}(T_{npv}))\f]
        */
        RiskyBond(const std::string& name,
                  const Currency& ccy,
                  Real recoveryRate,
                  const Handle<DefaultProbabilityTermStructure>& defaultTS,
                  const Handle<YieldTermStructure>& yieldTS,
                  Natural settlementDays = 0,
                  Calendar calendar = Calendar());
        virtual ~RiskyBond() {}
        virtual std::vector<ext::shared_ptr<CashFlow> > cashflows() const = 0;
        std::vector<ext::shared_ptr<CashFlow> > expectedCashflows();
        virtual Real notional(Date date = Date::minDate()) const = 0;
        virtual Date effectiveDate() const = 0;
        virtual Date maturityDate() const = 0;
        virtual std::vector<ext::shared_ptr<CashFlow> > interestFlows() const = 0;
        virtual std::vector<ext::shared_ptr<CashFlow> > notionalFlows() const = 0;
        Real riskfreeNPV() const;
        Real totalFutureFlows() const;
        std::string name() const;
        Currency ccy() const;
        Handle<YieldTermStructure> yieldTS() const;
        Handle<DefaultProbabilityTermStructure> defaultTS() const;
        Real recoveryRate() const;
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
        Real recoveryRate_;
        Handle<DefaultProbabilityTermStructure> defaultTS_;
        Handle<YieldTermStructure> yieldTS_;
    protected:
        // engines data
        Natural settlementDays_;
        Calendar calendar_;
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

    inline Handle<DefaultProbabilityTermStructure>
    RiskyBond::defaultTS() const {
        return defaultTS_;
    }

    inline Real RiskyBond::recoveryRate() const {
        return recoveryRate_;
    }

    /*! Default risky fixed bond
      \ingroup credit
    */
    class RiskyFixedBond : public RiskyBond {
    public:
        RiskyFixedBond(const std::string& name,
                       const Currency& ccy,
                       Real recoveryRate,
                       const Handle<DefaultProbabilityTermStructure>& defaultTS,
                       const Schedule& schedule, 
                       Real rate,
                       const DayCounter& dayCounter,
                       BusinessDayConvention paymentConvention,
                       const std::vector<Real>& notionals,
                       const Handle<YieldTermStructure>& yieldTS,
                       Natural settlementDays = 0);
        std::vector<ext::shared_ptr<CashFlow> > cashflows() const;
        Real notional(Date date = Date::minDate()) const;
        Date effectiveDate() const;
        Date maturityDate() const;
        std::vector<ext::shared_ptr<CashFlow> > interestFlows() const;
        std::vector<ext::shared_ptr<CashFlow> > notionalFlows() const;
    private:
        Schedule schedule_;
        Real rate_;
        DayCounter dayCounter_;
        // BusinessDayConvention paymentConvention_;
        std::vector<Real> notionals_;
        std::vector<ext::shared_ptr<CashFlow> > leg_;
        std::vector<ext::shared_ptr<CashFlow> > interestLeg_;
        std::vector<ext::shared_ptr<CashFlow> > redemptionLeg_;
    };


    /*! Default risky floating bonds
      \ingroup credit
    */
    class RiskyFloatingBond : public RiskyBond {
    public:
        RiskyFloatingBond(std::string name,
                          Currency ccy,
                          Real recoveryRate,
                          Handle<DefaultProbabilityTermStructure> defaultTS,
                          Schedule schedule,
                          ext::shared_ptr<IborIndex> index,
                          Integer fixingDays,
                          Real spread,
                          std::vector<Real> notionals,
                          Handle<YieldTermStructure> yieldTS,
                          Natural settlementDays = 0);
        std::vector<ext::shared_ptr<CashFlow> > cashflows() const;
        Real notional(Date date = Date::minDate()) const;
        Date effectiveDate() const;
        Date maturityDate() const;
        std::vector<ext::shared_ptr<CashFlow> > interestFlows() const;
        std::vector<ext::shared_ptr<CashFlow> > notionalFlows() const;
    private:
        Schedule schedule_;
        ext::shared_ptr<IborIndex> index_;
        DayCounter dayCounter_;
        Integer fixingDays_;
        Real spread_;
        // BusinessDayConvention paymentConvention_;
        std::vector<Real> notionals_;
        std::vector<ext::shared_ptr<CashFlow> > leg_;
        std::vector<ext::shared_ptr<CashFlow> > interestLeg_;
        std::vector<ext::shared_ptr<CashFlow> > redemptionLeg_;
    };

}


#endif
