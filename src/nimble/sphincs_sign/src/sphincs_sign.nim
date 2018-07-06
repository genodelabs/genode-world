import std/parseopt, std/streams
import sphincs/shake256_192f
import nimcrypto.hash, nimcrypto/keccak

proc hashFile(path: string): string =
  let fs = openFileStream(path, fmRead)
  result = newString(32)
  var
    ctx: sha3_256
    buf: array[512, byte]
  let bp = addr buf[0]
  init ctx
  while not fs.atEnd:
    let n = fs.readData(bp, buf.len)
    ctx.update(bp, n.uint)
  close fs
  var d = finish ctx
  copyMem(result[0].addr, d.data[0].addr, result.len)

proc readPair(path: string): KeyPair =
  let fs = openFileStream path
  defer: close fs
  doAssert(fs.readData(result.addr, sizeof(result)) == sizeof(result))

let rngStr = openFileStream("/dev/random")
proc readDevRand(p: pointer; size: int) =
  let n = rngStr.readData(p, size)
  doAssert(n == size, "short read from RNG")

proc signPath(pair: KeyPair; path: string) =
  let
    digest = hashFile path
    sig = pair.sign(digest, readDevRand)
  writeFile(path & ".sphincs", sig)

proc main() =

  var
    pair: KeyPair
    argi = 0
  for kind, key, val in getopt():
    if kind != cmdArgument:
      quit("invalid argument " & key & val)
    if argi == 0:
      pair = readPair(key)
    else:
      signPath(pair, key)
    inc argi

  if argi == 0:
    echo "usage: sphincs_sign [SECRET_KEY] [FILE]"

main()
