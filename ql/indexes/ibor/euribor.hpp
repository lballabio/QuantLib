/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2006 Chiara Fornarola
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano

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

/*! \file euribor.hpp
    \brief %Euribor index
*/

#ifndef quantlib_euribor_hpp
#define quantlib_euribor_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! %Euribor index
    /*! Euribor rate fixed by the ECB.

        \warning This is the rate fixed by the ECB. Use EurLibor
                 if you're interested in the London fixing by BBA.
    */
    class Euribor : public IborIndex {
      public:
        Euribor(const Period& tenor,
                Handle<YieldTermStructure> h = {});
    };

    //! Actual/365 %Euribor index
    /*! Euribor rate adjusted for the mismatch between the actual/360
        convention used for Euribor and the actual/365 convention
        previously used by a few pre-EUR currencies.
    */
    class Euribor365 : public IborIndex {
      public:
        Euribor365(const Period& tenor,
                   Handle<YieldTermStructure> h = {});
    };

    //! 1-week %Euribor index
    class Euribor1W : public Euribor {
      public:
        explicit Euribor1W(Handle<YieldTermStructure> h = {})
        : Euribor(Period(1, Weeks), std::move(h)) {}
    };

    /*! \deprecated Renamed to Euribor1W.
         Deprecated in version 1.35.
    */
    [[deprecated("Renamed to Euribor1W")]]
    typedef Euribor1W EuriborSW;
    
    /*! \deprecated If really needed, use the Euribor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If really needed, use the Euribor class with an explicit tenor instead")]] Euribor2W : public Euribor {
      public:
        explicit Euribor2W(Handle<YieldTermStructure> h = {})
        : Euribor(Period(2, Weeks), std::move(h)) {}
    };

    /*! \deprecated If really needed, use the Euribor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If really needed, use the Euribor class with an explicit tenor instead")]] Euribor3W : public Euribor {
      public:
        explicit Euribor3W(Handle<YieldTermStructure> h = {})
        : Euribor(Period(3, Weeks), std::move(h)) {}
    };

    //! 1-month %Euribor index
    class Euribor1M : public Euribor {
      public:
        explicit Euribor1M(Handle<YieldTermStructure> h = {})
        : Euribor(Period(1, Months), std::move(h)) {}
    };

    /*! \deprecated If really needed, use the Euribor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If really needed, use the Euribor class with an explicit tenor instead")]] Euribor2M : public Euribor {
      public:
        explicit Euribor2M(Handle<YieldTermStructure> h = {})
        : Euribor(Period(2, Months), std::move(h)) {}
    };

    //! 3-months %Euribor index
    class Euribor3M : public Euribor {
      public:
        explicit Euribor3M(Handle<YieldTermStructure> h = {})
        : Euribor(Period(3, Months), std::move(h)) {}
    };

    /*! \deprecated If really needed, use the Euribor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If really needed, use the Euribor class with an explicit tenor instead")]] Euribor4M : public Euribor {
      public:
        explicit Euribor4M(Handle<YieldTermStructure> h = {})
        : Euribor(Period(4, Months), std::move(h)) {}
    };

    /*! \deprecated If really needed, use the Euribor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If really needed, use the Euribor class with an explicit tenor instead")]] Euribor5M : public Euribor {
      public:
        explicit Euribor5M(Handle<YieldTermStructure> h = {})
        : Euribor(Period(5, Months), std::move(h)) {}
    };

    //! 6-months %Euribor index
    class Euribor6M : public Euribor {
      public:
        explicit Euribor6M(Handle<YieldTermStructure> h = {})
        : Euribor(Period(6, Months), std::move(h)) {}
    };

    /*! \deprecated If really needed, use the Euribor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If really needed, use the Euribor class with an explicit tenor instead")]] Euribor7M : public Euribor {
      public:
        explicit Euribor7M(Handle<YieldTermStructure> h = {})
        : Euribor(Period(7, Months), std::move(h)) {}
    };

    /*! \deprecated If really needed, use the Euribor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If really needed, use the Euribor class with an explicit tenor instead")]] Euribor8M : public Euribor {
      public:
        explicit Euribor8M(Handle<YieldTermStructure> h = {})
        : Euribor(Period(8, Months), std::move(h)) {}
    };

    /*! \deprecated If really needed, use the Euribor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If really needed, use the Euribor class with an explicit tenor instead")]] Euribor9M : public Euribor {
      public:
        explicit Euribor9M(Handle<YieldTermStructure> h = {})
        : Euribor(Period(9, Months), std::move(h)) {}
    };

    /*! \deprecated If really needed, use the Euribor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If really needed, use the Euribor class with an explicit tenor instead")]] Euribor10M : public Euribor {
      public:
        explicit Euribor10M(Handle<YieldTermStructure> h = {})
        : Euribor(Period(10, Months), std::move(h)) {}
    };

    /*! \deprecated If really needed, use the Euribor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If really needed, use the Euribor class with an explicit tenor instead")]] Euribor11M : public Euribor {
      public:
        explicit Euribor11M(Handle<YieldTermStructure> h = {})
        : Euribor(Period(11, Months), std::move(h)) {}
    };

    //! 1-year %Euribor index
    class Euribor1Y : public Euribor {
      public:
        explicit Euribor1Y(Handle<YieldTermStructure> h = {})
        : Euribor(Period(1, Years), std::move(h)) {}
    };


    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_SW : public Euribor365 {
      public:
        explicit Euribor365_SW(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(1, Weeks), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_2W : public Euribor365 {
      public:
        explicit Euribor365_2W(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(2, Weeks), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_3W : public Euribor365 {
      public:
        explicit Euribor365_3W(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(3, Weeks), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_1M : public Euribor365 {
      public:
        explicit Euribor365_1M(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(1, Months), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_2M : public Euribor365 {
      public:
        explicit Euribor365_2M(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(2, Months), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_3M : public Euribor365 {
      public:
        explicit Euribor365_3M(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(3, Months), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_4M : public Euribor365 {
      public:
        explicit Euribor365_4M(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(4, Months), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_5M : public Euribor365 {
      public:
        explicit Euribor365_5M(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(5, Months), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_6M : public Euribor365 {
      public:
        explicit Euribor365_6M(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(6, Months), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_7M : public Euribor365 {
      public:
        explicit Euribor365_7M(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(7, Months), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_8M : public Euribor365 {
      public:
        explicit Euribor365_8M(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(8, Months), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_9M : public Euribor365 {
      public:
        explicit Euribor365_9M(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(9, Months), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_10M : public Euribor365 {
      public:
        explicit Euribor365_10M(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(10, Months), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_11M : public Euribor365 {
      public:
        explicit Euribor365_11M(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(11, Months), std::move(h)) {}
    };

    /*! \deprecated If needed, use the Euribor365 class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the Euribor365 class with an explicit tenor instead")]] Euribor365_1Y : public Euribor365 {
      public:
        explicit Euribor365_1Y(Handle<YieldTermStructure> h = {})
        : Euribor365(Period(1, Years), std::move(h)) {}
    };

}

#endif
