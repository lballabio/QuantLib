
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file basicdataformatters.cpp
    \brief classes used to format basic types for output
*/

#include <ql/basicdataformatters.hpp>
#include <ql/null.hpp>

namespace QuantLib {

    std::string IntegerFormatter::toString(long l, int digits) {
        static long null = long(Null<int>());
        if (l == null)
            return std::string("null");
        char s[64];
        QL_SPRINTF(s,"%*ld",(digits>64?64:digits),l);
        return std::string(s);
    }

    std::string IntegerFormatter::toPowerOfTwo(long l, int digits) {
        if (l < 0L)
            return "-" + SizeFormatter::toPowerOfTwo(Size(-l),digits);
        else
            return SizeFormatter::toPowerOfTwo(Size(l),digits);
    }

    std::string SizeFormatter::toString(Size l, int digits) {
        static Size null = Size(Null<int>());
        if (l == null)
            return std::string("null");
        char s[64];
        QL_SPRINTF(s,"%*lu",(digits>64?64:digits),l);
        return std::string(s);
    }

    std::string SizeFormatter::toOrdinal(Size ll) {
        std::string suffix;
        unsigned long l = (unsigned long)(ll);
        if (l == 11UL || l == 12UL || l == 13UL) {
            suffix = "th";
        } else {
            switch (l % 10) {
              case 1:  suffix = "st";  break;
              case 2:  suffix = "nd";  break;
              case 3:  suffix = "rd";  break;
              default: suffix = "th";
            }
        }
        return toString(ll)+suffix;
    }

    std::string SizeFormatter::toPowerOfTwo(Size l, int digits) {
        static Size null = Size(Null<int>());
        if (l == null)
            return std::string("null");
        int power = 0;
        while (!(l & 1UL)) {
            power++;
            l >>= 1;
        }
        return toString(l,digits) + "*2^" + toString(power,2);
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

    std::string DoubleFormatter::toExponential(double x, int precision,
                                               int digits) {
        if (x == Null<double>())
            return std::string("null");
        char s[64];
        QL_SPRINTF(s,"%*.*e",(digits>64?64:digits),
                             (precision>64?64:precision),x);
        return std::string(s);
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

    std::string DateFormatter::toString(const Date& d,
                                        DateFormatter::Format f) {
        static const std::string monthName[] = {
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"
        };
        std::string output;
        if (d == Date()) {
            output = "Null date";
        } else {
            int dd = d.dayOfMonth(), mm = int(d.month()), yyyy = d.year();
            switch (f) {
              case Long:
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
                break;
              case Short:
                output = (mm < 10 ? "0" : "") +
                         IntegerFormatter::toString(mm);
                output += (dd < 10 ? "/0" : "/") +
                         IntegerFormatter::toString(dd);
                output += "/" + IntegerFormatter::toString(yyyy);
                break;
              case ISO:
                output = IntegerFormatter::toString(yyyy);
                output += (mm < 10 ? "-0" : "-") +
                         IntegerFormatter::toString(mm);
                output += (dd < 10 ? "-0" : "-") +
                         IntegerFormatter::toString(dd);
                break;
              default:
                QL_FAIL("unknown date format");
            }
        }
        return output;
    }

    std::ostream& operator<< (std::ostream& stream, const Date& date) {
        return stream << DateFormatter::toString(date, DateFormatter::Short);
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
          case ARS: return "ARS";
          case ATS: return "ATS";
          case BDT: return "BDT";
          case BEF: return "BEF";
          case BRL: return "BRL";
          case BYB: return "BYB";
          case CLP: return "CLP";
          case CNY: return "CNY";
          case COP: return "COP";
          case ILS: return "ILS";
          case INR: return "INR";
          case IQD: return "IQD";
          case IRR: return "IRR";
          case KWD: return "KWD";
          case MXP: return "MXP";
          case NPR: return "NPR";
          case PKR: return "PKR";
          case PLN: return "PLN";
          case SAR: return "SAR";
          case THB: return "THB";
          case TTD: return "TTD";
          case TWD: return "TWD";
          case VEB: return "VEB";
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

    std::string OptionTypeFormatter::toString(Option::Type type) {
        switch (type) {
          case Option::Call:     return "call";
          case Option::Put:      return "put";
          case Option::Straddle: return "straddle";
          default:
            QL_FAIL("unknown option type");
        }
    }

}
