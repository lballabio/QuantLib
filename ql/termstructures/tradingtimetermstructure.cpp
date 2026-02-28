#include <ql/termstructures/tradingtimetermstructure.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

	tradingTimeTermStructure::tradingTimeTermStructure(DayCounter dc, Real weekendWeight) 
        : TermStructure(dc), weekendWeight_(weekendWeight) 
    {}

	tradingTimeTermStructure::tradingTimeTermStructure(const Date& referenceDate,
                                                       Calendar calendar,
                                                       Real weekendWeight,
                                                       std::vector<Handle<Quote>> events,
                                                       const std::vector<Date>& eventDates)
    : TermStructure(referenceDate, calendar, Actual365Fixed()), weekendWeight_(weekendWeight), 
      events_(std::move(events)), eventDates_(eventDates),
      eventTimes_(eventDates.size()), nEvents_(events_.size())
    {
        setEvents(tradingTimeTermStructure::referenceDate());
        for (Size i = 0; i < nEvents_; ++i)
            registerWith(events_[i]);
    }

    tradingTimeTermStructure::tradingTimeTermStructure(Natural settlementDays,
                                                       Calendar calendar,
                                                       Real weekendWeight,
                                                       std::vector<Handle<Quote>> events,
                                                       const std::vector<Date>& eventDates)
    : TermStructure(settlementDays, calendar, Actual365Fixed()), weekendWeight_(weekendWeight),
      events_(std::move(events)), eventDates_(eventDates),
      eventTimes_(eventDates.size()), nEvents_(events_.size()) 
    {
        setEvents(tradingTimeTermStructure::referenceDate());
        for (Size i = 0; i < nEvents_; ++i)
            registerWith(events_[i]);
    }

    void tradingTimeTermStructure::setEvents(const Date& referenceDate, Period period) 
    {
        // handle weekends and holidays
        // given a weekend weight of w ( < 1.0) set an event with 
        // a weight of (w - 1) (which is < 0)! This is to handle 
        // the trading time calculation for a given time. Since 
        // this time is based on an Act/365 day counter, we need 
        // to include weekends as a negative weight!
        holidayDates_ = calendar().holidayList(referenceDate, calendar().advance(referenceDate, period), true);
        holidayTimes_.reserve(holidayDates_.size());
        for (Size j = 0; j < holidayDates_.size(); j++)
            holidayTimes_.push_back(timeFromReference(holidayDates_[j]));

        // fixed dates
        QL_REQUIRE(eventDates_.size() == nEvents_, "mismatch between number of events ("
                                                       << nEvents_ << ") and event dates ("
                                                       << eventDates_.size() << ")");
        
        for (Size i = 0; i < nEvents_; ++i)
            eventTimes_[i] = timeFromReference(eventDates_[i]);

        latestReference_ = referenceDate;
    }
    
    void tradingTimeTermStructure::update() {
        TermStructure::update();
        Date newReference = Date();
        try {
            newReference = referenceDate();
            if (newReference != latestReference_)
                setEvents(newReference);
        } catch (Error&) {
            if (newReference == Date()) {
                // the curve couldn't calculate the reference
                // date. Most of the times, this is because some
                // underlying handle wasn't set, so we can just absorb
                // the exception and continue; the jumps will be set
                // correctly when a valid underlying is set.
                return;
            } else {
                // something else happened during the call to
                // setEvents(), so we let the exception bubble up.
                throw;
            }
        }
    }

    Real tradingTimeTermStructure::tradingTime(const Date& d1, const Date& d2) const {
        // get the business days between the two dates - ignore beginning and ending dates
        Time bizDays = calendar_.businessDaysBetween(d1, d2, false, false);
        
        // adjust for the beginning and ending day
        bizDays += (calendar_.isBusinessDay(d1)) ? 1. - d1.fractionOfDay() : 0.0;
        bizDays += (calendar_.isBusinessDay(d2)) ? d2.fractionOfDay() : 0.0;

        Time totalDays = daysBetween(d1, d2);  // this already accounts for intraday time!

        Time adjDays = bizDays + (totalDays - bizDays) * weekendWeight_; // account for weekends + holidays

        // now adjust for events!
        for (Size i = 0; i < nEvents_; ++i) {
            if (eventDates_[i] > d1 && eventDates_[i] <= d2) {
                QL_REQUIRE(events_[i]->isValid(), "invalid " << io::ordinal(i + 1) << " jump quote");
                adjDays += events_[i]->value();
            }
        }

        // trading time is the year fraction, adjusted for the ratio of 
        // trading days to total days. We are assuming that the yearFraction
        // scales linearly with totalDays. This is trivially true for Act/365 
        // which is what is genrally used in FX. However, for other day counters
        // this might cause an issue so be careful! 
        return dayCounter().yearFraction(d1, d2) * adjDays / totalDays;
    }

    Real tradingTimeTermStructure::tradingTime(Time t) const {
        // t is the year fraction calcuated using the dayCounter
        // of the term structure. This is assumed to be Act/365 
        // which is the deault for FX. 
        // Holiday/weekend weights and event weights are speicfied in 
        // number of days. Therefore, to convert from real time to 
        // trading time, we calculate the number of days to be adjusted
        // by and then scale that by 365 to get the trading time!
        // This will clearly break down with other day counters!
        Real eventWeight = 0, holidayWeight = 0;
        for (Size i = 0; i < eventTimes_.size(); i++) {
            if (eventTimes_[i] < t)
                eventWeight += events_[i]->value();
        }

        for (Size j = 0; j < holidayTimes_.size(); j++) {
            if (holidayTimes_[j] < t)
                holidayWeight += (weekendWeight_ - 1.);
        }

        return t + eventWeight / 365. + holidayWeight / 365.;
    }


}