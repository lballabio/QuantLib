import shaft

tol = 1e-9

data =    [  3,   4,   5,   2,   3,   4,   5,   6,   4,   7]
weights = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]

s = shaft.Statistics()
s.addWeightedSequence(data, weights)

print 'samples', s.samples(),
if (s.samples()==10):
  print 'OK'
else:
  print 'wrong'

print 'sample Weight', s.sampleWeight(),
if (s.sampleWeight()==10):
  print 'OK'
else:
  print 'wrong'

print 'min', s.min(),
if (s.min()==2):
  print 'OK'
else:
  print 'wrong'

print 'max', s.max(),
if (s.max()==7):
  print 'OK'
else:
  print 'wrong'

print 'mean', s.mean(),
if (abs(s.mean()-4.3)<tol):
  print 'OK'
else:
  print 'wrong'

print 'variance', s.variance(),
if (abs(s.variance()-2.23333333333)<tol):
  print 'OK'
else:
  print 'wrong'

print 'stdDev', s.standardDeviation(),
if (abs(s.standardDeviation()-1.4944341181)<tol):
  print 'OK'
else:
  print 'wrong'

print 'skew', s.skewness(),
if (abs(s.skewness()-0.359543071407)<tol):
  print 'OK'
else:
  print 'wrong'

print 'kurt', s.kurtosis(), 
if (abs(s.kurtosis()+0.151799637209)<tol):
  print 'OK'
else:
  print 'wrong'


