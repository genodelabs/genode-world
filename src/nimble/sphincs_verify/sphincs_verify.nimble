# Package

version       = "0.1.0"
author        = "Emery Hemingway"
description   = "A post-quantum drop-in-replacement for the Depot verify tool"
license       = "GPLv3"
srcDir        = "src"
bin           = @["sphincs_verify"]

# Dependencies

requires "nim >= 0.18.0", "genode >= 18.7", "sphincs", "nimcrypto"
