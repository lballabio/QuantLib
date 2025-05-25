/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef quantlib_custom_ibor_hpp
#define quantlib_custom_ibor_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    /*! LIBOR-like index that allows specifying custom calendars for value
        and maturity dates calculations:

        * valueDate() advances on the valueCalendar and adjusts on the
         maturityCalendar.

        * maturityDate() advances on the maturityCalendar.

        * fixingDate() goes back on the valueCalendar.

        Typical LIBOR indexes use:

        * fixingCalendar = valueCalendar = UK, maturityCalendar =
        JoinHolidays(UK, CurrencyCalendar) for non-EUR currencies.

        * fixingCalendar = JoinHolidays(UK, TARGET), valueCalendar =
        maturityCalendar = TARGET for EUR.
    */
    class CustomIborIndex : public IborIndex {
      public:
        CustomIborIndex(const std::string& familyName,
                        const Period& tenor,
                        Natural settlementDays,
                        const Currency& currency,
                        const Calendar& fixingCalendar,
                        Calendar  valueCalendar,
                        Calendar  maturityCalendar,
                        BusinessDayConvention convention,
                        bool endOfMonth,
                        const DayCounter& dayCounter,
                        const Handle<YieldTermStructure>& h = {});
        //! \name InterestRateIndex interface
        //@{
        Date fixingDate(const Date& valueDate) const override;
        Date valueDate(const Date& fixingDate) const override;
        Date maturityDate(const Date& valueDate) const override;
        // @}
        //! \name IborIndex interface
        //@{
        ext::shared_ptr<IborIndex> clone(const Handle<YieldTermStructure>& h) const override;
        // @}
        //! \name Other inspectors
        //@{
        Calendar valueCalendar() const { return valueCalendar_; }
        Calendar maturityCalendar() const { return maturityCalendar_; }
        // @}
      private:
        Calendar valueCalendar_;
        Calendar maturityCalendar_;
    };

}

#endif
