
/*
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

#include <ql/currency.hpp>
#include <ql/Currencies/africa.hpp>
#include <ql/Currencies/america.hpp>
#include <ql/Currencies/asia.hpp>
#include <ql/Currencies/europe.hpp>
#include <ql/Currencies/oceania.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED
    Currency make_currency(CurrencyTag c) {
        switch (c) {
          case EUR: return EURCurrency();
          case GBP: return GBPCurrency();
          case USD: return USDCurrency();
          case DEM: return DEMCurrency();
          case ITL: return ITLCurrency();
          case CHF: return CHFCurrency();
          case AUD: return AUDCurrency();
          case CAD: return CADCurrency();
          case DKK: return DKKCurrency();
          case JPY: return JPYCurrency();
          case SEK: return SEKCurrency();
          case CZK: return CZKCurrency();
          case EEK: return EEKCurrency();
          case ISK: return ISKCurrency();
          case NOK: return NOKCurrency();
          case SKK: return SKKCurrency();
          case HKD: return HKDCurrency();
          case NZD: return NZDCurrency();
          case SGD: return SGDCurrency();
          case GRD: return GRDCurrency();
          case HUF: return HUFCurrency();
          case LVL: return LVLCurrency();
          case ROL: return ROLCurrency();
          case BGL: return BGLCurrency();
          case CYP: return CYPCurrency();
          case LTL: return LTLCurrency();
          case MTL: return MTLCurrency();
          case TRL: return TRLCurrency();
          case ZAR: return ZARCurrency();
          case SIT: return SITCurrency();
          case KRW: return KRWCurrency();
          case ARS: return ARSCurrency();
          case ATS: return ATSCurrency();
          case BDT: return BDTCurrency();
          case BEF: return BEFCurrency();
          case BRL: return BRLCurrency();
          case BYB: return BYRCurrency();
          case CLP: return CLPCurrency();
          case CNY: return CNYCurrency();
          case COP: return COPCurrency();
          case ILS: return ILSCurrency();
          case INR: return INRCurrency();
          case IQD: return IQDCurrency();
          case IRR: return IRRCurrency();
          case KWD: return KWDCurrency();
          case MXP: return MXNCurrency();
          case NPR: return NPRCurrency();
          case PKR: return PKRCurrency();
          case PLN: return PLNCurrency();
          case SAR: return SARCurrency();
          case THB: return THBCurrency();
          case TTD: return TTDCurrency();
          case TWD: return TWDCurrency();
          case VEB: return VEBCurrency();
          default:  QL_FAIL("unknown currency tag");
        }
    }
    #endif

    #ifndef QL_DISABLE_DEPRECATED
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
    #endif

    std::string CurrencyFormatter::toString(const Currency& c) {
        if (c.isValid())
            return c.code() + " currency (" + c.name() + ")";
        else
            return "null currency";
    }

}

