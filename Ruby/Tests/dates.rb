
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
 The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT

 The members of the QuantLib Group are listed in the Authors.txt file, also
 available at http://quantlib.sourceforge.net/Authors.txt
=end

=begin
 $Id$
 $Source$
 $Log$
 Revision 1.2  2001/04/06 18:46:21  nando
 changed Authors, Contributors, Licence and copyright header

=end


require 'QuantLib'
require 'runit/testcase'
require 'runit/testsuite'
require 'runit/cui/testrunner'

class DateTest < RUNIT::TestCase
    def name
        "Testing dates..."
    end
    def test
        minDate = QuantLib::Date.minDate.serialNumber
        maxDate = QuantLib::Date.maxDate.serialNumber

        dyold  = QuantLib.DateFromSerialNumber(minDate-1).dayOfYear
        dold   = QuantLib.DateFromSerialNumber(minDate-1).dayOfMonth
        mold   = QuantLib.DateFromSerialNumber(minDate-1).monthNumber
        yold   = QuantLib.DateFromSerialNumber(minDate-1).year
        wdnold = QuantLib.DateFromSerialNumber(minDate-1).weekdayNumber

        minDate.upto(maxDate) { |i|
            t = QuantLib.DateFromSerialNumber(i)
            # check serial number consistency
            assert(t.serialNumber == i,
                "inconsistent serial number:\n" +
                "original:      #{i}\n" +
                "date:          #{t}\n" +
                "serial number: #{t.serialNumber}\n")

            dy  = t.dayOfYear
            d   = t.dayOfMonth
            m   = t.monthNumber
            y   = t.year
            mm  = t.month
            wd  = t.weekday
            wdn = t.weekdayNumber

            # check if skipping any date
            unless dy==dyold+1 \
                or (dy==1 and dyold==365 and not QuantLib::Date.isLeap(yold)) \
                or (dy==1 and dyold==366 and QuantLib::Date.isLeap(yold))
                raise "wrong day of year increment:\n" + \
                      "date: #{t}\n" + \
                      "day of year: #{dy}\n" + \
                      "previous:    #{dyold}\n"
            end
            dyold = dy

            # check if skipping any date
            unless (d==dold+1 and m==mold   and y==yold  ) \
                or (d==1      and m==mold+1 and y==yold  ) \
                or (d==1      and m==1      and y==yold+1)
                raise "wrong day, month, year increment\n" + \
                      "date: #{t}\n" + \
                      "day, month, year: #{d}, #{m}, #{y}\n" + \
                      "previous:         #{dold}, #{mold}, #yold}\n"
            end
            dold = d
            mold = m
            yold = y

            # check month definition
            unless m>=1 and m<=12
                raise "invalid month\n" + \
                      "date: #{t}\n" + \
                      "month: #{m}\n"
            end
            # check day definition
            unless d >= 1
               raise "invalid day of month\n" + \
                     "date: #{t}\n" + \
                     "day: #{d}\n"
            end
            unless (m==1  and d<=31) \
                or (m==2  and d<=28) \
                or (m==2  and d==29 and QuantLib::Date.isLeap(y)) \
                or (m==3  and d<=31) \
                or (m==4  and d<=30) \
                or (m==5  and d<=31) \
                or (m==6  and d<=30) \
                or (m==7  and d<=31) \
                or (m==8  and d<=31) \
                or (m==9  and d<=30) \
                or (m==10 and d<=31) \
                or (m==11 and d<=30) \
                or (m==12 and d<=31)
                raise "invalid day of month\n" + \
                      "date: #{t}\n" + \
                      "day: #{d}\n" + \
                      "month: #{mm}\n"
            end

            # check weekdayNumber definition
            unless wdn==wdnold+1 or (wdn==1 and wdnold==7)
                raise "wrong weekday number increment\n" + \
                      "date: #{t}\n" + \
                      "weekday number: #{wdn}\n" + \
                      "previous:       #{wdnold}\n"
            end
            wdnold=wdn

            # create the same date with a different constructor
            s = QuantLib::Date.new(d,m,y)
            # check serial number consistency
            unless s.serialNumber == i
                raise "inconsistent serial number\n" + \
                      "date: #{t}\n" + \
                      "serial number: #{i}\n" + \
                      "cloned date: #{s}\n" + \
                      "serial number: #{s.serialNumber}\n"
            end

            # create the same date with yet another constructor
            s = QuantLib::Date.new(d,mm,y)
            # check serial number consistency
            unless s.serialNumber == i
                raise "inconsistent serial number\n" + \
                      "date: #{t}\n" + \
                      "serial number: #{i}\n" + \
                      "cloned date: #{s}\n" + \
                      "serial number: #{s.serialNumber}\n"
            end

        }
    end
end

if $0 == __FILE__
    RUNIT::CUI::TestRunner.run(DateTest.suite)
end




