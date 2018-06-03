TARGET  = gtotp_report
LIBS   += base cryptopp stdcxx
SRC_CC += component.cc

# Effc++ not compatible with CryptoPP headers
CC_CXX_WARN_STRICT =
