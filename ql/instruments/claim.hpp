/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file claim.hpp
    \brief Classes for default-event claims.
*/

#ifndef quantlib_claim_hpp
#define quantlib_claim_hpp

#include <ql/instruments/bond.hpp>

namespace QuantLib {

    //! Claim associated to a default event
    class Claim : public Observable, public Observer {
      public:
        virtual ~Claim() {}
        virtual Real amount(const Date& defaultDate,
                            Real notional,
                            Real recoveryRate) const = 0;
        void update() { notifyObservers(); }
    };


    //! Claim on a notional
    class FaceValueClaim : public Claim {
      public:
        Real amount(const Date& d,
                    Real notional,
                    Real recoveryRate) const;
    };

    //! Claim on the notional of a reference security, including accrual
    class FaceValueAccrualClaim : public Claim {
      public:
        FaceValueAccrualClaim(
                          const boost::shared_ptr<Bond>& referenceSecurity);
        Real amount(const Date& d,
                    Real notional,
                    Real recoveryRate) const;
      private:
        boost::shared_ptr<Bond> referenceSecurity_;
    };

}


#endif
