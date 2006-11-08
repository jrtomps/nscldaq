source configFile.tcl
source setup.tcl
source fom.tcl
source project.tcl

#  Test for m2 spectra:

spectrum test m2 {tdc.E.00 tdc.E.01 tdc.E.02 tdc.E.03 tdc.E.04 tdc.E.05} \
    {{0 1023 512} {0 1023 512}}

sbind test


