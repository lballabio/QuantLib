
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

/*! \file swap.hpp
    \brief Interest rate swap
*/

#ifndef quantlib_swap_h
#define quantlib_swap_h

#include <ql/instrument.hpp>
#include <ql/termstructure.hpp>
#include <ql/cashflow.hpp>
#include <ql/CashFlows/timebasket.hpp>

namespace QuantLib {

    //! Interest rate swap
    /*! The cash flows belonging to the first leg are payed;
        the ones belonging to the first leg are received. 
    */
    class Swap : public Instrument {
      public:
        Swap(const std::vector<Handle<CashFlow> >& firstLeg,
             const std::vector<Handle<CashFlow> >& secondLeg,
             const RelinkableHandle<TermStructure>& termStructure);
        //! \name Instrument interface
        //@{
        bool isExpired() const;
        //@}
        //! \name Additional interface
        //@{
        Date startDate() const;
        Date maturity() const;
        double firstLegBPS() const;
        double secondLegBPS() const;
        /*! \bug This method must still be checked. It is not guaranteed
                 to yield the right results.
        */
        TimeBasket sensitivity(int basis = 2) const;
        //@}
      protected:
        // methods
        void setupExpired() const;
        void performCalculations() const;
        // data members
        std::vector<Handle<CashFlow> > firstLeg_, secondLeg_;
        RelinkableHandle<TermStructure> termStructure_;
        mutable double firstLegBPS_, secondLegBPS_;
    };
}


#endif
