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
    class BGLCurrency : public Currency {
      public:
        BGLCurrency() {
            static boost::shared_ptr<Data> bglData(
                                         new Data("Bulgarian lev", "BGL", 100,
                                                  "lv", "", 100,
                                                  Rounding(),
                                                  "%1$.2f %3%"));
            data_ = bglData;
        }
    };

    //! Belarussian ruble
    /*! The ISO three-letter code is BYR; the numeric code is 974.
        It has no subdivisions.

        \ingroup currencies
    */
    class BYRCurrency : public Currency {
      public:
        BYRCurrency() {
            static boost::shared_ptr<Data> byrData(
                                     new Data("Belarussian ruble", "BYR", 974,
                                              "BR", "", 1,
                                              Rounding(),
                                              "%2% %1$.0f"));
            data_ = byrData;
        }
    };

    //! Swiss franc
    /*! The ISO three-letter code is CHF; the numeric code is 756.
        It is divided into 100 cents.

        \ingroup currencies
    */
    class CHFCurrency : public Currency {
      public:
        CHFCurrency() {
            static boost::shared_ptr<Data> chfData(
                                           new Data("Swiss franc", "CHF", 756,
                                                    "SwF", "", 100,
                                                    Rounding(),
                                                    "%3% %1$.2f"));
            data_ = chfData;
        }
    };

    //! Cyprus pound
    /*! The ISO three-letter code is CYP; the numeric code is 196.
        It is divided in 100 cents.

        \ingroup currencies
    */
    class CYPCurrency : public Currency {
      public:
        CYPCurrency() {
            static boost::shared_ptr<Data> cypData(
                                          new Data("Cyprus pound", "CYP", 196,
                                                   "\xA3" "C", "", 100,
                                                   Rounding(),
                                                   "%3% %1$.2f"));
            data_ = cypData;
        }
    };

    //! Czech koruna
    /*! The ISO three-letter code is CZK; the numeric code is 203.
        It is divided in 100 haleru.

        \ingroup currencies
    */
    class CZKCurrency : public Currency {
      public:
        CZKCurrency() {
            static boost::shared_ptr<Data> czkData(
                                          new Data("Czech koruna", "CZK", 203,
                                                   "Kc", "", 100,
                                                   Rounding(),
                                                   "%1$.2f %3%"));
            data_ = czkData;
        }
    };

    //! Danish krone
    /*! The ISO three-letter code is DKK; the numeric code is 208.
        It is divided in 100 øre.

        \ingroup currencies
    */
    class DKKCurrency : public Currency {
      public:
        DKKCurrency() {
            static boost::shared_ptr<Data> dkkData(
                                          new Data("Danish krone", "DKK", 208,
                                                   "Dkr", "", 100,
                                                   Rounding(),
                                                   "%3% %1$.2f"));
            data_ = dkkData;
        }
    };

    //! Estonian kroon
    /*! The ISO three-letter code is EEK; the numeric code is 233.
        It is divided in 100 senti.

        \ingroup currencies
    */
    class EEKCurrency : public Currency {
      public:
        EEKCurrency() {
            static boost::shared_ptr<Data> eekData(
                                        new Data("Estonian kroon", "EEK", 233,
                                                 "KR", "", 100,
                                                 Rounding(),
                                                 "%1$.2f %2%"));
            data_ = eekData;
        }
    };

    //! European Euro
    /*! The ISO three-letter code is EUR; the numeric code is 978.
        It is divided into 100 cents.

        \ingroup currencies
    */
    class EURCurrency : public Currency {
      public:
        EURCurrency() {
            static boost::shared_ptr<Data> eurData(
                                         new Data("European Euro", "EUR", 978,
                                                  "", "", 100,
                                                  ClosestRounding(2),
                                                  "%2% %1$.2f"));
            data_ = eurData;
        }
    };

    //! British pound sterling
    /*! The ISO three-letter code is GBP; the numeric code is 826.
        It is divided into 100 pence.

        \ingroup currencies
    */
    class GBPCurrency : public Currency {
      public:
        GBPCurrency() {
            static boost::shared_ptr<Data> gbpData(
                                new Data("British pound sterling", "GBP", 826,
                                         "\xA3", "p", 100,
                                         Rounding(),
                                         "%3% %1$.2f"));
            data_ = gbpData;
        }
    };

    //! Hungarian forint
    /*! The ISO three-letter code is HUF; the numeric code is 348.
        It has no subdivisions.

        \ingroup currencies
    */
    class HUFCurrency : public Currency {
      public:
        HUFCurrency() {
            static boost::shared_ptr<Data> hufData(
                                      new Data("Hungarian forint", "HUF", 348,
                                               "Ft", "", 1,
                                               Rounding(),
                                               "%1$.0f %3%"));
            data_ = hufData;
        }
    };

    //! Iceland krona
    /*! The ISO three-letter code is ISK; the numeric code is 352.
        It is divided in 100 aurar.

        \ingroup currencies
    */
    class ISKCurrency : public Currency {
      public:
        ISKCurrency() {
            static boost::shared_ptr<Data> iskData(
                                         new Data("Iceland krona", "ISK", 352,
                                                  "IKr", "", 100,
                                                  Rounding(),
                                                  "%1$.2f %3%"));
            data_ = iskData;
        }
    };

    //! Lithuanian litas
    /*! The ISO three-letter code is LTL; the numeric code is 440.
        It is divided in 100 centu.

        \ingroup currencies
    */
    class LTLCurrency : public Currency {
      public:
        LTLCurrency() {
            static boost::shared_ptr<Data> ltlData(
                                      new Data("Lithuanian litas", "LTL", 440,
                                               "Lt", "", 100,
                                               Rounding(),
                                               "%1$.2f %3%"));
            data_ = ltlData;
        }
    };

    //! Latvian lat
    /*! The ISO three-letter code is LVL; the numeric code is 428.
        It is divided in 100 santims.

        \ingroup currencies
    */
    class LVLCurrency : public Currency {
      public:
        LVLCurrency() {
            static boost::shared_ptr<Data> lvlData(
                                           new Data("Latvian lat", "LVL", 428,
                                                    "Ls", "", 100,
                                                    Rounding(),
                                                    "%3% %1$.2f"));
            data_ = lvlData;
        }
    };

    //! Maltese lira
    /*! The ISO three-letter code is MTL; the numeric code is 470.
        It is divided in 100 cents.

        \ingroup currencies
    */
    class MTLCurrency : public Currency {
      public:
        MTLCurrency() {
            static boost::shared_ptr<Data> mtlData(
                                          new Data("Maltese lira", "MTL", 470,
                                                   "Lm", "", 100,
                                                   Rounding(),
                                                   "%3% %1$.2f"));
            data_ = mtlData;
        }
    };

    //! Norwegian krone
    /*! The ISO three-letter code is NOK; the numeric code is 578.
        It is divided in 100 øre.

        \ingroup currencies
    */
    class NOKCurrency : public Currency {
      public:
        NOKCurrency() {
            static boost::shared_ptr<Data> nokData(
                                       new Data("Norwegian krone", "NOK", 578,
                                                "NKr", "", 100,
                                                Rounding(),
                                                "%3% %1$.2f"));
            data_ = nokData;
        }
    };

    //! Polish zloty
    /*! The ISO three-letter code is PLN; the numeric code is 985.
        It is divided in 100 groszy.

        \ingroup currencies
    */
    class PLNCurrency : public Currency {
      public:
        PLNCurrency() {
            static boost::shared_ptr<Data> plnData(
                                          new Data("Polish zloty", "PLN", 985,
                                                   "zl", "", 100,
                                                   Rounding(),
                                                   "%1$.2f %3%"));
            data_ = plnData;
        }
    };

    //! Romanian leu
    /*! The ISO three-letter code is ROL; the numeric code is 642.
        It is divided in 100 bani.

        \ingroup currencies
    */
    class ROLCurrency : public Currency {
      public:
        ROLCurrency() {
            static boost::shared_ptr<Data> rolData(
                                          new Data("Romanian leu", "ROL", 642,
                                                   "L", "", 100,
                                                   Rounding(),
                                                   "%1$.2f %3%"));
            data_ = rolData;
        }
    };

    //! Swedish krona
    /*! The ISO three-letter code is SEK; the numeric code is 752.
        It is divided in 100 öre.

        \ingroup currencies
    */
    class SEKCurrency : public Currency {
      public:
        SEKCurrency() {
            static boost::shared_ptr<Data> sekData(
                                         new Data("Swedish krona", "SEK", 752,
                                                  "kr", "", 100,
                                                  Rounding(),
                                                  "%1$.2f %3%"));
            data_ = sekData;
        }
    };

    //! Slovenian tolar
    /*! The ISO three-letter code is SIT; the numeric code is 705.
        It is divided in 100 stotinov.

        \ingroup currencies
    */
    class SITCurrency : public Currency {
      public:
        SITCurrency() {
            static boost::shared_ptr<Data> sitData(
                                       new Data("Slovenian tolar", "SIT", 705,
                                                "SlT", "", 100,
                                                Rounding(),
                                                "%1$.2f %3%"));
            data_ = sitData;
        }
    };

    //! Slovak koruna
    /*! The ISO three-letter code is SKK; the numeric code is 703.
        It is divided in 100 halierov.

        \ingroup currencies
    */
    class SKKCurrency : public Currency {
      public:
        SKKCurrency() {
            static boost::shared_ptr<Data> skkData(
                                         new Data("Slovak koruna", "SKK", 703,
                                                  "Sk", "", 100,
                                                  Rounding(),
                                                  "%1$.2f %3%"));
            data_ = skkData;
        }
    };

    //! Turkish lira
    /*! The ISO three-letter code is TRL; the numeric code is 792.
        It is divided in 100 kurus.

        Obsoleted by the new Turkish lira since 2005.

        \ingroup currencies
    */
    class TRLCurrency : public Currency {
      public:
        TRLCurrency() {
            static boost::shared_ptr<Data> trlData(
                                          new Data("Turkish lira", "TRL", 792,
                                                   "TL", "", 100,
                                                   Rounding(),
                                                   "%1$.0f %3%"));
            data_ = trlData;
        }
    };

    //! New Turkish lira
    /*! The ISO three-letter code is TRY; the numeric code is 949.
        It is divided in 100 new kurus.

        \ingroup currencies
    */
    class TRYCurrency : public Currency {
      public:
        TRYCurrency() {
            static boost::shared_ptr<Data> tryData(
                                      new Data("New Turkish lira", "TRY", 949,
                                               "YTL", "", 100,
                                               Rounding(),
                                               "%1$.2f %3%"));
            data_ = tryData;
        }
    };


    // currencies obsoleted by Euro

    //! Austrian shilling
    /*! The ISO three-letter code was ATS; the numeric code was 40.
        It was divided in 100 groschen.

        Obsoleted by the Euro since 1999.

        \ingroup currencies
    */
    class ATSCurrency : public Currency {
      public:
        ATSCurrency() {
            static boost::shared_ptr<Data> atsData(
                                      new Data("Austrian shilling", "ATS", 40,
                                               "", "", 100,
                                               Rounding(),
                                               "%2% %1$.2f",
                                               EURCurrency()));
            data_ = atsData;
        }
    };

    //! Belgian franc
    /*! The ISO three-letter code was BEF; the numeric code was 56.
        It had no subdivisions.

        Obsoleted by the Euro since 1999.

        \ingroup currencies
    */
    class BEFCurrency : public Currency {
      public:
        BEFCurrency() {
            static boost::shared_ptr<Data> befData(
                                          new Data("Belgian franc", "BEF", 56,
                                                   "", "", 1,
                                                   Rounding(),
                                                   "%2% %1$.0f",
                                                   EURCurrency()));
            data_ = befData;
        }
    };

    //! Deutsche mark
    /*! The ISO three-letter code was DEM; the numeric code was 276.
        It was divided into 100 pfennig.

        Obsoleted by the Euro since 1999.

        \ingroup currencies
    */
    class DEMCurrency : public Currency {
      public:
        DEMCurrency() {
            static boost::shared_ptr<Data> demData(
                                         new Data("Deutsche mark", "DEM", 276,
                                                  "DM", "", 100,
                                                  Rounding(),
                                                  "%1$.2f %3%",
                                                  EURCurrency()));
            data_ = demData;
        }
    };

    //! Spanish peseta
    /*! The ISO three-letter code was ESP; the numeric code was 724.
        It was divided in 100 centimos.

        Obsoleted by the Euro since 1999.

        \ingroup currencies
    */
    class ESPCurrency : public Currency {
      public:
        ESPCurrency() {
            static boost::shared_ptr<Data> espData(
                                        new Data("Spanish peseta", "ESP", 724,
                                                 "Pta", "", 100,
                                                 Rounding(),
                                                 "%1$.0f %3%",
                                                 EURCurrency()));
            data_ = espData;
        }
    };

    //! Finnish markka
    /*! The ISO three-letter code was FIM; the numeric code was 246.
        It was divided in 100 penniä.

        Obsoleted by the Euro since 1999.

        \ingroup currencies
    */
    class FIMCurrency : public Currency {
      public:
        FIMCurrency() {
            static boost::shared_ptr<Data> fimData(
                                        new Data("Finnish markka", "FIM", 246,
                                                 "mk", "", 100,
                                                 Rounding(),
                                                 "%1$.2f %3%",
                                                 EURCurrency()));
            data_ = fimData;
        }
    };

    //! French franc
    /*! The ISO three-letter code was FRF; the numeric code was 250.
        It was divided in 100 centimes.

        Obsoleted by the Euro since 1999.

        \ingroup currencies
    */
    class FRFCurrency : public Currency {
      public:
        FRFCurrency() {
            static boost::shared_ptr<Data> frfData(
                                          new Data("French franc", "FRF", 250,
                                                   "", "", 100,
                                                   Rounding(),
                                                   "%1$.2f %2%",
                                                   EURCurrency()));
            data_ = frfData;
        }
    };

    //! Greek drachma
    /*! The ISO three-letter code was GRD; the numeric code was 300.
        It was divided in 100 lepta.

        Obsoleted by the Euro since 2001.

        \ingroup currencies
    */
    class GRDCurrency : public Currency {
      public:
        GRDCurrency() {
            static boost::shared_ptr<Data> grdData(
                                         new Data("Greek drachma", "GRD", 300,
                                                  "", "", 100,
                                                  Rounding(),
                                                  "%1$.2f %2%",
                                                  EURCurrency()));
            data_ = grdData;
        }
    };

    //! Irish punt
    /*! The ISO three-letter code was IEP; the numeric code was 372.
        It was divided in 100 pence.

        Obsoleted by the Euro since 1999.

        \ingroup currencies
    */
    class IEPCurrency : public Currency {
      public:
        IEPCurrency() {
            static boost::shared_ptr<Data> iepData(
                                            new Data("Irish punt", "IEP", 372,
                                                     "", "", 100,
                                                     Rounding(),
                                                     "%2% %1$.2f",
                                                     EURCurrency()));
            data_ = iepData;
        }
    };

    //! Italian lira
    /*! The ISO three-letter code was ITL; the numeric code was 380.
        It had no subdivisions.

        Obsoleted by the Euro since 1999.

        \ingroup currencies
    */
    class ITLCurrency : public Currency {
      public:
        ITLCurrency() {
            static boost::shared_ptr<Data> itlData(
                                          new Data("Italian lira", "ITL", 380,
                                                   "L", "", 1,
                                                   Rounding(),
                                                   "%3% %1$.0f",
                                                   EURCurrency()));
            data_ = itlData;
        }
    };

    //! Luxembourg franc
    /*! The ISO three-letter code was LUF; the numeric code was 442.
        It was divided in 100 centimes.

        Obsoleted by the Euro since 1999.

        \ingroup currencies
    */
    class LUFCurrency : public Currency {
      public:
        LUFCurrency() {
            static boost::shared_ptr<Data> lufData(
                                      new Data("Luxembourg franc", "LUF", 442,
                                               "F", "", 100,
                                               Rounding(),
                                               "%1$.0f %3%",
                                               EURCurrency()));
            data_ = lufData;
        }
    };

    //! Dutch guilder
    /*! The ISO three-letter code was NLG; the numeric code was 528.
        It was divided in 100 cents.

        Obsoleted by the Euro since 1999.

        \ingroup currencies
    */
    class NLGCurrency : public Currency {
      public:
        NLGCurrency() {
            static boost::shared_ptr<Data> nlgData(
                                         new Data("Dutch guilder", "NLG", 528,
                                                  "f", "", 100,
                                                  Rounding(),
                                                  "%3% %1$.2f",
                                                  EURCurrency()));
            data_ = nlgData;
        }
    };

    //! Portuguese escudo
    /*! The ISO three-letter code was PTE; the numeric code was 620.
        It was divided in 100 centavos.

        Obsoleted by the Euro since 1999.

        \ingroup currencies
    */
    class PTECurrency : public Currency {
      public:
        PTECurrency() {
            static boost::shared_ptr<Data> pteData(
                                     new Data("Portuguese escudo", "PTE", 620,
                                              "Esc", "", 100,
                                              Rounding(),
                                              "%1$.0f %3%",
                                              EURCurrency()));
            data_ = pteData;
        }
    };

}


#endif
