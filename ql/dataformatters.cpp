
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file dataformatters.cpp
    \brief classes used to format data for output

    \fullpath
    ql/%dataformatters.cpp
*/

// $Id$

#include <ql/dataformatters.hpp>
#include <ql/null.hpp>

namespace QuantLib {

    std::string IntegerFormatter::toString(int i, int digits) {
        if (i == Null<int>())
        return std::string("null");
        char s[64];
        QL_SPRINTF(s,"%*d",(digits>64?64:digits),i);
        return std::string(s);
    }

    std::string DoubleFormatter::toString(double x, int precision, 
                                          int digits) {
        if (x == Null<double>())
            return std::string("null");
        char s[64];
        QL_SPRINTF(s,"%*.*f",(digits>64?64:digits),
                             (precision>64?64:precision),x);
        return std::string(s);
    }

    std::string ArrayFormatter::toString(const Array& a, int precision, 
                                         int digits, Size perRow) {
        std::string s = "[ ";
        for (Size i=0, n=0; i<a.size(); i++,n++) {
            if (n == perRow) {
                s += "\n  ";
                n = 0;
            }
            if (i>0)
                s += " ; ";
            s += DoubleFormatter::toString(a[i],precision,digits);
        }
        s += " ]";
        return s;
    }

    std::ostream& operator<< (std::ostream& stream, const Array& a) {
        return stream << ArrayFormatter::toString(a);
    }

    std::string EuroFormatter::toString(double amount) {
        std::string output;
        if (amount < 0.0) {
            output = "-";
            amount = -amount;
        } else {
            output = " ";
        }
        int triples = 0;
        while (amount >= 1000.0) {
            amount /= 1000;
            triples++;
        }
        output += IntegerFormatter::toString(int(amount));
        amount -= int(amount);
        while (triples > 0) {
            amount *= 1000;
            output += ","+IntegerFormatter::toString(int(amount),3);
            amount -= int(amount);
            triples--;
        }
        amount *= 100;
        output += "."+IntegerFormatter::toString(int(amount+0.5),2);
        return output;
    }

    std::string RateFormatter::toString(double rate, int precision) {
        return DoubleFormatter::toString(rate*100,precision)+"%";
    }

    std::string DateFormatter::toString(const Date& d, bool shortFormat) {
        static const std::string monthName[] = {
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December" };
        std::string output;
        if (d == Date()) {
            output = "Null date";
        } else {
            int dd = d.dayOfMonth(), mm = int(d.month()), yyyy = d.year();
            if (shortFormat) {
                output = (mm < 10 ? "0" : "") + IntegerFormatter::toString(mm);
                output += (dd < 10 ? "/0" : "/") +
                    IntegerFormatter::toString(dd);
                output += "/" + IntegerFormatter::toString(yyyy);
            } else {
                output = monthName[mm-1] + " ";
                output += IntegerFormatter::toString(dd);
                switch (dd) {
                  case 1:
                  case 21:
                  case 31:
                    output += "st, ";
                    break;
                  case 2:
                  case 22:
                    output += "nd, ";
                    break;
                  case 3:
                  case 23:
                    output += "rd, ";
                    break;
                  default:
                    output += "th, ";
                }
                output += IntegerFormatter::toString(yyyy);
            }
        }
        return output;
    }

    std::ostream& operator<< (std::ostream& stream, const Date& date) {
        return stream << DateFormatter::toString(date, true);
    }

    std::string CurrencyFormatter::toString(Currency c) {
        switch (c) {
          case EUR: return "EUR";
          case GBP: return "GBP";
          case USD: return "USD";
          case DEM: return "DEM";
          case ITL: return "ITL";
          case CHF: return "CHF";
          case AUD: return "AUD";
          case CAD: return "CAD";
          case DKK: return "DKK";
          case JPY: return "JPY";
          case PLZ: return "PLZ";
          case SEK: return "SEK";
          case CZK: return "CZK";
          case EEK: return "EEK";
          case ISK: return "ISK";
          case NOK: return "NOK";
          case SKK: return "SKK";
          case HKD: return "HKD";
          case NZD: return "NZD";
          case SGD: return "SGD";
          case GRD: return "GRD";
          case HUF: return "HUF";
          case LVL: return "LVL";
          case ROL: return "ROL";
          case BGL: return "BGL";
          case CYP: return "CYP";
          case LTL: return "LTL";
          case MTL: return "MTL";
          case TRL: return "TRL";
          case ZAR: return "ZAR";
          case SIT: return "SIT";
          case KRW: return "KRW";
          default:  return "unknown";
        }
    }

    std::string StringFormatter::toLowercase(const std::string& s) {
        std::string output = s;
        for (std::string::iterator i=output.begin(); i!=output.end(); i++)
            *i = QL_TOLOWER(*i);
        return output;
    }

    std::string StringFormatter::toUppercase(const std::string& s) {
        std::string output = s;
        for (std::string::iterator i=output.begin(); i!=output.end(); i++)
            *i = QL_TOUPPER(*i);
        return output;
    }

}
