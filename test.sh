#!/bin/sh

# "-b mode		set stereo mode (0=mono, default 1=headphone, 2=stereo)\n"
# "-8		    generate 8-bit output\n"
# "-p num		subsong to play (default 0)\n"
# "-f freq		suggest playback rate in samples/sec (default 44100)\n"
# "-o file		write audio output to file\n"
# "-i		    print info about the module (text, subsong, etc.)\n"
# "-w num		set low-pass filter frequency (0=none, 3=lowest, default 0)\n"
# "-l num		set loop mode (0=no repeat, default 1=infinite)\n"
# "-v           disable oversampling (=linear interpolation)\n"
# "-D           force hack for Danger Freak title tune\n"
# "-G           force old hack for GemX title tune (still incomplete)\n"
# "-x           export to XRNS XML\n"
# "-~           debug mode (commands pp and pm)\n"

./tfmx -b 1 -i -p 0 "/Users/kosta/Music/Amiga Music/TFMX/Turrican2/Turrican2-LVL1.TFX"