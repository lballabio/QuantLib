
/*
 Copyright (C) 2004 Decillion Pty(Ltd)
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file currency.hpp
    \brief Known currencies
*/

#ifndef quantlib_currency_hpp
#define quantlib_currency_hpp

#include <ql/Math/rounding.hpp>
#include <ql/errors.hpp>
#include <boost/shared_ptr.hpp>
#include <assert.h>
#include <boost/format.hpp>
#include <ostream>

namespace QuantLib {

    //! %Currency specification
    class Currency {
      public:
        //! default constructor
        /*! Instances built via this constructor have undefined
            behavior. Such instances can only act as placeholders
            and must be reassigned to a valid currency before being
            used.
        */
        Currency();
        //! \name Inspectors
        //@{
        //! currency name, e.g, "U.S. Dollar"
        const std::string& name() const;
        //! ISO 4217 three-letter code, e.g, "USD"
        const std::string& code() const;
        //! ISO 4217 numeric code, e.g, "840"
        Integer numericCode() const;
        //! symbol, e.g, "$"
        const std::string& symbol() const;
        //! fraction symbol, e.g, "¢"
        const std::string& fractionSymbol() const;
        //! number of fractionary parts in a unit, e.g, 100
        Integer fractionsPerUnit() const;
        //! rounding convention
        const Rounding& rounding() const;
        //! output format
        /*! The format will be fed three positional parameters,
            namely, value, code, and symbol, in this order.
        */
        boost::format format() const;
        //@}
        //! \name other info
        //@{
        //! is this a usable instance?
        bool isValid() const;
        //! currency used for triangulated exchange when required
        const Currency& triangulationCurrency() const;
        //@}
      protected:
        struct Data;
        boost::shared_ptr<Data> data_;
    };

    struct Currency::Data {
        std::string name, code;
        Integer numeric;
        std::string symbol, fractionSymbol;
        Integer fractionsPerUnit;
        Rounding rounding;
        Currency triangulated;
        boost::format format;

        Data(const std::string& name,
             const std::string& code,
             Integer numericCode,
             const std::string& symbol,
             const std::string& fractionSymbol,
             Integer fractionsPerUnit,
             const Rounding& rounding,
             const std::string& formatString,
             const Currency& triangulationCurrency = Currency())
        : name(name), code(code), numeric(numericCode),
          symbol(symbol), fractionsPerUnit(fractionsPerUnit),
          rounding(rounding), triangulated(triangulationCurrency),
          format(formatString) {
            format.exceptions(boost::io::all_error_bits ^
                              boost::io::too_many_args_bit);
        }
    };

    /*! \relates Currency */
    bool operator==(const Currency&, const Currency&);

    /*! \relates Currency */
    bool operator!=(const Currency&, const Currency&);

    /*! \relates Currency */
    std::ostream& operator<<(std::ostream&, const Currency&);


    #ifndef QL_DISABLE_DEPRECATED
    //! format currencies for output
    /*! \deprecated use streams and manipulators for proper formatting */
    class CurrencyFormatter {
      public:
        static std::string toString(const Currency& c);
    };
    #endif


    // inline definitions

    inline Currency::Currency() {}

    inline const std::string& Currency::name() const {
        return data_->name;
    }

    inline const std::string& Currency::code() const {
        return data_->code;
    }

    inline Integer Currency::numericCode() const {
        return data_->numeric;
    }

    inline const std::string& Currency::symbol() const {
        return data_->symbol;
    }

    inline const std::string& Currency::fractionSymbol() const {
        return data_->fractionSymbol;
    }

    inline Integer Currency::fractionsPerUnit() const {
        return data_->fractionsPerUnit;
    }

    inline const Rounding& Currency::rounding() const {
        return data_->rounding;
    }

    inline boost::format Currency::format() const {
        return data_->format;
    }

    inline bool Currency::isValid() const {
        return data_;
    }

    inline const Currency& Currency::triangulationCurrency() const {
        return data_->triangulated;
    }

    inline bool operator==(const Currency& c1, const Currency& c2) {
        return c1.name() == c2.name();
    }

    inline bool operator!=(const Currency& c1, const Currency& c2) {
        return !(c1 == c2);
    }

}


#endif
