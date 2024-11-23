# About This Fork

This repository is a fork of the original BBC BASIC for the Zilog EZ80 by Dean Belfield, specifically targeting the Agon Light family of retrocomputers. Originally written for the proprietary, Windows-only, closed-source Zilog ZDS-II assembler, this sourcecode has been made compatiblilty with the open-source, multi-platform `agon-ez80asm` assembler by [Jeroen Venema (evenomator)](https://github.com/envenomator/agon-ez80asm). This allows assembly on native hardware as well as various modern platforms, including macOS, Windows, and x86-64 and ARM64 Linux.

### Assembling and Running
The file `basic.bin` in the project root directory contains all the includes, in the proper order, to assemble the executable. On harware, navigate to the directory containing the source code and enter, e.g.: `ez80asm basic.asm /bin/EZBASIC.BIN`, or whatever taget filename you prefer, so long as the destination directory is in `/bin` on the SD card so that MOS recognizes it as a vaild command. From there usage aims to be exactly as the ZDS version.

### Modifications and features
- Translation of source code to the `agon-ez80asm` syntax, aiming for full compatibility with the ZDS II source code of the original.
- Maintaining use of ADL (Address Data Long) mode, allowing the BASIC programs and data to extend past the non-ADL limitation of 64K.
- Documentation of changes inline with the original code. (ongoing)
- Additional comments documenting unmodified code where it adds clarity and utility. (ongoing)

This fork is maintained by Brandon R. Gates, who can be found on Discord as BeeGee747. The source code and additional details about this fork are [available on GitHub](https://github.com/bgates747/agon-bbc-basic-adl-ez80asm).

## Acknowledgments:
Many thanks to R.T. Russell for his original work on BBC BASIC, Dean Belfield for the port to Agon and Jeroen Venema for providing open-source resources and tools that have enabled this adaptation.

Brandon R. Gates 10 November 2024

Original text of this file follows
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# agon-bbc-basic-adl

A port of BBC Basic for Z80 to the Agon, converted to run in ADL mode

### What is the Agon

Agon is a modern, fully open-source, 8-bit microcomputer and microcontroller in one small, low-cost board. As a computer, it is a standalone device that requires no host PC: it puts out its own video (VGA), audio (2 identical mono channels), accepts a PS/2 keyboard and has its own mass-storage in the form of a µSD card.

https://www.thebyteattic.com/p/agon.html

### What is BBC Basic for Z80?

The original version of BBC Basic was written by Sophie Wilson at Acorn in 1981 for the BBC Micro range of computers, and was designed to support the UK Computer Literacy Project. R.T.Russell was involved in the specification of BBC Basic, and wrote his own Z80 version that was subsequently ported to a number of Z80 based machines. [I highly recommend reading his account of this on his website for more details](http://www.bbcbasic.co.uk/bbcbasic/history.html).

As an aside, R.T.Russell still supports BBC Basic, and has ported it for a number of modern platforms, including Android, Windows, and SDL, which are [available from his website here](https://www.bbcbasic.co.uk/index.html).

### What is ADL mode?

ADL stands for Address Data Long. When the eZ80 is switched into this mode, the eZ80 runs natively in 24-bit mode.

### Why am I doing this?

I have already ported BBC BASIC for Z80 to the Agon (agon-bbc-basic), but it is essentially sandboxed in a 64K segment in the Agon address space. This version of BBC BASIC has been modified to run in ADL mode, and thus BASIC programs and data can take advantage of the whole of available user RAM, currently 429K.

### Assembling and Running

This project is designed to be assembled and linked using the Zilog ZDS II toolkit - see the [readme](https://github.com/breakintoprogram/agon-mos/blob/main/README.md#build) in MOS for more details.

NB:
- The project is configured to download this to RAM at &40000 via the ZDS cable

### Documentation

The AGON documentation can now be found on the [Agon Light Documentation Wiki](https://github.com/breakintoprogram/agon-docs/wiki)

### License

This code is distributable under the terms of a zlib license. Read the file [COPYING](COPYING) for more information.

Many thanks to R.T. Russell for open sourcing the source code, and David Given for facilitating this.

http://cowlark.com/2019-06-14-bbcbasic-opensource/index.html

The BASIC interpreter, as originally written by R.T. Russell and [downloaded from David Given's GitHub page](https://github.com/davidgiven/cpmish/tree/master/third_party/bbcbasic), has been modified either for compatibility reasons when assembling using the ZDS IDE, or for development reasons for this release.

The original files are: [eval.z80](eval.z80), [exec.z80](exec.z80), [fpp.z80](fpp.z80), [patch.z80](patch.z80), [main.z80](main.z80), [ram.z80](ram.z80) and [sorry.z80](sorry.z80), [bbcbasic.txt](bbcbasic.txt), the license ([COPYING](COPYING)) and all the files in the examples folder.

Any additions or modifications I've made to port this to the Agon have been released under the same licensing terms as the original code, along with any tools, examples or utilities contained within this project. Code that has been copied or inspired by other sources is clearly marked, with the appropriate accreditations.

Dean Belfield

Twitter: [@breakintoprogram](https://twitter.com/BreakIntoProg)
Blog: http://www.breakintoprogram.co.uk
