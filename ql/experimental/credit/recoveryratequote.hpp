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

#ifndef quantlib_recoveryrate_quote_hpp
#define quantlib_recoveryrate_quote_hpp

#include <ql/quote.hpp>
#include <ql/experimental/credit/defaulttype.hpp>
#include <map>

namespace QuantLib {

    //! Stores a recovery rate market quote and the associated seniority.
    class RecoveryRateQuote : public Quote {
        friend std::map<Seniority, Real> makeIsdaConvMap();
    public:
        /*! Returns a map with the ISDA conventional (values by
            default) of the recovery rate per each ISDA seniority.
        */
        static Real conventionalRecovery(Seniority sen) {
            return IsdaConvRecoveries[sen];
        }
        RecoveryRateQuote(Real value = Null<Real>(),
                          Seniority seniority = NoSeniority);
        //! \name Quote interface
        //@{
        Real value() const;
        Seniority seniority() const;
        bool isValid() const;
        //@}
        //! \name Modifiers
        //@{
        //! returns the difference between the new value and the old value
        Real setValue(Real value = Null<Real>());
        void reset();
        //@}

        /*! Turn a set of recoveries into a seniority-recovery map
            (intended to be used in an event construction)
        */
        // member? move to friend?
        template <Size N>
        static std::map<Seniority, Real> makeIsdaMap(const Real (&(arrayIsdaRR))[N]);

      private:
        // Conventional recoveries for ISDA seniorities
        static const Real IsdaConvRecoveries[];
        // The seniority this recovery is quoted for.
        Seniority seniority_;
        // The recovery value. In fractional units.
        Real recoveryRate_;
    };

    inline Seniority RecoveryRateQuote::seniority() const {
        return seniority_;
    }

    inline Real RecoveryRateQuote::value() const {
        QL_ENSURE(isValid(), "invalid Recovery Quote");
        return recoveryRate_;
    }

    inline bool RecoveryRateQuote::isValid() const {
        // not to be consufed with proper initialization [0-1]
        return recoveryRate_!=Null<Real>();/* &&
            seniority_ != NoSeniority;*/
    }

    //! Helper function for conventional recoveries. Returns the ISDA
    //    conventional recovery rates for the ISDA seniorities.
    std::map<Seniority, Real> makeIsdaConvMap();


    // template definitions

    // helpers allow further automatic inclusion of seniorities
    template <Size N>
    std::map<Seniority, Real> RecoveryRateQuote::makeIsdaMap(const Real (&(arrayIsdaRR))[N]) {
        // TO DO: include check on sizes... not to go beyond enum sizes.
        // TO DO: check Reals are valid, i.e. non Null and within [0-1] range
        std::map<Seniority, Real> isdaMap;
        for(Size i=0; i<N; i++) {
            Seniority isdaType = Seniority(i);//compiler dependent?
            isdaMap[isdaType] = arrayIsdaRR[i];
        }
        return isdaMap;
    }
}

#endif
