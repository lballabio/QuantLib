import math, time
#import QuantLib
import shaft

typRange = ['Call']
typRange = ['Call', 'Put', 'Straddle']
underRange = [100]
underRange = [80, 95, 99.90, 100, 100.10, 105, 120]
strikeRange = [100]
strikeRange = [80, 95, 99.90, 100, 100.10, 105, 120]
qRateRange = [0.05]
qRateRange = [0.01, 0.05, 0.10]
rRateRange = [0.05]
rRateRange = [0.01, 0.05, 0.10]
resTimeRange = [1.0]
resTimeRange = [0.001, 0.1, 0.5, 1.0, 3.0]
volRange = [0.2]
volRange = [0.01, 0.2, 0.3, 0.7, 0.9]
dVolRange = [1.0]
dVolRange = [0.5, 0.7, 0.999, 1.0, 1.001, 1.3, 1.5]

maxEval = 100
tol = 1e-6

dummyiterations = 1

startTime = time.time()
for typ in typRange:
  print typ
  for under in underRange:
    for strike in strikeRange:
      for qRate in qRateRange:
        for rRate in rRateRange:
          for resTime in resTimeRange:
            for vol in volRange:
              #bsm = QuantLib.BSMEuropeanOption(typ, under, strike, qRate, rRate, resTime, vol)
              bsm = shaft.BSMEuropeanOption(typ, under, strike, qRate, rRate, resTime, vol)
              bsmValue = bsm.value()
              for dVol in dVolRange:
                vol2 = vol*dVol
                #bsm2 = QuantLib.BSMEuropeanOption(typ, under, strike, qRate, rRate, resTime, vol2)
                bsm2 = shaft.BSMEuropeanOption(typ, under, strike, qRate, rRate, resTime, vol2)
                try:
                  for i in range(dummyiterations):
                    implVol = bsm2.impliedVolatility(bsmValue, tol, maxEval)
                except Exception, e:
                  print 'type=%s; under=%5.2f; strike=%5.2f; qRate=%4.2f; rRate=%4.2f; resTime=%5.3f;' % (typ, under, strike, qRate, rRate, resTime)
                  print 'at %18.16f vol the option value is %20.12e' % (vol, bsmValue)
                  print 'at %18.16f vol the option value is %20.12e' % (vol2, bsm2.value())
                  print
                  print 'while trying to calculate implied vol from value %20.12e' % (bsmValue)
                  print 'the following exception has been raised:'
                  raise e
                err = abs(implVol-vol)
                if err > tol :
                  #bsm3 = QuantLib.BSMEuropeanOption(typ, under, strike, qRate, rRate, resTime, implVol)
                  bsm3 = shaft.BSMEuropeanOption(typ, under, strike, qRate, rRate, resTime, implVol)
                  bsm3Value = bsm3.value()
                  if (abs(bsm3Value-bsmValue)/under>1e-3):
                    bsm2Value = bsm2.value()
                    print 'type=%s; under=%5.2f; strike=%5.2f; qRate=%4.2f; rRate=%4.2f; resTime=%5.3f;' % (typ, under, strike, qRate, rRate, resTime)
                    print 'at %18.16f vol the option value is %20.12e' % (vol, bsmValue)
                    print 'at %18.16f vol the option value is %20.12e' % (vol2, bsm2Value)
                    print
                    print 'for option value %20.12e implied vol is: %20.16f' % (bsmValue, implVol)
                    print ' the error is %10.2e (tolerance is %10.2e)' % (err, tol)
                    print 'at %18.16f vol the option value is %20.12e' % (implVol, bsm3Value)
                    print ' that is', bsm3Value - bsmValue, 'above target value'
                    raise RuntimeError, "out of tolerance"
print 'time elapsed', time.time() - startTime
