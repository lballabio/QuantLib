/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2006, 2007 Chiara Fornarola
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

/*! \file eurlibor.hpp
    \brief %EUR %LIBOR rate
*/

#ifndef quantlib_eur_libor_hpp
#define quantlib_eur_libor_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! base class for all ICE %EUR %LIBOR indexes but the O/N
    /*! Euro LIBOR fixed by ICE.

        See <https://www.theice.com/marketdata/reports/170>.

        \warning This is the rate fixed in London by BBA. Use Euribor if
                 you're interested in the fixing by the ECB.
    */
    class EURLibor : public IborIndex {
      public:
        EURLibor(const Period& tenor,
                 const Handle<YieldTermStructure>& h = {});
        /*! \name Date calculations

            See <https://www.theice.com/marketdata/reports/170>.
            @{
        */
        Date valueDate(const Date& fixingDate) const override;
        Date maturityDate(const Date& valueDate) const override;
        // @}
      private:
        Calendar target_;
    };

    //! base class for the one day deposit ICE %EUR %LIBOR indexes
    /*! Euro O/N LIBOR fixed by ICE. It can be also used for T/N and S/N
        indexes, even if such indexes do not have ICE fixing.

        See <https://www.theice.com/marketdata/reports/170>.

        \warning This is the rate fixed in London by ICE. Use Eonia if
                 you're interested in the fixing by the ECB.
    */
    class DailyTenorEURLibor : public IborIndex {
      public:
        DailyTenorEURLibor(Natural settlementDays,
                           const Handle<YieldTermStructure>& h = {});
    };

    //! Overnight %EUR %Libor index
    class EURLiborON : public DailyTenorEURLibor {
      public:
        explicit EURLiborON(const Handle<YieldTermStructure>& h = {})
        : DailyTenorEURLibor(0, h) {}
    };

    /*! \deprecated If needed, use the EURLibor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the EURLibor class with an explicit tenor instead")]] EURLiborSW : public EURLibor {
      public:
        explicit EURLiborSW(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(1, Weeks), h) {}
    };

    /*! \deprecated If needed, use the EURLibor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the EURLibor class with an explicit tenor instead")]] EURLibor2W : public EURLibor {
      public:
        explicit EURLibor2W(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(2, Weeks), h) {}
    };

    //! 1-month %EUR %Libor index
    class EURLibor1M : public EURLibor {
      public:
        explicit EURLibor1M(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(1, Months), h) {}
    };

    /*! \deprecated If needed, use the EURLibor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the EURLibor class with an explicit tenor instead")]] EURLibor2M : public EURLibor {
      public:
        explicit EURLibor2M(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(2, Months), h) {}
    };

    //! 3-months %EUR %Libor index
    class EURLibor3M : public EURLibor {
      public:
        explicit EURLibor3M(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(3, Months), h) {}
    };

    /*! \deprecated If needed, use the EURLibor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the EURLibor class with an explicit tenor instead")]] EURLibor4M : public EURLibor {
      public:
        explicit EURLibor4M(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(4, Months), h) {}
    };

    /*! \deprecated If needed, use the EURLibor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the EURLibor class with an explicit tenor instead")]] EURLibor5M : public EURLibor {
      public:
        explicit EURLibor5M(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(5, Months), h) {}
    };

    //! 6-months %EUR %Libor index
    class EURLibor6M : public EURLibor {
      public:
        explicit EURLibor6M(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(6, Months), h) {}
    };

    /*! \deprecated If needed, use the EURLibor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the EURLibor class with an explicit tenor instead")]] EURLibor7M : public EURLibor{
      public:
        explicit EURLibor7M(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(7, Months), h) {}
    };

    /*! \deprecated If needed, use the EURLibor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the EURLibor class with an explicit tenor instead")]] EURLibor8M : public EURLibor {
      public:
        explicit EURLibor8M(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(8, Months), h) {}
    };

    /*! \deprecated If needed, use the EURLibor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the EURLibor class with an explicit tenor instead")]] EURLibor9M : public EURLibor {
      public:
        explicit EURLibor9M(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(9, Months), h) {}
    };

    /*! \deprecated If needed, use the EURLibor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the EURLibor class with an explicit tenor instead")]] EURLibor10M : public EURLibor {
      public:
        explicit EURLibor10M(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(10, Months), h) {}
    };

    /*! \deprecated If needed, use the EURLibor class with an explicit tenor instead.
                    Deprecated in version 1.35.
    */
    class [[deprecated("If needed, use the EURLibor class with an explicit tenor instead")]] EURLibor11M : public EURLibor {
      public:
        explicit EURLibor11M(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(11, Months), h) {}
    };

    //! 1-year %EUR %Libor index
    class EURLibor1Y : public EURLibor {
      public:
        explicit EURLibor1Y(const Handle<YieldTermStructure>& h = {})
        : EURLibor(Period(1, Years), h) {}
    };


}

#endif
