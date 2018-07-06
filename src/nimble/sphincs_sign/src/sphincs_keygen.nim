import std/streams
import sphincs/shake256_256f

let rngStr = openFileStream("/dev/random")
proc readDevRand(p: pointer; size: int) =
  let n = rngStr.readData(p, size)
  doAssert(n == size, "short read from RNG")

proc writeToFile[T](path: string; x: var T) =
  let fs = openFileStream(path, fmWrite)
  fs.writeData(x.addr, sizeof(x))
  close fs

var pair = shake256_256f.generateKeypair(readDevRand)
writeToFile("secret", pair)
writeToFile("public", pair.pk)
