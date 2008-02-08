ph7xxx create adc1 -slot 6
ph7xxx create adc2 -slot 7


stack create events 
stack config events -type event -modules [list adc1 adc2] -delay 10 -lamtimeout 8


set parameters(adc1) [list ph1.00 ph1.01 ph1.02 ph1.03  \
			   ph1.04 ph1.05 ph1.06 ph1.07  \
			   ph1.08 ph1.09 ph1.10 ph1.11  \
			   ph1.12 ph1.13 ph1.14 ph1.15]

set parameters(adc2) [list ph2.00 ph2.01 ph2.02 ph2.03  \
			   ph2.04 ph2.05 ph2.06 ph2.07  \
			   ph2.08 ph2.09 ph2.10 ph2.11  \
			   ph2.12 ph2.13 ph2.14 ph2.15]