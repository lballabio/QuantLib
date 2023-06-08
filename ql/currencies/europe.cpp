/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2016 Quaternion Risk Management Ltd

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

/*
    Data from http://fx.sauder.ubc.ca/currency_table.html
    and http://www.thefinancials.com/vortex/CurrencyFormats.html
*/

#include <ql/currencies/europe.hpp>

namespace QuantLib {

    // Bulgarian lev
    /* The ISO three-letter code is BGL; the numeric code is 100.
       It is divided in 100 stotinki.
    */
    BGLCurrency::BGLCurrency() {
        static ext::shared_ptr<Data> bglData(
                                     new Data("Bulgarian lev", "BGL", 100,
                                              "lv", "", 100,
                                              Rounding(),
                                              "%1$.2f %3%"));
        data_ = bglData;
    }

    // Belarussian ruble
    /* The ISO three-letter code is BYR; the numeric code is 974.
       It has no subdivisions.
    */
    BYRCurrency::BYRCurrency() {
        static ext::shared_ptr<Data> byrData(
                                 new Data("Belarussian ruble", "BYR", 974,
                                          "BR", "", 1,
                                          Rounding(),
                                          "%2% %1$.0f"));
        data_ = byrData;
    }

    // Swiss franc
    /* The ISO three-letter code is CHF; the numeric code is 756.
       It is divided into 100 cents.
    */
    CHFCurrency::CHFCurrency() {
        static ext::shared_ptr<Data> chfData(
                                       new Data("Swiss franc", "CHF", 756,
                                                "SwF", "", 100,
                                                Rounding(),
                                                "%3% %1$.2f"));
        data_ = chfData;
    }

    // Cyprus pound
    /* The ISO three-letter code is CYP; the numeric code is 196.
       It is divided in 100 cents.
    */
    CYPCurrency::CYPCurrency() {
        static ext::shared_ptr<Data> cypData(
                                      new Data("Cyprus pound", "CYP", 196,
                                               "\xA3" "C", "", 100,
                                               Rounding(),
                                               "%3% %1$.2f"));
        data_ = cypData;
    }

    // Czech koruna
    /* The ISO three-letter code is CZK; the numeric code is 203.
       It is divided in 100 haleru.
    */
    CZKCurrency::CZKCurrency() {
        static ext::shared_ptr<Data> czkData(
                                      new Data("Czech koruna", "CZK", 203,
                                               "Kc", "", 100,
                                               Rounding(),
                                               "%1$.2f %3%"));
        data_ = czkData;
    }

    // Danish krone
    /* The ISO three-letter code is DKK; the numeric code is 208.
       It is divided in 100 øre.
    */
    DKKCurrency::DKKCurrency() {
        static ext::shared_ptr<Data> dkkData(
                                      new Data("Danish krone", "DKK", 208,
                                               "Dkr", "", 100,
                                               Rounding(),
                                               "%3% %1$.2f"));
        data_ = dkkData;
    }

    // Estonian kroon
    /* The ISO three-letter code is EEK; the numeric code is 233.
       It is divided in 100 senti.
    */
    EEKCurrency::EEKCurrency() {
        static ext::shared_ptr<Data> eekData(
                                    new Data("Estonian kroon", "EEK", 233,
                                             "KR", "", 100,
                                             Rounding(),
                                             "%1$.2f %2%"));
        data_ = eekData;
    }

    // European Euro
    /* The ISO three-letter code is EUR; the numeric code is 978.
       It is divided into 100 cents.
    */
    EURCurrency::EURCurrency() {
        static ext::shared_ptr<Data> eurData(
                                     new Data("European Euro", "EUR", 978,
                                              "", "", 100,
                                              ClosestRounding(2),
                                              "%2% %1$.2f"));
        data_ = eurData;
    }

    // British pound sterling
    /* The ISO three-letter code is GBP; the numeric code is 826.
       It is divided into 100 pence.
    */
    GBPCurrency::GBPCurrency() {
        static ext::shared_ptr<Data> gbpData(
                            new Data("British pound sterling", "GBP", 826,
                                     "\xA3", "p", 100,
                                     Rounding(),
                                     "%3% %1$.2f"));
        data_ = gbpData;
    }

    // Hungarian forint
    /* The ISO three-letter code is HUF; the numeric code is 348.
       It has no subdivisions.
    */
    HUFCurrency::HUFCurrency() {
        static ext::shared_ptr<Data> hufData(
                                  new Data("Hungarian forint", "HUF", 348,
                                           "Ft", "", 1,
                                           Rounding(),
                                           "%1$.0f %3%"));
        data_ = hufData;
    }

    // Icelandic krona
    /* The ISO three-letter code is ISK; the numeric code is 352.
       It is divided in 100 aurar.
    */
    ISKCurrency::ISKCurrency() {
        static ext::shared_ptr<Data> iskData(
                                     new Data("Iceland krona", "ISK", 352,
                                              "IKr", "", 100,
                                              Rounding(),
                                              "%1$.2f %3%"));
        data_ = iskData;
    }

    // Lithuanian litas
    /* The ISO three-letter code is LTL; the numeric code is 440.
       It is divided in 100 centu.
    */
    LTLCurrency::LTLCurrency() {
        static ext::shared_ptr<Data> ltlData(
                                  new Data("Lithuanian litas", "LTL", 440,
                                           "Lt", "", 100,
                                           Rounding(),
                                           "%1$.2f %3%"));
        data_ = ltlData;
    }

    // Latvian lat
    /* The ISO three-letter code is LVL; the numeric code is 428.
       It is divided in 100 santims.
    */
    LVLCurrency::LVLCurrency() {
        static ext::shared_ptr<Data> lvlData(
                                       new Data("Latvian lat", "LVL", 428,
                                                "Ls", "", 100,
                                                Rounding(),
                                                "%3% %1$.2f"));
        data_ = lvlData;
    }

    // Norwegian krone
    /* The ISO three-letter code is NOK; the numeric code is 578.
       It is divided in 100 øre.
    */
    NOKCurrency::NOKCurrency() {
        static ext::shared_ptr<Data> nokData(
                                   new Data("Norwegian krone", "NOK", 578,
                                            "NKr", "", 100,
                                            Rounding(),
                                            "%3% %1$.2f"));
        data_ = nokData;
    }

    // Polish zloty
    /* The ISO three-letter code is PLN; the numeric code is 985.
       It is divided in 100 groszy.
    */
    PLNCurrency::PLNCurrency() {
        static ext::shared_ptr<Data> plnData(
                                      new Data("Polish zloty", "PLN", 985,
                                               "zl", "", 100,
                                               Rounding(),
                                               "%1$.2f %3%"));
        data_ = plnData;
    }

    // Romanian leu
    /* The ISO three-letter code was ROL; the numeric code was 642.
       It was divided in 100 bani.
    */
    ROLCurrency::ROLCurrency() {
        static ext::shared_ptr<Data> rolData(
                                      new Data("Romanian leu", "ROL", 642,
                                               "L", "", 100,
                                               Rounding(),
                                               "%1$.2f %3%"));
        data_ = rolData;
    }

    // Romanian new leu
    /* The ISO three-letter code is RON; the numeric code is 946.
       It is divided in 100 bani.
    */
    RONCurrency::RONCurrency() {
        static ext::shared_ptr<Data> ronData(
                                      new Data("Romanian new leu",
                                               "RON", 946,
                                               "L", "", 100,
                                               Rounding(),
                                               "%1$.2f %3%"));
        data_ = ronData;
    }

    // Russian ruble
    /* The ISO three-letter code is RUB; the numeric code is 643.
       It is divided in 100 kopeyki.
    */
    RUBCurrency::RUBCurrency() {
        static ext::shared_ptr<Data> rubData(
                                      new Data("Russian ruble",
                                               "RUB", 643,
                                               "", "", 100,
                                               Rounding(),
                                               "%1$.2f %2%"));
        data_ = rubData;
    }

    // Swedish krona
    /* The ISO three-letter code is SEK; the numeric code is 752.
       It is divided in 100 öre.
    */
    SEKCurrency::SEKCurrency() {
        static ext::shared_ptr<Data> sekData(
                                     new Data("Swedish krona", "SEK", 752,
                                              "kr", "", 100,
                                              Rounding(),
                                              "%1$.2f %3%"));
        data_ = sekData;
    }

    // Slovenian tolar
    /* The ISO three-letter code is SIT; the numeric code is 705.
       It is divided in 100 stotinov.
    */
    SITCurrency::SITCurrency() {
        static ext::shared_ptr<Data> sitData(
                                   new Data("Slovenian tolar", "SIT", 705,
                                            "SlT", "", 100,
                                            Rounding(),
                                            "%1$.2f %3%"));
        data_ = sitData;
    }

    // Turkish lira
    /* The ISO three-letter code was TRL; the numeric code was 792.
       It was divided in 100 kurus.
    */
    TRLCurrency::TRLCurrency() {
        static ext::shared_ptr<Data> trlData(
                                      new Data("Turkish lira", "TRL", 792,
                                               "TL", "", 100,
                                               Rounding(),
                                               "%1$.0f %3%"));
        data_ = trlData;
    }

    // New Turkish lira
    /* The ISO three-letter code is TRY; the numeric code is 949.
       It is divided in 100 new kurus.
    */
    TRYCurrency::TRYCurrency() {
        static ext::shared_ptr<Data> tryData(
                                  new Data("New Turkish lira", "TRY", 949,
                                           "YTL", "", 100,
                                           Rounding(),
                                           "%1$.2f %3%"));
        data_ = tryData;
    }


    // currencies obsoleted by Euro

    // Austrian shilling
    /* The ISO three-letter code was ATS; the numeric code was 40.
       It was divided in 100 groschen.
    */
    ATSCurrency::ATSCurrency() {
        static ext::shared_ptr<Data> atsData(
                                  new Data("Austrian shilling", "ATS", 40,
                                           "", "", 100,
                                           Rounding(),
                                           "%2% %1$.2f",
                                           EURCurrency()));
        data_ = atsData;
    }

    // Belgian franc
    /* The ISO three-letter code was BEF; the numeric code was 56.
       It had no subdivisions.
    */
    BEFCurrency::BEFCurrency() {
        static ext::shared_ptr<Data> befData(
                                      new Data("Belgian franc", "BEF", 56,
                                               "", "", 1,
                                               Rounding(),
                                               "%2% %1$.0f",
                                               EURCurrency()));
        data_ = befData;
    }

    // Deutsche mark
    /* The ISO three-letter code was DEM; the numeric code was 276.
       It was divided into 100 pfennig.
    */
    DEMCurrency::DEMCurrency() {
        static ext::shared_ptr<Data> demData(
                                     new Data("Deutsche mark", "DEM", 276,
                                              "DM", "", 100,
                                              Rounding(),
                                              "%1$.2f %3%",
                                              EURCurrency()));
        data_ = demData;
    }

    // Spanish peseta
    /* The ISO three-letter code was ESP; the numeric code was 724.
       It was divided in 100 centimos.
    */
    ESPCurrency::ESPCurrency() {
        static ext::shared_ptr<Data> espData(
                                    new Data("Spanish peseta", "ESP", 724,
                                             "Pta", "", 100,
                                             Rounding(),
                                             "%1$.0f %3%",
                                             EURCurrency()));
        data_ = espData;
    }

    // Finnish markka
    /* The ISO three-letter code was FIM; the numeric code was 246.
       It was divided in 100 penniä.
    */
    FIMCurrency::FIMCurrency() {
        static ext::shared_ptr<Data> fimData(
                                    new Data("Finnish markka", "FIM", 246,
                                             "mk", "", 100,
                                             Rounding(),
                                             "%1$.2f %3%",
                                             EURCurrency()));
        data_ = fimData;
    }

    // French franc
    /* The ISO three-letter code was FRF; the numeric code was 250.
       It was divided in 100 centimes.
    */
    FRFCurrency::FRFCurrency() {
        static ext::shared_ptr<Data> frfData(
                                      new Data("French franc", "FRF", 250,
                                               "", "", 100,
                                               Rounding(),
                                               "%1$.2f %2%",
                                               EURCurrency()));
        data_ = frfData;
    }

    // Greek drachma
    /* The ISO three-letter code was GRD; the numeric code was 300.
       It was divided in 100 lepta.
    */
    GRDCurrency::GRDCurrency() {
        static ext::shared_ptr<Data> grdData(
                                     new Data("Greek drachma", "GRD", 300,
                                              "", "", 100,
                                              Rounding(),
                                              "%1$.2f %2%",
                                              EURCurrency()));
        data_ = grdData;
    }

    // Irish punt
    /* The ISO three-letter code was IEP; the numeric code was 372.
       It was divided in 100 pence.
    */
    IEPCurrency::IEPCurrency() {
        static ext::shared_ptr<Data> iepData(
                                        new Data("Irish punt", "IEP", 372,
                                                 "", "", 100,
                                                 Rounding(),
                                                 "%2% %1$.2f",
                                                 EURCurrency()));
        data_ = iepData;
    }

    // Italian lira
    /* The ISO three-letter code was ITL; the numeric code was 380.
       It had no subdivisions.
    */
    ITLCurrency::ITLCurrency() {
        static ext::shared_ptr<Data> itlData(
                                      new Data("Italian lira", "ITL", 380,
                                               "L", "", 1,
                                               Rounding(),
                                               "%3% %1$.0f",
                                               EURCurrency()));
        data_ = itlData;
    }

    // Luxembourg franc
    /* The ISO three-letter code was LUF; the numeric code was 442.
       It was divided in 100 centimes.
    */
    LUFCurrency::LUFCurrency() {
        static ext::shared_ptr<Data> lufData(
                                  new Data("Luxembourg franc", "LUF", 442,
                                           "F", "", 100,
                                           Rounding(),
                                           "%1$.0f %3%",
                                           EURCurrency()));
        data_ = lufData;
    }

    // Maltese lira
    /* The ISO three-letter code is MTL; the numeric code is 470.
       It is divided in 100 cents.
    */
    MTLCurrency::MTLCurrency() {
        static ext::shared_ptr<Data> mtlData(
                                      new Data("Maltese lira", "MTL", 470,
                                               "Lm", "", 100,
                                               Rounding(),
                                               "%3% %1$.2f"));
        data_ = mtlData;
    }

    // Dutch guilder
    /* The ISO three-letter code was NLG; the numeric code was 528.
       It was divided in 100 cents.
    */
    NLGCurrency::NLGCurrency() {
        static ext::shared_ptr<Data> nlgData(
                                     new Data("Dutch guilder", "NLG", 528,
                                              "f", "", 100,
                                              Rounding(),
                                              "%3% %1$.2f",
                                              EURCurrency()));
        data_ = nlgData;
    }

    // Portuguese escudo
    /* The ISO three-letter code was PTE; the numeric code was 620.
       It was divided in 100 centavos.
    */
    PTECurrency::PTECurrency() {
        static ext::shared_ptr<Data> pteData(
                                 new Data("Portuguese escudo", "PTE", 620,
                                          "Esc", "", 100,
                                          Rounding(),
                                          "%1$.0f %3%",
                                          EURCurrency()));
        data_ = pteData;
    }

    // Slovak koruna
    /* The ISO three-letter code is SKK; the numeric code is 703.
       It is divided in 100 halierov.
    */
    SKKCurrency::SKKCurrency() {
        static ext::shared_ptr<Data> skkData(
                                     new Data("Slovak koruna", "SKK", 703,
                                              "Sk", "", 100,
                                              Rounding(),
                                              "%1$.2f %3%"));
        data_ = skkData;
    }

    // Ukrainian hryvnia
    /* The ISO three-letter code is UAH; the numeric code is 980.
       It is divided in 100 kopiykas.
     */
    UAHCurrency::UAHCurrency() {
        static ext::shared_ptr<Data> uahData(
                                     new Data("Ukrainian hryvnia", "UAH", 980,
                                              "hrn", "", 100,
                                              Rounding(),
                                              "%1$.2f %3%"));
        data_ = uahData;
    }

    // Serbian dinar
    RSDCurrency::RSDCurrency() {
        static ext::shared_ptr<Data> rsdData(
            new Data("Serbian dinar", "RSD", 941, "RSD", "", 100, Rounding(), "1$.2f %3%"));
        data_ = rsdData;
    }

    // Croatian kuna
    HRKCurrency::HRKCurrency() {
        static ext::shared_ptr<Data> hrkData(
            new Data("Croatian kuna", "HRK", 191, "HRK", "", 100, Rounding(), "1$.2f %3%"));
        data_ = hrkData;
    }

    // Bulgarian lev
    BGNCurrency::BGNCurrency() {
        static ext::shared_ptr<Data> bgnData(
            new Data("Bulgarian lev", "BGN", 975, "BGN", "", 100, Rounding(), "1$.2F %3%"));
        data_ = bgnData;
    }

    // Georgian lari
    GELCurrency::GELCurrency() {
        static ext::shared_ptr<Data> gelData(
            new Data("Georgian lari", "GEL", 981, "GEL", "", 100, Rounding(), "1$.2F %3%"));
        data_ = gelData;
    }
}


