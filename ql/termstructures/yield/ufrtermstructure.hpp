/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Marcin Rybacki

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

/*! \file ufrtermstructure.hpp
    \brief Ultimate Forward Rate term structure
*/

#ifndef quantlib_ufr_term_structure_hpp
#define quantlib_ufr_term_structure_hpp

#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Ultimate Forward Rate term structure

    /*! Dutch regulatory term structure for pension funds with a
        parametrized extrapolation mechanism designed for
        discounting long date liabilities.

        Relevant documentation can be found on the Dutch Central
        Bank website:

        FTK term structure documentation (Financieel toetsingskader):
        https://www.toezicht.dnb.nl/binaries/50-212329.pdf

        UFR 2015 term structure documentation:
        https://www.toezicht.dnb.nl/binaries/50-234028.pdf

        UFR 2019 term structure documentation:
        https://www.rijksoverheid.nl/documenten/kamerstukken/2019/06/11/advies-commissie-parameters

        This term structure will remain linked to the original
        structure, i.e., any changes in the latter will be
        reflected in this structure as well.

        \ingroup yieldtermstructures

        \test
        - tbd
    */

   class UFRTermStructure : public YieldTermStructure {
      public:
        ImpliedTermStructure(const Handle<YieldTermStructure>&,
                             const Handle<Quote>& llfr,
                             const Handle<Quote>& ufr,);
        //! \name YieldTermStructure interface
        //@{
        DayCounter dayCounter() const;
        Calendar calendar() const;
        Natural settlementDays() const;
        Date maxDate() const;
      protected:
        DiscountFactor discountImpl(Time) const;
        //@}
      private:
        Handle<YieldTermStructure> originalCurve_;
    };
}

#endif