.k kerflags
. This file contains an execute script for building Kermit
.
. <kerflags> allows you to specify -DDEBUG -DTLOG to enable debugging logging
.
. This script expects the following logical defines:
.    ASSIGN lc: <directory containing LC1, LC2, and ALINK commands>
.    ASSIGN include: <directory include directories are rooted at>
.    ASSIGN lib: <directory containing LC.LIB, AMIGA.LIB, and C.O>
.
. The -v flag is used with LC2 to disable stack overflow checking in Kermit.
. The temporary compiler quad files are created in RAM:.
.
stack 20000
echo "compiling ckcmai:"
lc:lc1 <kerflags> -csw -iinclude:lattice/ -oram: ckcmai
lc:lc2 -v -ockcmai.o ram:ckcmai
echo "compiling ckucmd:"
lc:lc1 <kerflags> -csw -iinclude:lattice/ -oram: ckucmd
lc:lc2 -v -ockucmd.o ram:ckucmd
echo "compiling ckuusr:"
lc:lc1 <kerflags> -csw -iinclude:lattice/ -oram: ckuusr
lc:lc2 -v -ockuusr.o ram:ckuusr
echo "compiling ckuus2:"
lc:lc1 <kerflags> -csw -iinclude:lattice/ -oram: ckuus2
lc:lc2 -v -ockuus2.o ram:ckuus2
echo "compiling ckuus3:"
lc:lc1 <kerflags> -csw -iinclude:lattice/ -oram: ckuus3
lc:lc2 -v -ockuus3.o ram:ckuus3
echo "compiling ckcpro:"
lc:lc1 <kerflags> -csw -iinclude:lattice/ -oram: ckcpro
lc:lc2 -v -ockcpro.o ram:ckcpro
echo "compiling ckcfns:"
lc:lc1 <kerflags> -csw -iinclude:lattice/ -oram: ckcfns
lc:lc2 -v -ockcfns.o ram:ckcfns
echo "compiling ckcfn2:"
lc:lc1 <kerflags> -csw -iinclude:lattice/ -oram: ckcfn2
lc:lc2 -v -ockcfn2.o ram:ckcfn2
echo "compiling ckicon:"
lc:lc1 <kerflags> -csw -iinclude:lattice/ -oram: ckicon
lc:lc2 -v -ockicon.o ram:ckicon
echo "compiling ckitio:"
lc:lc1 <kerflags> -iinclude: -csw -iinclude:lattice/ -oram: ckitio
lc:lc2 -v -ockitio.o ram:ckitio
echo "compiling ckifio:"
lc:lc1 <kerflags> -csw -iinclude:lattice/ -oram: ckifio
lc:lc2 -v -ockifio.o ram:ckifio
echo "compiling ckistu:"
lc:lc1 <kerflags> -csw -iinclude:lattice/ -oram: ckistu
lc:lc2 -v -ockistu.o ram:ckistu
echo "compiling ckiutl:"
lc:lc1 <kerflags> -iinclude: -csw -iinclude:lattice/ -oram: ckiutl
lc:lc2 -v -ockiutl.o ram:ckiutl
echo "linking Kermit:"
lc:alink with ckiker.lnk faster
