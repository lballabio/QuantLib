require 'mkmf'
$CFLAGS   += ' -DHAVE_CONFIG_H'
$CPPFLAGS += ' -I/usr/local/include/QuantLib'
$libs     += ' -lQuantLib'
create_makefile("QuantLib")

