#ifndef quantlib_axl_trading_time_term_structure_hpp
#define quantlib_axl_trading_time_term_structure_hpp

#include <ql/termstructure.hpp>
#include <ql/quote.hpp>
#include <ql/time/date.hpp>
#include <ql/time/calendars/weekendsonly.hpp>

namespace QuantLib {
    class tradingTimeTermStructure : public TermStructure {

      public:

        tradingTimeTermStructure(DayCounter dc = DayCounter(), 
                                 Real weekendWeight = 0.0);

        tradingTimeTermStructure(const Date& referenceDate,
                                 Calendar calendar = WeekendsOnly(),
                                 Real weekendWeight = 0.0,
                                 std::vector<Handle<Quote>> events = {},
                                 const std::vector<Date>& eventDates = {});

        tradingTimeTermStructure(Natural settlementDays=0,
                                 Calendar calendar = WeekendsOnly(),
                                 Real weekendWeight = 0.0,
                                 std::vector<Handle<Quote>> events = {},
                                 const std::vector<Date>& eventDates = {});

        //! \name Observer interface
        //@{
        void update() override;
        //@}
         
        //! \name TermStructure interface
        //@{
        virtual Date maxDate() const override { return Date::maxDate(); }
        //@}

        Real tradingTime(const Date& d1, const Date& d2) const;
        Real tradingTime(const Date& d) const { return tradingTime(referenceDate(), d); };

        Real tradingTime(Time t) const;

      private:
        // methods
        void setEvents(const Date& referenceDate, Period period = Period(5, Years));

        // data members
        Real weekendWeight_;
        std::vector<Handle<Quote>> events_;
        std::vector<Date> eventDates_;
        std::vector<Time> eventTimes_;
        std::vector<Date> holidayDates_;
        std::vector<Time> holidayTimes_;
        Size nEvents_ = 0;
        Date latestReference_;
    
    };

} // namespace QuantLib

#endif
