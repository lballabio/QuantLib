
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

/*! \file Instruments/swap.hpp
    \brief Interest rate swap
*/

#ifndef quantlib_swap_hpp
#define quantlib_swap_hpp

#include <ql/instrument.hpp>
#include <ql/termstructure.hpp>
#include <ql/cashflow.hpp>
#include <ql/CashFlows/timebasket.hpp>

namespace QuantLib {

    //! Interest rate swap
    /*! The cash flows belonging to the first leg are paid;
        the ones belonging to the second leg are received.

        \ingroup instruments
    */
    class Swap : public Instrument {
      public:
        Swap(const std::vector<boost::shared_ptr<CashFlow> >& firstLeg,
             const std::vector<boost::shared_ptr<CashFlow> >& secondLeg,
             const Handle<YieldTermStructure>& termStructure);
        //! \name Instrument interface
        //@{
        bool isExpired() const;
        //@}
        //! \name Additional interface
        //@{
        Date startDate() const;
        Date maturity() const;
        Real firstLegBPS() const;
        Real secondLegBPS() const;
        /*! \bug this method must still be checked. It is not guaranteed
                 to yield the right results.
        */
        TimeBasket sensitivity(Integer basis = 2) const;
        //@}
      protected:
        // methods
        void setupExpired() const;
        void performCalculations() const;
        // data members
        std::vector<boost::shared_ptr<CashFlow> > firstLeg_, secondLeg_;
        Handle<YieldTermStructure> termStructure_;
        mutable Real firstLegBPS_, secondLegBPS_;
    };
}


#endif
