
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

/*! \file calendar.h
	\brief Calendar abstract class
*/

#ifndef quantlib_calendar_h
#define quantlib_calendar_h

#include "qldefines.h"
#include "qlerrors.h"
#include "date.h"
#include "handle.h"
#include <string>

namespace QuantLib {

	//! Conventions for rolling, i.e., moving from a holiday to its next/previous business day.
	enum RollingConvention {
		Following,				/*!< go to next business day */
		ModifiedFollowing,		/*!< go to next business day unless it belongs to a different month,
									in which case go back to previous business day */
		Preceding,				/*!< go back to previous business day */
		ModifiedPreceding		/*!< go back to previous business day unless it belongs to a different month,
									in which case go to next business day */
	};
	
	//! Calendar abstract class
	/*! This class is purely abstract and defines the interface of concrete
		calendar classes which will be derived from this one.
		
		It provides methods for determining whether a date is a business day or a 
		holiday for a given market, and for incrementing/decrementing a date of a 
		given number of business days.
	*/
	class Calendar {
	  public:
		//! Returns the name of the calendar.
		/*!	\warning This method is used for output and comparison between calendars.
			It is <b>not</b> meant to be used for writing switch-on-type code.
		*/
		virtual std::string name() const = 0;
		//! Returns <tt>true</tt> iff the date is a business day for the given market.
		virtual bool isBusinessDay(const Date&) const = 0;
		//! Returns <tt>true</tt> iff the date is a holiday for the given market.
		bool isHoliday(const Date& d) const { return !isBusinessDay(d); }
		//! Returns the next/previous business day on the given market with respect to the given date and convention.
		Date roll(RollingConvention, const Date&) const;
		//! Advances the given date of the givn number of business days and returns the result.
		/*! \note The input date is not modified.
		*/
		Date advance(const Date&, int businessDays) const;
	};
	
	QL_DECLARE_TEMPLATE_SPECIALIZATION(
	bool operator==(const Handle<Calendar>&, const Handle<Calendar>&))
	QL_DECLARE_TEMPLATE_SPECIALIZATION(
	bool operator!=(const Handle<Calendar>&, const Handle<Calendar>&))
	
	/*! \namespace QuantLib::Calendars
		\brief Concrete implementations of the Calendar interface
	*/
	namespace Calendars {
	
		//! Calendar with no holidays, not even saturdays and sundays.
		class NullCalendar : public Calendar {
		  public:
			NullCalendar() {}
			//! returns "None"
			std::string name() const { return "None"; }
			//! always returns <tt>true</tt>
			bool isBusinessDay(const Date& d) const { d; return true; }
		};
	
	}
	
	// inline definitions

	/*! \defgroup calendarcomparisons Comparisons between calendars */
	
	/*! \ingroup calendarcomparisons
		\relates Calendar
	*/
	QL_TEMPLATE_SPECIALIZATION
	inline bool operator==(const Handle<Calendar>& h1, const Handle<Calendar>& h2) {
		return (h1->name() == h2->name());
	}
	
	/*! \ingroup calendarcomparisons
		\relates Calendar
	*/
	QL_TEMPLATE_SPECIALIZATION
	inline bool operator!=(const Handle<Calendar>& h1, const Handle<Calendar>& h2) {
		return (h1->name() != h2->name());
	}

}


#endif
