
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file termstructure.h
    \brief Term structure

    $Source$
    $Name$
    $Log$
    Revision 1.12  2001/01/18 14:36:13  nando
    80 columns enforced
    private members with trailing underscore

    Revision 1.11  2001/01/18 13:18:11  nando
    now term structure allows extrapolation

    Revision 1.10  2001/01/17 14:37:55  nando
    tabs removed

    Revision 1.9  2001/01/09 12:08:42  lballabio
    Cleaned up style in a few files

    Revision 1.8  2000/12/14 12:32:29  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_term_structure_h
#define quantlib_term_structure_h

#include "qldefines.h"
#include "date.h"
#include "calendar.h"
#include "rate.h"
#include "spread.h"
#include "discountfactor.h"
#include "currency.h"
#include "handle.h"
#include "observable.h"
#include <vector>

namespace QuantLib {

    //! Term structure
    /*! This class is purely abstract and defines the interface of concrete
        rate structures which will be derived from this one.
    */
    class TermStructure : public Patterns::Observable {
      public:
        virtual ~TermStructure() {}
        //! returns a copy of this structure with no observers registered
        virtual Handle<TermStructure> clone() const = 0;
        //! \name Rates
        //@{
        //! zero yield rate for a given date
        virtual Rate zeroYield(const Date&,
                               bool extrapolate = false) const = 0;
        //! zero yield rate for a given set of dates
        std::vector<Rate> zeroYield(const std::vector<Date>&,
                                    bool extrapolate = false) const;
        //! discount factor for a given date
        virtual DiscountFactor discount(const Date&,
                                        bool extrapolate = false) const = 0;
        //! discount factor for a given set of dates
        std::vector<DiscountFactor> discount(const std::vector<Date>&,
                                             bool extrapolate = false) const;
        //! instantaneous forward rate for a given date
        virtual Rate forward(const Date&,
                             bool extrapolate = false) const = 0;
        //! instantaneous forward rate for a given set of dates
        std::vector<Rate> forward(const std::vector<Date>&,
                                  bool extrapolate = false) const;
        //@}

        //! \name Other inspectors
        //@{
        //! returns the currency upon which the term structure is defined
        virtual Handle<Currency> currency() const = 0;
        //! returns the calendar upon which the term structure is defined
        virtual Handle<Calendar> calendar() const = 0;
        //! returns the date at which the structure is defined
        virtual Date todaysDate() const = 0;
        //! returns the settlement date relative to today's date
        virtual Date settlementDate() const = 0;
        //! returns the earliest date for which the curve can return rates
        virtual Date minDate() const = 0;
        //! returns the latest date for which the curve can return rates
        virtual Date maxDate() const = 0;
        //@}
    };

    //! Zero yield term structure
    /*! This abstract class acts as an adapter to TermStructure allowing the
        programmer to implement only the <tt>zeroYield(const Date&)</tt> method
        in derived classes.
    */
    class ZeroYieldStructure : public TermStructure {
      public:
        virtual ~ZeroYieldStructure() {}
        /*! returns the discount factor for the given date calculating it from
            the zero yield.
        */
        DiscountFactor discount(const Date&, bool extrapolate = false) const;
        /*! returns the instantaneous forward rate for the given date
            calculating it from the zero yield.
        */
        Rate forward(const Date&, bool extrapolate = false) const;
    };

    //! Discount factor term structure
    /*! This abstract class acts as an adapter to TermStructure allowing the
        programmer to implement only the <tt>discount(const Date&)</tt> method
        in derived classes.
    */
    class DiscountStructure : public TermStructure {
      public:
        virtual ~DiscountStructure() {}
        /*! returns the zero yield rate for the given date calculating it from
            the discount.
        */
        Rate zeroYield(const Date&, bool extrapolate = false) const;
        /*! returns the instantaneous forward rate for the given date
            calculating it from the discount.
        */
        Rate forward(const Date&, bool extrapolate = false) const;
    };

    //! Forward rate term structure
    /*! This abstract class acts as an adapter to TermStructure allowing the
        programmer to implement only the <tt>forward(const Date&)</tt> method
        in derived classes.
    */
    class ForwardRateStructure : public TermStructure {
      public:
        virtual ~ForwardRateStructure() {}
        /*! returns the zero yield rate for the given date calculating it from
            the instantaneous forward rate.
        */
        Rate zeroYield(const Date&, bool extrapolate = false) const;
        /*! returns the discount factor for the given date calculating it from
            the instantaneous forward rate.
        */
        DiscountFactor discount(const Date&, bool extrapolate = false) const;
    };

    //! Implied term structure at a given date in the future
    /*! This term structure will remain linked to the original structure, i.e.,
        any changes in the latter will be reflected in this structure as well.
    */
    class ImpliedTermStructure : public DiscountStructure {
      public:
        ImpliedTermStructure(const Handle<TermStructure>&,
            const Date& evaluationDate);
        Handle<TermStructure> clone() const;
        Handle<Currency> currency() const;
        Date todaysDate() const;
        Date settlementDate() const;
        Handle<Calendar> calendar() const;
        Date maxDate() const;
        Date minDate() const;
        //! returns the discount factor as seen from the evaluation date
        DiscountFactor discount(const Date&,
                                bool extrapolate = false) const;
        //! registers with the original structure as well
        void registerObserver(Patterns::Observer*);
        //! unregisters with the original structure as well
        void unregisterObserver(Patterns::Observer*);
        //! unregisters with the original structure as well
        void unregisterAll();
      private:
        Handle<TermStructure> originalCurve_;
        Date evaluationDate_;
    };

    //! Term structure with an added spread on the zero yield rate
    /*! This term structure will remain linked to the original structure, i.e.,
        any changes in the latter will be reflected in this structure as well.
    */
    class SpreadedTermStructure : public ZeroYieldStructure {
      public:
        SpreadedTermStructure(const Handle<TermStructure>&, Spread spread);
        Handle<TermStructure> clone() const;
        Handle<Currency> currency() const;
        Date todaysDate() const;
        Date settlementDate() const;
        Handle<Calendar> calendar() const;
        Date maxDate() const;
        Date minDate() const;
        //! returns the spreaded zero yield rate
        Rate zeroYield(const Date&, bool extrapolate = false) const;
        //! registers with the original structure as well
        void registerObserver(Patterns::Observer*);
        //! unregisters with the original structure as well
        void unregisterObserver(Patterns::Observer*);
        //! unregisters with the original structure as well
        void unregisterAll();
      private:
        Handle<TermStructure> originalCurve_;
        Spread spread_;
    };


    // inline definitions

    inline std::vector<Rate> TermStructure::zeroYield(
                              const std::vector<Date>& x,
                              bool extrapolate) const {
        std::vector<Rate> y(x.size());
        std::vector<Date>::const_iterator j=x.begin();
        for (std::vector<Rate>::iterator i=y.begin(); i!=y.end(); ++i,++j)
            *i = zeroYield(*j, extrapolate);
        return y;
    }

    inline std::vector<DiscountFactor> TermStructure::discount(
                                            const std::vector<Date>& x,
                                            bool extrapolate) const {
        std::vector<DiscountFactor> y(x.size());
        std::vector<Date>::const_iterator j=x.begin();
        for (std::vector<DiscountFactor>::iterator i=y.begin(); i!=y.end();
                                                                    ++i,++j)
            *i = discount(*j, extrapolate);
        return y;
    }

    inline std::vector<Rate> TermStructure::forward(const std::vector<Date>& x,
                                                    bool extrapolate) const {
        std::vector<Rate> y(x.size());
        std::vector<Date>::const_iterator j=x.begin();
        for (std::vector<Rate>::iterator i=y.begin(); i!=y.end(); ++i,++j)
            *i = forward(*j, extrapolate);
        return y;
    }


    // curve deriving discount and forward from zero yield

    inline DiscountFactor ZeroYieldStructure::discount(const Date& d,
                                                       bool extrapolate) const {
        Rate r = zeroYield(d, extrapolate);
        double t = double(d-settlementDate())/365;
        return DiscountFactor(QL_EXP(-r*t));
    }

    inline Rate ZeroYieldStructure::forward(const Date& d,
                                            bool extrapolate) const {
        Rate r1 = zeroYield(d, extrapolate), r2 = zeroYield(d+1, true);
        // r1+t*(r2-r1)/dt = r1+(days/365)*(r2-r1)/(1 day/365)
        return r1+(d-settlementDate())*double(r2-r1);
    }


    // curve deriving zero yield and forward from discount

    inline Rate DiscountStructure::zeroYield(const Date& d,
                                             bool extrapolate) const {
        DiscountFactor f = discount(d, extrapolate);
        double t = double(d-settlementDate())/365;
        return Rate(-QL_LOG(f)/t);
    }

    inline Rate DiscountStructure::forward(const Date& d,
                                           bool extrapolate) const {
        DiscountFactor f1 = discount(d, extrapolate),
                       f2 = discount(d+1, true);
        // log(f1/f2)/dt = log(f1/f2)/(1/365)
        return Rate(QL_LOG(f1/f2)*365);
    }


    // curve deriving zero yield and discount from forward

    inline Rate ForwardRateStructure::zeroYield(const Date& d,
                                                bool extrapolate) const {
        // This is just a default, highly inefficient implementation.
        // Derived classes should implement their own zeroYield method.
        if (d == settlementDate())
            return forward(settlementDate());
        double sum = 0.5*forward(settlementDate());
        for (Date i=settlementDate()+1; i<d; i++)
            sum += forward(i, extrapolate);
        sum += 0.5*forward(d, extrapolate);
        return Rate(sum/(d-settlementDate()));
    }

    inline DiscountFactor ForwardRateStructure::discount(
                                                    const Date& d,
                                                    bool extrapolate) const {
        Rate r = zeroYield(d, extrapolate);
        double t = double(d-settlementDate())/365;
        return DiscountFactor(QL_EXP(-r*t));
    }


    // time-shifted curve

    inline ImpliedTermStructure::ImpliedTermStructure(
        const Handle<TermStructure>& h, const Date& evaluationDate)
    : originalCurve_(h), evaluationDate_(evaluationDate) {

        QL_REQUIRE(evaluationDate<=originalCurve_->maxDate(),
            "ImpliedTermStructure::ImpliedTermStructure : "
            "the evaluation date "
            "can't be greater than the original curve max date");

    }

    inline Handle<Currency> ImpliedTermStructure::currency() const {
        return originalCurve_->currency();
    }

    inline Date ImpliedTermStructure::todaysDate() const {
        return evaluationDate_;
    }

    inline Date ImpliedTermStructure::settlementDate() const {
        return originalCurve_->currency()->settlementDate(evaluationDate_);
    }

    inline Handle<Calendar> ImpliedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Date ImpliedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Date ImpliedTermStructure::minDate() const {
        return settlementDate();
    }

    inline DiscountFactor ImpliedTermStructure::discount(
                                                    const Date& d,
                                                    bool extrapolate) const {
        // evaluationDate cannot be an extrapolation
        return originalCurve_->discount(d, extrapolate) /
            originalCurve_->discount(evaluationDate_, false);
    }

    inline Handle<TermStructure> ImpliedTermStructure::clone() const {
        return Handle<TermStructure>(new ImpliedTermStructure(
            originalCurve_->clone(),evaluationDate_));
    }

    inline void ImpliedTermStructure::registerObserver(Patterns::Observer* o) {
        TermStructure::registerObserver(o);
        originalCurve_->registerObserver(o);
    }

    inline void ImpliedTermStructure::unregisterObserver(
      Patterns::Observer* o) {
        TermStructure::unregisterObserver(o);
        originalCurve_->unregisterObserver(o);
    }

    inline void ImpliedTermStructure::unregisterAll() {
        for (std::set<Patterns::Observer*>::iterator i = observers().begin();
          i!=observers().end(); ++i)
            originalCurve_->unregisterObserver(*i);
        TermStructure::unregisterAll();
    }


    // spreaded curve

    inline SpreadedTermStructure::SpreadedTermStructure(
        const Handle<TermStructure>& h, Spread spread)
    : originalCurve_(h), spread_(spread) {}

    inline Handle<Currency> SpreadedTermStructure::currency() const {
        return originalCurve_->currency();
    }

    inline Date SpreadedTermStructure::todaysDate() const {
        return originalCurve_->todaysDate();
    }

    inline Date SpreadedTermStructure::settlementDate() const {
        return originalCurve_->settlementDate();
    }

    inline Handle<Calendar> SpreadedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Date SpreadedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Date SpreadedTermStructure::minDate() const {
        return originalCurve_->minDate();
    }

    inline Rate SpreadedTermStructure::zeroYield(const Date& d,
                                                 bool extrapolate) const {
        return originalCurve_->zeroYield(d, extrapolate)+spread_;
    }

    inline Handle<TermStructure> SpreadedTermStructure::clone() const {
        return Handle<TermStructure>(new SpreadedTermStructure(
            originalCurve_->clone(),spread_));
    }

    inline void SpreadedTermStructure::registerObserver(Patterns::Observer* o) {
        TermStructure::registerObserver(o);
        originalCurve_->registerObserver(o);
    }

    inline void SpreadedTermStructure::unregisterObserver(
      Patterns::Observer* o) {
        TermStructure::unregisterObserver(o);
        originalCurve_->unregisterObserver(o);
    }

    inline void SpreadedTermStructure::unregisterAll() {
        for (std::set<Patterns::Observer*>::iterator i = observers().begin();
          i!=observers().end(); ++i)
            originalCurve_->unregisterObserver(*i);
        TermStructure::unregisterAll();
    }

}

#endif
