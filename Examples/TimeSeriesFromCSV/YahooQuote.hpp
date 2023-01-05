/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*!
Copyright (C) 2023 Oliver Ofenloch

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

#include <ql/time/date.hpp> // Date
#include <ql/types.hpp>     // Real, Natural, Integer, etc.

//! class to store a single datum of a stock quote retrieved from Yahoo!
class YahooQuote {
  public:
    YahooQuote(){};
    YahooQuote(QuantLib::Date const& Date,
               QuantLib::Real const& Open,
               QuantLib::Real const& High,
               QuantLib::Real const& Low,
               QuantLib::Real const& Close,
               QuantLib::Real const& AdjClose,
               QuantLib::Real const& Volume)
    : date_(Date), open_(Open), high_(High), low_(Low), close_(Close), adj_close_(AdjClose),
      volume_(Volume){};
    QuantLib::Date date() const { return date_; };
    QuantLib::Real open() const { return open_; };
    QuantLib::Real high() const { return high_; };
    QuantLib::Real low() const { return low_; };
    QuantLib::Real close() const { return close_; };
    QuantLib::Real adj_close() const { return adj_close_; };
    QuantLib::Real daily_return_abs() const { return daily_return_abs_; };
    QuantLib::Real daily_return_rel() const { return daily_return_rel_; };
    void set_daily_return_abs(QuantLib::Real const& daily_return_abs) {
        daily_return_abs_ = daily_return_abs;
    };
    void set_daily_return_rel(QuantLib::Real const& daily_return_rel) {
        daily_return_rel_ = daily_return_rel;
    };

  private:
    QuantLib::Date date_{QuantLib::Null<QuantLib::Date>()};
    QuantLib::Real open_{QuantLib::Null<QuantLib::Real>()};
    QuantLib::Real high_{QuantLib::Null<QuantLib::Real>()};
    QuantLib::Real low_{QuantLib::Null<QuantLib::Real>()};
    QuantLib::Real close_{QuantLib::Null<QuantLib::Real>()};
    QuantLib::Real adj_close_{QuantLib::Null<QuantLib::Real>()};
    QuantLib::Real volume_{QuantLib::Null<QuantLib::Real>()};
    QuantLib::Real daily_return_abs_{QuantLib::Null<QuantLib::Real>()};
    QuantLib::Real daily_return_rel_{QuantLib::Null<QuantLib::Real>()};
}; // class YahooQuote

//! specialization of Null for classYahooQuote
/*! This is needed to use class YahooQuote as data type in
    template<class T, class Container = std::map<Date, T>>
    class QuantLib::TimeSeries< T, Container >

    see discussion on issue #1546 (https://github.com/lballabio/QuantLib/issues/1546)
 */
namespace QuantLib {
#ifdef QL_NULL_AS_FUNCTIONS
    //! specialization of Null template for the YahooQuote class
    template <>
    inline YahooQuote Null<YahooQuote>() {
        return {};
    }
#else
    //! specialization of Null template for the YahooQuote class
    template <>
    class Null<YahooQuote> {
      public:
        Null() = default;
        operator YahooQuote() const { return {}; }
    };
#endif
} // namespace QuantLib