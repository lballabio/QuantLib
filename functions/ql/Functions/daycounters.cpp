
/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano

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

#include <ql/Functions/daycounters.hpp>
#include <ql/DayCounters/all.hpp>
#include <ql/basicdataformatters.hpp>

namespace QuantLib {

    DayCounter dayCounterFromString(std::string inputString) {

        std::string s = StringFormatter::toLowercase(inputString);

        if (        s == "1/1"          // ISDA
                 || s == "one")
            return OneDayCounter();

        else if (   s == "actual/365"    // ISDA
                 || s == "act/365"       // ISDA
                 || s == "a/365"         // ISDA
                 || s == "actual365" 
                 || s == "act365" 
                 || s == "a365" 

                 || s == "actual/actual" // ISDA
                 || s == "act/actual"
                 || s == "a/actual"
                 || s == "actualactual"
                 || s == "actual/act"
                 || s == "act/act"       // ISDA
                 || s == "a/act"
                 || s == "actualact"
                 || s == "actact"

                 || s == "actual/actual (isda)"
                 || s == "act/actual (isda)"
                 || s == "a/actual (isda)"
                 || s == "actualactual (isda)"
                 || s == "actual/act (isda)"
                 || s == "act/act (isda)"
                 || s == "a/act (isda)"
                 || s == "actualact (isda)"
                 || s == "actact (isda)"

                 || s == "actual/actual (historical)"
                 || s == "act/actual (historical)"
                 || s == "a/actual (historical)"
                 || s == "actualactual (historical)"
                 || s == "actual/act (historical)"
                 || s == "act/act (historical)"
                 || s == "a/act (historical)"
                 || s == "actualact (historical)"
                 || s == "actact (historical)"

                 || s == "actual/actual (h)"
                 || s == "act/actual (h)"
                 || s == "a/actual (h)"
                 || s == "actualactual (h)"
                 || s == "actual/act (h)"
                 || s == "act/act (h)"
                 || s == "a/act (h)"
                 || s == "actualact (h)"
                 || s == "actact (h)"

                 || s == "actual/actualh"
                 || s == "act/actualh"
                 || s == "a/actualh"
                 || s == "actualactualh"
                 || s == "actual/acth"
                 || s == "act/acth"
                 || s == "a/acth"
                 || s == "actualacth"
                 || s == "actacth")
            return ActualActual(ActualActual::ISDA);

        else if (   s == "actual/365 (fixed)" // ISDA
                 || s == "act/365 (fixed)"    // ISDA
                 || s == "a/365 (fixed)"      // ISDA
                 || s == "actual365 (fixed)" 
                 || s == "act365 (fixed)" 
                 || s == "a365 (fixed)"
                 || s == "actual/365 (f)"
                 || s == "act/365 (f)"  
                 || s == "a/365 (f)"   
                 || s == "actual365 (f)" 
                 || s == "act365 (f)" 
                 || s == "a365 (f)"
                 || s == "actual/365f"
                 || s == "act/365f"  
                 || s == "a/365f"             // ISDA
                 || s == "actual365f" 
                 || s == "act365f" 
                 || s == "a365f")
            return Actual365Fixed();

        else if (   s == "actual/360"    // ISDA
                 || s == "act/360"       // ISDA
                 || s == "a/360"         // ISDA
                 || s == "actual360"
                 || s == "act360"
                 || s == "a360")
            return Actual360();

        else if (   s == "30/360"     // ISDA
                 || s == "30360"
                 || s == "360/360"    // ISDA
                 || s == "360360"
                 || s == "bond basis" // ISDA
                 || s == "bondbasis"
                 || s == "30/360us"
                 || s == "30360us"
                 || s == "360/360us"
                 || s == "360360us")
            return Thirty360(Thirty360::BondBasis);
        
        else if (   s == "30e/360"     // ISDA
                 || s == "30e360"
                 || s == "360e/360"
                 || s == "360e360"
                 || s == "eurobond basis" // ISDA
                 || s == "eurobondbasis"
                 || s == "30/360eu"
                 || s == "30360eu"
                 || s == "360/360eu"
                 || s == "360360eu")
            return Thirty360(Thirty360::BondBasis);
        
        /************* Non ISDA daycounters ****************/

        else if (   s == "simple"
                 || s == "s")
            return SimpleDayCounter();

        else if (   s == "30i/360"
                 || s == "30i360"
                 || s == "360i/360"
                 || s == "360i360"
                 || s == "30it/360"
                 || s == "30it360"
                 || s == "360it/360"
                 || s == "360it360"
                 || s == "30/360i"
                 || s == "30360i"
                 || s == "360/360i"
                 || s == "360360i"
                 || s == "30/360it"
                 || s == "30360it"
                 || s == "360/360it"
                 || s == "360360it"
                 )
            return Thirty360(Thirty360::Italian);
        
        else if (   s == "actual/actual (isma)"
                 || s == "act/actual (isma)"
                 || s == "a/actual (isma)"
                 || s == "actualactual (isma)"
                 || s == "actual/act (isma)"
                 || s == "act/act (isma)"
                 || s == "a/act (isma)"
                 || s == "actualact (isma)"
                 || s == "actact (isma)"

                 || s == "actual/actual (bond)"
                 || s == "act/actual (bond)"
                 || s == "a/actual (bond)"
                 || s == "actualactual (bond)"
                 || s == "actual/act (bond)"
                 || s == "act/act (bond)"
                 || s == "a/act (bond)"
                 || s == "actualact (bond)"
                 || s == "actact (bond)"

                 || s == "actual/actual (b)"
                 || s == "act/actual (b)"
                 || s == "a/actual (b)"
                 || s == "actualactual (b)"
                 || s == "actual/act (b)"
                 || s == "act/act (b)"
                 || s == "a/act (b)"
                 || s == "actualact (b)"
                 || s == "actact (b)"

                 || s == "actual/actualb"
                 || s == "act/actualb"
                 || s == "a/actualb"
                 || s == "actualactualb"
                 || s == "actual/actb"
                 || s == "act/actb"
                 || s == "a/actb"
                 || s == "actualactb"
                 || s == "actactb")
            return ActualActual(ActualActual::ISMA);

        else if (   s == "actual/actual (afb)"
                 || s == "act/actual (afb)"
                 || s == "a/actual (afb)"
                 || s == "actualactual (afb)"
                 || s == "actual/act (afb)"
                 || s == "act/act (afb)"
                 || s == "a/act (afb)"
                 || s == "actualact (afb)"
                 || s == "actact (afb)"

                 || s == "actual/actual (euro)"
                 || s == "act/actual (euro)"
                 || s == "a/actual (euro)"
                 || s == "actualactual (euro)"
                 || s == "actual/act (euro)"
                 || s == "act/act (euro)"
                 || s == "a/act (euro)"
                 || s == "actualact (euro)"
                 || s == "actact (euro)"

                 || s == "actual/actual (e)"
                 || s == "act/actual (e)"
                 || s == "a/actual (e)"
                 || s == "actualactual (e)"
                 || s == "actual/act (e)"
                 || s == "act/act (e)"
                 || s == "a/act (e)"
                 || s == "actualact (e)"
                 || s == "actact (e)"

                 || s == "actual/actuale"
                 || s == "act/actuale"
                 || s == "a/actuale"
                 || s == "actualactuale"
                 || s == "actual/acte"
                 || s == "act/acte"
                 || s == "a/acte"
                 || s == "actualacte"
                 || s == "actacte")
            return ActualActual(ActualActual::AFB);

        else
            QL_FAIL("Unknown day counter: " + inputString);

    }

    BigInteger accrualDays(DayCounter dc, Date d1, Date d2) {
        return dc.dayCount(d1, d2);
    }


    Time accrualFactor(DayCounter dc, Date d1, Date d2, Date d3, Date d4) {
        return dc.yearFraction(d1, d2, d3, d4);
    }

}
