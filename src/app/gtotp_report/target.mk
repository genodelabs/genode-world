TARGET  = gtotp_report
LIBS   += cryptopp stdcxx libc base
SRC_CC += component.cc

# Effc++ not compatible with CryptoPP headers
CC_CXX_WARN_STRICT =
