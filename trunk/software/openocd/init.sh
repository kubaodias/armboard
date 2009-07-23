#!/bin/sh
openocd -f interface/turtelizer.cfg -f target/at91rm9200.cfg -f openocd.cfg -c "jtag_khz 1000"
