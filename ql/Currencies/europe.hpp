/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file europe.hpp
    \brief European currencies

    Data from http://fx.sauder.ubc.ca/currency_table.html
    and http://www.thefinancials.com/vortex/CurrencyFormats.html
*/

#ifndef quantlib_european_currencies_hpp
#define quantlib_european_currencies_hpp

#include <ql/currency.hpp>

namespace QuantLib {

    //! Bulgarian lev
    /*! The ISO three-letter code is BGL; the numeric code is 100.
        It is divided in 100 stotinki.

        \ingroup currencies
    */
    class BGL : public Currency {
      public:
        BGL() {
            static boost::shared_ptr<Data> bglData(
                                         new Data("Bulgarian lev", "BGL", 100,
                                                  "lv", "", 100,
                                                  Rounding(),
                                                  "%1$.2f %3%"));
            data_ = bglData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to BLG */
    typedef BGL BGLCurrency;
    #endif

    //! Belarussian ruble
    /*! The ISO three-letter code is BYR; the numeric code is 974.
        It has no subdivisions.

        \ingroup currencies
    */
    class BYR : public Currency {
      public:
        BYR() {
            static boost::shared_ptr<Data> byrData(
                                     new Data("Belarussian ruble", "BYR", 974,
                                              "BR", "", 1,
                                              Rounding(),
                                              "%2% %1$.0f"));
            data_ = byrData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to BYR */
    typedef BYR BYRCurrency;
    #endif

    //! Swiss franc
    /*! The ISO three-letter code is CHF; the numeric code is 756.
        It is divided into 100 cents.

        \ingroup currencies
    */
    class CHF : public Currency {
      public:
        CHF() {
            static boost::shared_ptr<Data> chfData(
                                           new Data("Swiss franc", "CHF", 756,
                                                    "SwF", "", 100,
                                                    Rounding(),
                                                    "%3% %1$.2f"));
            data_ = chfData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to CHF */
    typedef CHF CHFCurrency;
    #endif

    //! Cyprus pound
    /*! The ISO three-letter code is CYP; the numeric code is 196.
        It is divided in 100 cents.

        \ingroup currencies
    */
    class CYP : public Currency {
      public:
        CYP() {
            static boost::shared_ptr<Data> cypData(
                                          new Data("Cyprus pound", "CYP", 196,
                                                   "\xA3" "C", "", 100,
                                                   Rounding(),
                                                   "%3% %1$.2f"));
            data_ = cypData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to CYP */
    typedef CYP CYPCurrency;
    #endif

    //! Czech koruna
    /*! The ISO three-letter code is CZK; the numeric code is 203.
        It is divided in 100 haleru.

        \ingroup currencies
    */
    class CZK : public Currency {
      public:
        CZK() {
            static boost::shared_ptr<Data> czkData(
                                          new Data("Czech koruna", "CZK", 203,
                                                   "Kc", "", 100,
                                                   Rounding(),
                                                   "%1$.2f %3%"));
            data_ = czkData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to CZK */
    typedef CZK CZKCurrency;
    #endif

    //! Danish krone
    /*! The ISO three-letter code is DKK; the numeric code is 208.
        It is divided in 100 øre.

        \ingroup currencies
    */
    class DKK : public Currency {
      public:
        DKK() {
            static boost::shared_ptr<Data> dkkData(
                                          new Data("Danish krone", "DKK", 208,
                                                   "Dkr", "", 100,
                                                   Rounding(),
                                                   "%3% %1$.2f"));
            data_ = dkkData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to DKK */
    typedef DKK DKKCurrency;
    #endif

    //! Estonian kroon
    /*! The ISO three-letter code is EEK; the numeric code is 233.
        It is divided in 100 senti.

        \ingroup currencies
    */
    class EEK : public Currency {
      public:
        EEK() {
            static boost::shared_ptr<Data> eekData(
                                        new Data("Estonian kroon", "EEK", 233,
                                                 "KR", "", 100,
                                                 Rounding(),
                                                 "%1$.2f %2%"));
            data_ = eekData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to EEK */
    typedef EEK EEKCurrency;
    #endif

    //! European Euro
    /*! The ISO three-letter code is EUR; the numeric code is 978.
        It is divided into 100 cents.

        \ingroup currencies
    */
    class EUR : public Currency {
      public:
        EUR() {
            static boost::shared_ptr<Data> eurData(
                                         new Data("European Euro", "EUR", 978,
                                                  "", "", 100,
                                                  ClosestRounding(2),
                                                  "%2% %1$.2f"));
            data_ = eurData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to EUR */
    typedef EUR EURCurrency;
    #endif

    //! British pound sterling
    /*! The ISO three-letter code is GBP; the numeric code is 826.
        It is divided into 100 pence.

        \ingroup currencies
    */
    class GBP : public Currency {
      public:
        GBP() {
            static boost::shared_ptr<Data> gbpData(
                                new Data("British pound sterling", "GBP", 826,
                                         "\xA3", "p", 100,
                                         Rounding(),
                                         "%3% %1$.2f"));
            data_ = gbpData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to GBP */
    typedef GBP GBPCurrency;
    #endif

    //! Hungarian forint
    /*! The ISO three-letter code is HUF; the numeric code is 348.
        It has no subdivisions.

        \ingroup currencies
    */
    class HUF : public Currency {
      public:
        HUF() {
            static boost::shared_ptr<Data> hufData(
                                      new Data("Hungarian forint", "HUF", 348,
                                               "Ft", "", 1,
                                               Rounding(),
                                               "%1$.0f %3%"));
            data_ = hufData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to HUF */
    typedef HUF HUFCurrency;
    #endif

    //! Iceland krona
    /*! The ISO three-letter code is ISK; the numeric code is 352.
        It is divided in 100 aurar.

        \ingroup currencies
    */
    class ISK : public Currency {
      public:
        ISK() {
            static boost::shared_ptr<Data> iskData(
                                         new Data("Iceland krona", "ISK", 352,
                                                  "IKr", "", 100,
                                                  Rounding(),
                                                  "%1$.2f %3%"));
            data_ = iskData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to ISK */
    typedef ISK ISKCurrency;
    #endif

    //! Lithuanian litas
    /*! The ISO three-letter code is LTL; the numeric code is 440.
        It is divided in 100 centu.

        \ingroup currencies
    */
    class LTL : public Currency {
      public:
        LTL() {
            static boost::shared_ptr<Data> ltlData(
                                      new Data("Lithuanian litas", "LTL", 440,
                                               "Lt", "", 100,
                                               Rounding(),
                                               "%1$.2f %3%"));
            data_ = ltlData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to LTL */
    typedef LTL LTLCurrency;
    #endif

    //! Latvian lat
    /*! The ISO three-letter code is LVL; the numeric code is 428.
        It is divided in 100 santims.

        \ingroup currencies
    */
    class LVL : public Currency {
      public:
        LVL() {
            static boost::shared_ptr<Data> lvlData(
                                           new Data("Latvian lat", "LVL", 428,
                                                    "Ls", "", 100,
                                                    Rounding(),
                                                    "%3% %1$.2f"));
            data_ = lvlData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to LVL */
    typedef LVL LVLCurrency;
    #endif

    //! Maltese lira
    /*! The ISO three-letter code is MTL; the numeric code is 470.
        It is divided in 100 cents.

        \ingroup currencies
    */
    class MTL : public Currency {
      public:
        MTL() {
            static boost::shared_ptr<Data> mtlData(
                                          new Data("Maltese lira", "MTL", 470,
                                                   "Lm", "", 100,
                                                   Rounding(),
                                                   "%3% %1$.2f"));
            data_ = mtlData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to MTL */
    typedef MTL MTLCurrency;
    #endif

    //! Norwegian krone
    /*! The ISO three-letter code is NOK; the numeric code is 578.
        It is divided in 100 øre.

        \ingroup currencies
    */
    class NOK : public Currency {
      public:
        NOK() {
            static boost::shared_ptr<Data> nokData(
                                       new Data("Norwegian krone", "NOK", 578,
                                                "NKr", "", 100,
                                                Rounding(),
                                                "%3% %1$.2f"));
            data_ = nokData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to NOK */
    typedef NOK NOKCurrency;
    #endif

    //! Polish zloty
    /*! The ISO three-letter code is PLN; the numeric code is 985.
        It is divided in 100 groszy.

        \ingroup currencies
    */
    class PLN : public Currency {
      public:
        PLN() {
            static boost::shared_ptr<Data> plnData(
                                          new Data("Polish zloty", "PLN", 985,
                                                   "zl", "", 100,
                                                   Rounding(),
                                                   "%1$.2f %3%"));
            data_ = plnData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to PLN */
    typedef PLN PLNCurrency;
    #endif

    //! Romanian leu
    /*! The ISO three-letter code is ROL; the numeric code is 642.
        It is divided in 100 bani.

        \ingroup currencies
    */
    class ROL : public Currency {
      public:
        ROL() {
            static boost::shared_ptr<Data> rolData(
                                          new Data("Romanian leu", "ROL", 642,
                                                   "L", "", 100,
                                                   Rounding(),
                                                   "%1$.2f %3%"));
            data_ = rolData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to ROL */
    typedef ROL ROLCurrency;
    #endif

    //! Swedish krona
    /*! The ISO three-letter code is SEK; the numeric code is 752.
        It is divided in 100 öre.

        \ingroup currencies
    */
    class SEK : public Currency {
      public:
        SEK() {
            static boost::shared_ptr<Data> sekData(
                                         new Data("Swedish krona", "SEK", 752,
                                                  "kr", "", 100,
                                                  Rounding(),
                                                  "%1$.2f %3%"));
            data_ = sekData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to SEK */
    typedef SEK SEKCurrency;
    #endif

    //! Slovenian tolar
    /*! The ISO three-letter code is SIT; the numeric code is 705.
        It is divided in 100 stotinov.

        \ingroup currencies
    */
    class SIT : public Currency {
      public:
        SIT() {
            static boost::shared_ptr<Data> sitData(
                                       new Data("Slovenian tolar", "SIT", 705,
                                                "SlT", "", 100,
                                                Rounding(),
                                                "%1$.2f %3%"));
            data_ = sitData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to SIT */
    typedef SIT SITCurrency;
    #endif

    //! Slovak koruna
    /*! The ISO three-letter code is SKK; the numeric code is 703.
        It is divided in 100 halierov.

        \ingroup currencies
    */
    class SKK : public Currency {
      public:
        SKK() {
            static boost::shared_ptr<Data> skkData(
                                         new Data("Slovak koruna", "SKK", 703,
                                                  "Sk", "", 100,
                                                  Rounding(),
                                                  "%1$.2f %3%"));
            data_ = skkData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to SKK */
    typedef SKK SKKCurrency;
    #endif

    //! Turkish lira
    /*! The ISO three-letter code is TRL; the numeric code is 792.
        It is divided in 100 kurus.

        Obsoleted by the new Turkish lira since 2005.

        \ingroup currencies
    */
    class TRL : public Currency {
      public:
        TRL() {
            static boost::shared_ptr<Data> trlData(
                                          new Data("Turkish lira", "TRL", 792,
                                                   "TL", "", 100,
                                                   Rounding(),
                                                   "%1$.0f %3%"));
            data_ = trlData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to TRL */
    typedef TRL TRLCurrency;
    #endif

    //! New Turkish lira
    /*! The ISO three-letter code is TRY; the numeric code is 949.
        It is divided in 100 new kurus.

        \ingroup currencies
    */
    class TRY : public Currency {
      public:
        TRY() {
            static boost::shared_ptr<Data> tryData(
                                      new Data("New Turkish lira", "TRY", 949,
                                               "YTL", "", 100,
                                               Rounding(),
                                               "%1$.2f %3%"));
            data_ = tryData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to TRY */
    typedef TRY TRYCurrency;
    #endif


    // currencies obsoleted by Euro

    //! Austrian shilling
    /*! The ISO three-letter code was ATS; the numeric code was 40.
        It was divided in 100 groschen.

        \ingroup currencies
    */
    class ATS : public Currency {
      public:
        ATS() {
            static boost::shared_ptr<Data> atsData(
                                      new Data("Austrian shilling", "ATS", 40,
                                               "", "", 100,
                                               Rounding(),
                                               "%2% %1$.2f",
                                               EUR()));
            data_ = atsData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to ATS */
    typedef ATS ATSCurrency;
    #endif

    //! Belgian franc
    /*! The ISO three-letter code is BEF; the numeric code is 56.
        It has no subdivisions.

        \ingroup currencies
    */
    class BEF : public Currency {
      public:
        BEF() {
            static boost::shared_ptr<Data> befData(
                                          new Data("Belgian franc", "BEF", 56,
                                                   "", "", 1,
                                                   Rounding(),
                                                   "%2% %1$.0f",
                                                   EUR()));
            data_ = befData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to BEF */
    typedef BEF BEFCurrency;
    #endif

    //! Deutsche mark
    /*! The ISO three-letter code was DEM; the numeric code was 276.
        It was divided into 100 pfennig.

        \ingroup currencies
    */
    class DEM : public Currency {
      public:
        DEM() {
            static boost::shared_ptr<Data> demData(
                                         new Data("Deutsche mark", "DEM", 276,
                                                  "DM", "", 100,
                                                  Rounding(),
                                                  "%1$.2f %3%",
                                                  EUR()));
            data_ = demData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to DEM */
    typedef DEM DEMCurrency;
    #endif

    //! Spanish peseta
    /*! The ISO three-letter code is ESP; the numeric code is 724.
        It is divided in 100 centimos.

        \ingroup currencies
    */
    class ESP : public Currency {
      public:
        ESP() {
            static boost::shared_ptr<Data> espData(
                                        new Data("Spanish peseta", "ESP", 724,
                                                 "Pta", "", 100,
                                                 Rounding(),
                                                 "%1$.0f %3%",
                                                 EUR()));
            data_ = espData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to ESP */
    typedef ESP ESPCurrency;
    #endif

    //! Finnish markka
    /*! The ISO three-letter code is FIM; the numeric code is 246.
        It is divided in 100 penniä.

        \ingroup currencies
    */
    class FIM : public Currency {
      public:
        FIM() {
            static boost::shared_ptr<Data> fimData(
                                        new Data("Finnish markka", "FIM", 246,
                                                 "mk", "", 100,
                                                 Rounding(),
                                                 "%1$.2f %3%",
                                                 EUR()));
            data_ = fimData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to FIM */
    typedef FIM FIMCurrency;
    #endif

    //! French franc
    /*! The ISO three-letter code is FRF; the numeric code is 250.
        It is divided in 100 centimes.

        \ingroup currencies
    */
    class FRF : public Currency {
      public:
        FRF() {
            static boost::shared_ptr<Data> frfData(
                                          new Data("French franc", "FRF", 250,
                                                   "", "", 100,
                                                   Rounding(),
                                                   "%1$.2f %2%",
                                                   EUR()));
            data_ = frfData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to FRF */
    typedef FRF FRFCurrency;
    #endif

    //! Greek drachma
    /*! The ISO three-letter code is GRD; the numeric code is 300.
        It is divided in 100 lepta.

        \ingroup currencies
    */
    class GRD : public Currency {
      public:
        GRD() {
            static boost::shared_ptr<Data> grdData(
                                         new Data("Greek drachma", "GRD", 300,
                                                  "", "", 100,
                                                  Rounding(),
                                                  "%1$.2f %2%",
                                                  EUR()));
            data_ = grdData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to GRD */
    typedef GRD GRDCurrency;
    #endif

    //! Irish punt
    /*! The ISO three-letter code is IEP; the numeric code is 372.
        It is divided in 100 pence.

        \ingroup currencies
    */
    class IEP : public Currency {
      public:
        IEP() {
            static boost::shared_ptr<Data> iepData(
                                            new Data("Irish punt", "IEP", 372,
                                                     "", "", 100,
                                                     Rounding(),
                                                     "%2% %1$.2f",
                                                     EUR()));
            data_ = iepData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to IEP */
    typedef IEP IEPCurrency;
    #endif

    //! Italian lira
    /*! The ISO three-letter code was ITL; the numeric code was 380.
        It had no subdivisions.

        \ingroup currencies
    */
    class ITL : public Currency {
      public:
        ITL() {
            static boost::shared_ptr<Data> itlData(
                                          new Data("Italian lira", "ITL", 380,
                                                   "L", "", 1,
                                                   Rounding(),
                                                   "%3% %1$.0f",
                                                   EUR()));
            data_ = itlData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to ITL */
    typedef ITL ITLCurrency;
    #endif

    //! Luxembourg franc
    /*! The ISO three-letter code is LUF; the numeric code is 442.
        It is divided in 100 centimes.

        \ingroup currencies
    */
    class LUF : public Currency {
      public:
        LUF() {
            static boost::shared_ptr<Data> lufData(
                                      new Data("Luxembourg franc", "LUF", 442,
                                               "F", "", 100,
                                               Rounding(),
                                               "%1$.0f %3%",
                                               EUR()));
            data_ = lufData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to LUF */
    typedef LUF LUFCurrency;
    #endif

    //! Dutch guilder
    /*! The ISO three-letter code is NLG; the numeric code is 528.
        It is divided in 100 cents.

        \ingroup currencies
    */
    class NLG : public Currency {
      public:
        NLG() {
            static boost::shared_ptr<Data> nlgData(
                                         new Data("Dutch guilder", "NLG", 528,
                                                  "f", "", 100,
                                                  Rounding(),
                                                  "%3% %1$.2f",
                                                  EUR()));
            data_ = nlgData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to NLG */
    typedef NLG NLGCurrency;
    #endif

    //! Portuguese escudo
    /*! The ISO three-letter code is PTE; the numeric code is 620.
        It is divided in 100 centavos.

        \ingroup currencies
    */
    class PTE : public Currency {
      public:
        PTE() {
            static boost::shared_ptr<Data> pteData(
                                     new Data("Portuguese escudo", "PTE", 620,
                                              "Esc", "", 100,
                                              Rounding(),
                                              "%1$.0f %3%",
                                              EUR()));
            data_ = pteData;
        }
    };
    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to PTE */
    typedef PTE PTECurrency;
    #endif

}


#endif
