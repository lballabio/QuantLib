
=begin
 Copyright (C) 2000-2001 QuantLib Group
 
 This file is part of QuantLib.
 QuantLib is a C++ open source library for financial quantitative
 analysts and developers --- http://quantlib.sourceforge.net/

 QuantLib is free software and you are allowed to use, copy, modify, merge,
 publish, distribute, and/or sell copies of it under the conditions stated 
 in the QuantLib License.

 This program is distributed in the hope that it will be useful, but 
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.

 You should have received a copy of the license along with this file;
 if not, contact ferdinando@ametrano.net

 QuantLib license is also available at:
 http://quantlib.sourceforge.net/LICENSE.TXT

 $Source$
 $Log$
 Revision 1.1  2001/03/23 12:37:12  lballabio
 Translated from Python

=end


require 'QuantLib'
require 'TestUnit'

TestUnit.test('dates') {
    minDate = QuantLib::Date.minDate.serialNumber
    maxDate = QuantLib::Date.maxDate.serialNumber

    dyold  = QuantLib.DateFromSerialNumber(minDate-1).dayOfYear
    dold   = QuantLib.DateFromSerialNumber(minDate-1).dayOfMonth
    mold   = QuantLib.DateFromSerialNumber(minDate-1).monthNumber
    yold   = QuantLib.DateFromSerialNumber(minDate-1).year
    wdnold = QuantLib.DateFromSerialNumber(minDate-1).weekdayNumber

    TestUnit.printDetails(
        "test starting at #{QuantLib::Date.minDate} (#{minDate})...")

    minDate.upto(maxDate) { |i|
        begin
            t = QuantLib.DateFromSerialNumber(i)
            # check serial number consistency
            if t.serialNumber != i
                TestUnit.printDetails("Major problem")
            end

            dy  = t.dayOfYear
            d   = t.dayOfMonth
            m   = t.monthNumber
            y   = t.year
            mm  = t.month
            wd  = t.weekday
            wdn = t.weekdayNumber

            # check if skipping any date
            if dy==dyold+1 \
            or (dy==1 and dyold==365 and not QuantLib::Date.isLeap(yold)) \
            or (dy==1 and dyold==366 and QuantLib::Date.isLeap(yold))
                dyold = dy
            else
                TestUnit.printDetails(
                    "dyold: #{dyold}, dy: #{dy}")
                raise "wrong day of year increment"
            end

            # check if skipping any date
            if (d==dold+1 and m==mold   and y==yold  ) \
            or (d==1      and m==mold+1 and y==yold  ) \
            or (d==1      and m==1      and y==yold+1)
               dold = d
               mold = m
               yold = y
            else
               TestUnit.printDetails(
                   "dold: #{dold}, d: #{d},",
                   "mold: #{mold}, m: #{m},",
                   "yold: #{yold}, y: #{y},")
               raise "wrong day, month, year increment"
            end

            # check month definition
            if m<1 or m>12
                TestUnit.printDetails("m: #{m}")
                raise "undefined month"
            end
            # check day definition
            if d<1
                TestUnit.printDetails("d: #{d}")
                raise "day < 1"
            end
            if (m==1  and d>31) \
            or (m==2  and d>29) \
            or (m==2  and d==29 and not QuantLib::Date.isLeap(y)) \
            or (m==3  and d>31) \
            or (m==4  and d>30) \
            or (m==5  and d>31) \
            or (m==6  and d>30) \
            or (m==7  and d>31) \
            or (m==8  and d>31) \
            or (m==9  and d>30) \
            or (m==10  and d>31) \
            or (m==11  and d>30) \
            or (m==12  and d>31)
                TestUnit.printDetails("m: #{m}, d: #{d}")
                raise "day too big for month"
            end

            # check weekdayNumber definition
            if wdn==wdnold+1 or (wdn==1 and wdnold==7)
                wdnold=wdn
            else
                TestUnit.printDetails("wdn: #{wdn}, wdnold: #{wdnold}")
                raise "weekday number error"
            end

            # create the same date with a different constructor
            s = QuantLib::Date.new(d,m,y)
            # check serial number consistency
            if s.serialNumber != i
                TestUnit.printDetails("date: #{t}")
                TestUnit.printDetails("serial number: #{i}")
                TestUnit.printDetails("new date: #{s}")
                TestUnit.printDetails("serial number: #{s.serialNumber}")
                raise "new date serial number different from old date"
            end

            # create the same date with yet another constructor
            s = QuantLib::Date.new(d,mm,y)
            # check serial number consistency
            if s.serialNumber != i
                TestUnit.printDetails("date: #{t}")
                TestUnit.printDetails("serial number: #{i}")
                TestUnit.printDetails("new date: #{s}")
                TestUnit.printDetails("serial number: #{s.serialNumber}")
                raise "new date serial number different from old date"
            end

        rescue Exception => e
            t = QuantLib.DateFromSerialNumber(i)
            TestUnit.printDetails("exception thrown at #{t}")
            TestUnit.printDetails("serial number: #{i}")
            TestUnit.printDetails("day of year: #{t.dayOfYear}")
            TestUnit.printDetails("day of month: #{t.dayOfMonth}")
            TestUnit.printDetails("month: #{t.month}")
            TestUnit.printDetails("year: #{t.year}")
            raise e
        end
    }
    TestUnit.printDetails(
        "test ended at #{QuantLib::Date.maxDate} (#{maxDate})")
}

