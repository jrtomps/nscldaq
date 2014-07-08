#===================================================================
# class AXLM72
#===================================================================

package provide xlm72 1.0

package require Itcl
package require vmusb 


itcl::class AXLM72 {
	protected variable device
	private variable self
	private variable slot
	private variable base
	private variable vme
	private variable fpga
	private variable dsp
	private variable srama
	private variable sramb
	
	constructor {de sl} {
		set device $de
		set self [string trimleft $this :]
		set slot $sl
		set base [expr $sl<<27]
		set vme [expr $base+0x800000]
		set dsp [expr $base+0x600000]
		set fpga [expr $base+0x400000]
		set sramb [expr $base+0x200000]
		set srama [expr $base+0x000000]
	}
	
	destructor {}
	
# interactive functions
	public method GetVariable {v} {set $v}
	public method Read {dev address} {return [$device Read32 [expr [set $dev]+$address]]}
	public method Write {dev address data} { $device Write32 [expr [set $dev]+$address] $data}
	public method ReadSBLT {dev address words} {return [$device ReadSBLT32 [expr [set $dev]+$address] $words 0]}
	public method ReadFBLT {dev address blocks} {return [$device ReadFBLT32 [expr [set $dev]+$address] $blocks 0]}
	public method ReadNBLT {ndev nadd mask dev address} {return [$device ReadNBLT32 [expr [set $ndev]+$nadd] [expr [set $dev]+$address] $mask 0]}
	public method AccessBus {code} { Write vme 0 $code; Write vme [expr 0xc] 1}
	public method ReleaseBus {} {Write vme 0 0; Write vme [expr 0xc] 0}
	public method BootFPGA {}
	public method SetFPGABoot {source} {Write vme 8 $source}
	public method Configure {filename}
# stack functions
	public method sRead {stack dev add} {$device sRead32 $stack [expr [set $dev]+$add]}
	public method sWrite {stack dev add data} {$device sWrite32 $stack [expr [set $dev]+$add] $data}
	public method sReadSBLT {stack dev add words} {$device sReadSBLT32 $stack [expr [set $dev]+$add] $words 0}
	public method sReadFBLT {stack dev add blocks} {$device sReadFBLT32 $stack [expr [set $dev]+$add] $blocks 0}
	public method sReadNBLT {stack ndev nadd mask dev add} {$device sReadNBLT32 $stack [expr [set $ndev]+$nadd] [expr [set $dev]+$add] $mask 0}
	public method sAccessBus {stack code} {sWrite $stack vme 0 $code}; # sWrite $stack vme 0xc 1
	public method sReleaseBus {stack} {sWrite $stack vme 0 0}; # sWrite $stack vme 0xc 0
}

itcl::body AXLM72::BootFPGA {} {
# Keep the DSP in reset mode (bit2=1)
	Write vme 4 3
	Write vme 4 2
# Leave some time to boot FPGA
	after 100
}

itcl::body AXLM72::Configure {filename} {
	set file [open $filename r]
	fconfigure $file -translation binary
	set config [read $file]
	close $file
	set size [file size $filename]
	binary scan $config c$size bytes
# Detect first 0xff in file
	set index 0
	while {[lindex $bytes $index] != -1} {incr index}

# Determine number of 256 bytes blocks
	set nblocks [expr int(($size-$index+1)/64)+1]

# Stack is made of 16 bits words
	set stacksize [expr $nblocks*128+8]
# Write stack header
#	lappend stack [expr ($stacksize-1)]
#  lappend stack 0
	lappend stack [expr ((0x4000<<16) | 0x80b) ]; # multi-block BLT write
	lappend stack $nblocks; 
	lappend stack [expr $srama ];
# Translate and write configuration in SRAMA
	for {set i $index} {$i < $size} {incr i} {
		set byte [lindex $bytes $i]
		set blow [expr $byte&0xf]
		set bhigh [expr ($byte&0xf0)>>4]
		set llow [expr (($blow&0x7)<<2) + (($blow&0x8)<<7)]
		set lhigh [expr (($bhigh&0x7)<<2) + (($bhigh&0x8)<<7)]
		lappend stack [expr (($lhigh<<16) | $llow) ]
	}
# Execute long stack
	AccessBus [expr 0x1]
  flush stdout
#	XXUSBExecuteLongStack $device $stack
  $device ExecuteLongStack $stack
	after 100
	ReleaseBus
# Now boot the FPGA from SRAMA
	SetFPGABoot [expr 0x10000]
	BootFPGA
}

