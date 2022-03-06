/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file quote.hpp
    \brief purely virtual base class for market observables
*/

#ifndef quantlib_quote_hpp
#define quantlib_quote_hpp

#include <ql/handle.hpp>
#include <ql/errors.hpp>
#include <ql/utilities/null.hpp>

namespace QuantLib {

    //! purely virtual base class for market observables
    /*! \test the observability of class instances is tested.
     */
    class Quote : public virtual Observable {
      public:
        ~Quote() override = default;
        //! returns the current value
        virtual Real value() const = 0;
        //! returns true if the Quote holds a valid value
        virtual bool isValid() const = 0;
    };

}

#endif


#ifndef id_218293248ea43a60927930ee4736ea57
#define id_218293248ea43a60927930ee4736ea57
inline bool test_218293248ea43a60927930ee4736ea57(int* i) { return i != 0; }
#endif
