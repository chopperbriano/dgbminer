
if WANT_JANSSON
JANSSON_INCLUDES= -I$(top_srcdir)/compat/jansson
else
JANSSON_INCLUDES=
endif

EXTRA_DIST	= example-cfg.json nomacro.pl

SUBDIRS		= compat

ALL_INCLUDES	= @PTHREAD_FLAGS@ -fno-strict-aliasing $(JANSSON_INCLUDES) -I.

bin_PROGRAMS	= cpuminer

dist_man_MANS	= cpuminer.1

cpuminer_SOURCES = \
  cpu-miner.c \
  util.c \
  api.c \
  sysinfos.c \
  algo-gate-api.c\
  malloc-huge.c \
  algo/cubehash/cubehash_sse2.c\
  algo/cubehash/cube-hash-2way.c \
  algo/echo/sph_echo.c \
  algo/echo/echo-hash-4way.c \
  algo/echo/aes_ni/hash.c\
  algo/luffa/luffa_for_sse2.c \
  algo/luffa/luffa-hash-2way.c \
  algo/simd/nist.c \
  algo/simd/vector.c \
  algo/simd/simd-hash-2way.c \
  algo/qubit/qubit-gate.c \
  algo/qubit/qubit.c \
  algo/qubit/qubit-2way.c \
  algo/qubit/deep-gate.c \
  algo/qubit/deep-2way.c \
  algo/qubit/deep.c \
  algo/scrypt/scrypt.c \
  algo/scrypt/scrypt-core-4way.c \
  algo/scrypt/neoscrypt.c \
  algo/sha/sha256-hash.c \
  algo/sha/sph_sha2.c \
  algo/sha/sph_sha2big.c \
  algo/sha/sha256-hash-4way.c \
  algo/sha/sha512-hash-4way.c \
  algo/sha/sha256-hash-opt.c \
  algo/sha/sha256-hash-2way-ni.c \
  algo/sha/hmac-sha256-hash.c \
  algo/sha/hmac-sha256-hash-4way.c \
  algo/sha/sha256d.c \
  algo/sha/sha2.c \
  algo/sha/sha256d-4way.c \
  algo/sha/sha256t-gate.c \
  algo/sha/sha256t-4way.c \
  algo/sha/sha256t.c \
  algo/sha/sha256q-4way.c \
  algo/sha/sha256q.c \
  algo/sha/sha512256d-4way.c \
  algo/sha/sha256dt.c \
  algo/shavite/sph_shavite.c \
  algo/shavite/sph-shavite-aesni.c \
  algo/shavite/shavite-hash-2way.c \
  algo/shavite/shavite-hash-4way.c \
  algo/shavite/shavite.c \
  algo/skein/sph_skein.c \
  algo/skein/skein-hash-4way.c \
  algo/skein/skein.c \
  algo/skein/skein-4way.c \
  algo/skein/skein-gate.c \
  algo/skein/skein2.c \
  algo/skein/skein2-4way.c \
  algo/odo/odo.cpp \
  algo/odo/odocrypt.cpp \
  algo/odo/KeccakP-800-reference.cpp

disable_flags =

if USE_ASM
   cpuminer_SOURCES += asm/neoscrypt_asm.S
if ARCH_x86
   cpuminer_SOURCES += asm/sha2-x86.S asm/scrypt-x86.S asm/aesb-x86.S
endif
if ARCH_x86_64
   cpuminer_SOURCES += asm/sha2-x64.S asm/scrypt-x64.S asm/aesb-x64.S
endif
if ARCH_ARM
   cpuminer_SOURCES += asm/sha2-arm.S asm/scrypt-arm.S
endif
else
   disable_flags += -DNOASM
endif

if HAVE_WINDOWS
   cpuminer_SOURCES += compat/winansi.c
endif

cpuminer_LDFLAGS	= @LDFLAGS@
cpuminer_LDADD	= @LIBCURL@ @JANSSON_LIBS@ @PTHREAD_LIBS@ @WS2_LIBS@ -lssl -lcrypto -lgmp
cpuminer_CPPFLAGS = @LIBCURL_CPPFLAGS@ $(ALL_INCLUDES)
cpuminer_CFLAGS   = -Wno-pointer-sign -Wno-pointer-to-int-cast $(disable_flags)

if HAVE_WINDOWS
cpuminer_CFLAGS += -Wl,--stack,10485760
endif

if HAVE_WINDOWS
# use to profile an object
# gprof_cflags = -pg -g3
# cpuminer_LDFLAGS += -pg
# cpuminer_CFLAGS += -fno-inline-functions -static

# copy/paste from generated Makefile
common_ccflags = $(DEFS) $(ALL_INCLUDES) $(cpuminer_CPPFLAGS) $(CPPFLAGS) $(cpuminer_CFLAGS) $(CFLAGS)


cpuminer-neoscrypt.o: neoscrypt.c
	@echo "CUSTOM ${@}: ${filter %.o,${^}} ${filter %.c,${^}}"
	$(CC) $(common_ccflags) -g -O3 $(gprof_cflags) -MT $@ -MD -MP -c -o $@ $<


endif
