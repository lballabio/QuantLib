
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

/*! \file dataformatters.cpp
    \brief classes used to format data for output
*/

#include <ql/dataformatters.hpp>
#include <ql/null.hpp>

namespace QuantLib {

    std::string EuroFormatter::toString(Decimal amount) {
        std::string output;
        if (amount < 0.0) {
            output = "-";
            amount = -amount;
        } else {
            output = " ";
        }
        Integer triples = 0;
        while (amount >= 1000.0) {
            amount /= 1000;
            triples++;
        }
        output += IntegerFormatter::toString(Integer(amount));
        amount -= Integer(amount);
        while (triples > 0) {
            amount *= 1000;
            output += ","+IntegerFormatter::toString(Integer(amount),3);
            amount -= Integer(amount);
            triples--;
        }
        amount *= 100;
        output += "."+IntegerFormatter::toString(Integer(amount+0.5),2);
        return output;
    }

    std::string RateFormatter::toString(Rate rate, Integer precision) {
        return DecimalFormatter::toPercentage(rate,precision);
    }

    std::string VolatilityFormatter::toString(Volatility vol,
                                              Integer precision) {
        return DecimalFormatter::toPercentage(vol,precision);
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
            Integer dd = d.dayOfMonth(), mm = Integer(d.month()),
                    yyyy = d.year();
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

    std::string CurrencyFormatter::toString(CurrencyTag c) {
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

    std::string OptionTypeFormatter::toString(Option::Type type) {
        switch (type) {
          case Option::Call:     return "call";
          case Option::Put:      return "put";
          default:
            QL_FAIL("unknown option type");
        }
    }

    #ifndef QL_PATCH_MICROSOFT
    std::ostream& operator<< (std::ostream& stream, const Array& a) {
        return stream << ArrayFormatter::toString(a.begin(), a.end());
    }

    std::ostream& operator<< (std::ostream& stream, const Matrix& matrix) {
        for (Size i=0; i<matrix.rows(); i++) {
            for (Size j=0; j<matrix.columns(); j++) {
                stream << DecimalFormatter::toString(matrix[i][j]) << " ";
            }
            stream << "\n";
        }
        return stream;
    }
    #endif

}
