
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

/*! \file currency.h
	\brief Abstract currency class
*/

/*! \namespace QuantLib::Currencies
	\brief Concrete implementations of the Currency interface
*/

#ifndef quantlib_currency_h
#define quantlib_currency_h

#include "qldefines.h"
#include "calendar.h"
#include "date.h"
#include "handle.h"
#include <string>

namespace QuantLib {

	//! Abstract currency class
	/*! This class is purely abstract and defines the interface of concrete
		currency classes which will be derived from this one.
		
		It provides methods for determining a number of market conventions
		which vary depending on the used currency.
	*/
	class Currency {
	  public:
		//! Returns the name of the currency.
		/*!	\warning This method is used for output and comparison between currencies.
			It is <b>not</b> meant to be used for writing switch-on-type code.
		*/
		virtual std::string name() const = 0;
		//! \name Settlement conventions
		//@{
		//! Returns the calendar upon which the settlement days are calculated.
		virtual Handle<Calendar> settlementCalendar() const = 0;
		//! Returns the number of settlement days.
		virtual int settlementDays() const = 0;
		//! Returns the settlement date relative to a given actual date.
		Date settlementDate(const Date&) const;
		//@}
		// conventions for deposits or any other rates can be added
	};
	
	// comparison based on name
	
	QL_DECLARE_TEMPLATE_SPECIALIZATION(
	bool operator==(const Handle<Currency>&, const Handle<Currency>&))
	QL_DECLARE_TEMPLATE_SPECIALIZATION(
	bool operator!=(const Handle<Currency>&, const Handle<Currency>&))
	
	
	// inline definitions
	
	inline Date Currency::settlementDate(const Date& d) const {
		return settlementCalendar()->advance(d,settlementDays());
	}
	
	/*! Returns <tt>true</tt> iff the two currencies belong to the same derived class.
		\relates Currency
	*/
	QL_TEMPLATE_SPECIALIZATION
	inline bool operator==(const Handle<Currency>& c1, const Handle<Currency>& c2) {
		return (c1->name() == c2->name());
	}
	
	/*! \relates Currency */
	QL_TEMPLATE_SPECIALIZATION
	inline bool operator!=(const Handle<Currency>& c1, const Handle<Currency>& c2) {
		return (c1->name() != c2->name());
	}

}


#endif
