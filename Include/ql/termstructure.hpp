
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file termstructure.hpp
    \brief Term structure

    $Id$
*/

// $Source$
// $Log$
// Revision 1.11  2001/07/02 12:36:17  sigmud
// pruned redundant header inclusions
//
// Revision 1.10  2001/06/20 11:52:29  lballabio
// Some observability is back
//
// Revision 1.9  2001/06/12 13:43:04  lballabio
// Today's date is back into term structures
// Instruments are now constructed with settlement days instead of settlement date
//
// Revision 1.8  2001/05/29 10:00:07  lballabio
// Removed check that would fail on null relinkable handle
//
// Revision 1.7  2001/05/29 09:24:06  lballabio
// Using relinkable handle to term structure
//
// Revision 1.6  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_term_structure_h
#define quantlib_term_structure_h

#include "ql/calendar.hpp"
#include "ql/rate.hpp"
#include "ql/spread.hpp"
#include "ql/discountfactor.hpp"
#include "ql/currency.hpp"
#include "ql/relinkablehandle.hpp"
#include <vector>

namespace QuantLib {

    //! Term structure
    /*! This class is purely abstract and defines the interface of concrete
        rate structures which will be derived from this one.
    */
    class TermStructure : public Patterns::Observable {
      public:
        virtual ~TermStructure() {}
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

        //! \name Dates
        //@{
        //! returns today's date
        virtual Date todaysDate() const = 0;
        //! returns the number of settlement days
        virtual int settlementDays() const = 0;
        //! returns the calendar for settlement calculation
        virtual Handle<Calendar> calendar() const = 0;
        //! returns the settlement date
        virtual Date settlementDate() const = 0;
        //! returns the earliest date for which the curve can return rates
        virtual Date minDate() const = 0;
        //! returns the latest date for which the curve can return rates
        virtual Date maxDate() const = 0;
        //@}

        //! \name Other inspectors
        //@{
        //! returns the currency upon which the term structure is defined
        virtual Currency currency() const = 0;
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
    /*! The given date will be the implied today's date.
        \note This term structure will remain linked to the original structure,
        i.e., any changes in the latter will be reflected in this structure as
        well.
    */
    class ImpliedTermStructure : public DiscountStructure,
                                 public Patterns::Observer {
      public:
        ImpliedTermStructure(const RelinkableHandle<TermStructure>&,
            const Date& todaysDate);
        ~ImpliedTermStructure();

        //! \name TermStructure interface
        //@{
        Currency currency() const;
        Date todaysDate() const;
        int settlementDays() const;
        Handle<Calendar> calendar() const;
        Date settlementDate() const;
        Date maxDate() const;
        Date minDate() const;
        //! returns the discount factor as seen from the evaluation date
        DiscountFactor discount(const Date&, bool extrapolate = false) const;
        //@}

        //! \name Observer interface
        //@{
        void update();
        //@}
      private:
        RelinkableHandle<TermStructure> originalCurve_;
        Date todaysDate_;
    };

    //! Term structure with an added spread on the zero yield rate
    /*! This term structure will remain linked to the original structure, i.e.,
        any changes in the latter will be reflected in this structure as well.
    */
    class SpreadedTermStructure : public ZeroYieldStructure,
                                  public Patterns::Observer {
      public:
        SpreadedTermStructure(const RelinkableHandle<TermStructure>&, 
            Spread spread);
        ~SpreadedTermStructure();

        //! \name TermStructure interface
        //@{
        Currency currency() const;
        Date todaysDate() const;
        int settlementDays() const;
        Handle<Calendar> calendar() const;
        Date settlementDate() const;
        Date maxDate() const;
        Date minDate() const;
        //! returns the spreaded zero yield rate
        Rate zeroYield(const Date&, bool extrapolate = false) const;
        //@}

        //! \name Observer interface
        //@{
        void update();
        //@}
      private:
        RelinkableHandle<TermStructure> originalCurve_;
        Spread spread_;
    };


    // inline definitions

    inline std::vector<Rate> TermStructure::zeroYield(
        const std::vector<Date>& x, bool extrapolate) const {
            std::vector<Rate> y(x.size());
            std::vector<Date>::const_iterator j=x.begin();
            for (std::vector<Rate>::iterator i=y.begin(); i!=y.end(); ++i,++j)
                *i = zeroYield(*j, extrapolate);
            return y;
    }

    inline std::vector<DiscountFactor> TermStructure::discount(
        const std::vector<Date>& x, bool extrapolate) const {
            std::vector<DiscountFactor> y(x.size());
            std::vector<Date>::const_iterator j=x.begin();
            for (std::vector<DiscountFactor>::iterator i=y.begin(); i!=y.end();
                                                                        ++i,++j)
                *i = discount(*j, extrapolate);
            return y;
    }

    inline std::vector<Rate> TermStructure::forward(
        const std::vector<Date>& x, bool extrapolate) const {
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

    inline DiscountFactor ForwardRateStructure::discount(const Date& d,
        bool extrapolate) const {
            Rate r = zeroYield(d, extrapolate);
            double t = double(d-settlementDate())/365;
            return DiscountFactor(QL_EXP(-r*t));
    }


    // time-shifted curve

    inline ImpliedTermStructure::ImpliedTermStructure(
        const RelinkableHandle<TermStructure>& h, const Date& todaysDate)
    : originalCurve_(h), todaysDate_(todaysDate) {
        if (!originalCurve_.isNull())
            originalCurve_.registerObserver(this);
    }

    inline ImpliedTermStructure::~ImpliedTermStructure() {
        if (!originalCurve_.isNull())
            originalCurve_.unregisterObserver(this);
    }

    inline Currency ImpliedTermStructure::currency() const {
        return originalCurve_->currency();
    }

    inline Date ImpliedTermStructure::todaysDate() const {
        return todaysDate_;
    }

    inline int ImpliedTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline Handle<Calendar> ImpliedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Date ImpliedTermStructure::settlementDate() const {
        return calendar()->advance(
            todaysDate_,settlementDays(),Days);
    }

    inline Date ImpliedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Date ImpliedTermStructure::minDate() const {
        return settlementDate();
    }

    inline DiscountFactor ImpliedTermStructure::discount(const Date& d,
        bool extrapolate) const {
            // evaluationDate cannot be an extrapolation
            /* discount at evaluation date cannot be cached
               since the original curve could change between
               invocations of this method */
            return originalCurve_->discount(d, extrapolate) /
                originalCurve_->discount(settlementDate());
    }

    inline void ImpliedTermStructure::update() {
        notifyObservers();
    }


    // spreaded curve

    inline SpreadedTermStructure::SpreadedTermStructure(
        const RelinkableHandle<TermStructure>& h, Spread spread)
    : originalCurve_(h), spread_(spread) {
        if (!originalCurve_.isNull())
            originalCurve_.registerObserver(this);
    }

    inline SpreadedTermStructure::~SpreadedTermStructure() {
        if (!originalCurve_.isNull())
            originalCurve_.unregisterObserver(this);
    }

    inline Currency SpreadedTermStructure::currency() const {
        return originalCurve_->currency();
    }

    inline Date SpreadedTermStructure::todaysDate() const {
        return originalCurve_->todaysDate();
    }

    inline int SpreadedTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline Handle<Calendar> SpreadedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Date SpreadedTermStructure::settlementDate() const {
        return originalCurve_->settlementDate();
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

    inline void SpreadedTermStructure::update() {
        notifyObservers();
    }

}


#endif
