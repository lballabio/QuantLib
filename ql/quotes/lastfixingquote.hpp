/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano

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

/*! \file lastfixingquote.hpp
    \brief quote for the last fixing available for a given index
*/

#ifndef quantlib_last_fixing_quote_hpp
#define quantlib_last_fixing_quote_hpp

#include <ql/quote.hpp>
#include <ql/index.hpp>
#include <ql/patterns/observable.hpp>

namespace QuantLib {

    //! Quote adapter for the last fixing available of a given Index
    class LastFixingQuote : public Quote,
                            public Observer {
      public:
        LastFixingQuote(ext::shared_ptr<Index> index);
        //! \name Quote interface
        //@{
        Real value() const override;
        bool isValid() const override;
        //@}
        //! \name Observer interface
        //@{
        void update() override { notifyObservers(); }
        //@}
        //! \name LastFixingQuote interface
        //@{
        const ext::shared_ptr<Index>& index() const { return index_; }
        Date referenceDate() const; 
        //@}
      protected:
        ext::shared_ptr<Index> index_;
    };

}

#endif


#ifndef id_8b205dfcf22f77cde14dfd8b4862ad75
#define id_8b205dfcf22f77cde14dfd8b4862ad75
inline bool test_8b205dfcf22f77cde14dfd8b4862ad75(const int* i) {
    return i != nullptr;
}
#endif
