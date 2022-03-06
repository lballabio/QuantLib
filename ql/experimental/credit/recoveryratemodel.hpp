/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Jose Aparicio

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

#ifndef quantlib_recovery_rate_model_hpp
#define quantlib_recovery_rate_model_hpp

#include <ql/settings.hpp>
#include <ql/handle.hpp>
#include <ql/experimental/credit/defaultprobabilitykey.hpp>
#include <ql/experimental/credit/recoveryratequote.hpp>

namespace QuantLib {

    /*! Models of the recovery rate provide future values of a recovery
        rate in the event of a default.
    */
    class RecoveryRateModel : public virtual Observable {
      public:
        /*! returns the expected recovery rate at a future time conditional
            on some default event type and seniority.
        */
        virtual Real recoveryValue(const Date& defaultDate,
            const DefaultProbKey& defaultKey = DefaultProbKey()) const {
            // no check on dates...
            return recoveryValueImpl(defaultDate, defaultKey);
        }
        /*! Returns true if the model will return recovery rates for
            the requested seniority.
        */
        virtual bool appliesToSeniority(Seniority) const = 0;
        ~RecoveryRateModel() override = default;

      protected:
        /*! Returns Null<Real> if unable to produce a recovery for
            the requested seniority.
        */
        virtual Real recoveryValueImpl(const Date&,
                                       const DefaultProbKey& defaultKey
                                       ) const = 0;
    };


    /*! Simple Recovery Rate model returning the constant value of the quote
        independently of the date and the seniority.
    */
    class ConstantRecoveryModel : public RecoveryRateModel,
                                  public Observer {
      public:
        explicit ConstantRecoveryModel(const Handle<RecoveryRateQuote>& quote);
        explicit ConstantRecoveryModel(Real recovery,
                                       Seniority sen = NoSeniority);
        void update() override { notifyObservers(); }
        bool appliesToSeniority(Seniority) const override { return true; }

      protected:
        /*! Notice the quote's value is returned without a
            check on a match of the seniorties of the
            quote and the request.
        */
        Real recoveryValueImpl(const Date&, const DefaultProbKey&) const override {
            // no match on requested seniority, all pass
            return quote_->value();
        }

      private:
        Handle<RecoveryRateQuote> quote_;
    };

}

#endif


#ifndef id_6559ab722af8dc8aa7d8dce97b9a413d
#define id_6559ab722af8dc8aa7d8dce97b9a413d
inline bool test_6559ab722af8dc8aa7d8dce97b9a413d(int* i) { return i != 0; }
#endif
