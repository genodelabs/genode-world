# Package

version       = "0.1.0"
author        = "Emery Hemingway"
description   = "A post-quantum drop-in-replacement for the Depot verify tool"
license       = "GPLv3"
srcDir        = "src"
bin           = @["sphincs_keygen"]

# Dependencies

requires "nim >= 0.18.0", "sphincs"
