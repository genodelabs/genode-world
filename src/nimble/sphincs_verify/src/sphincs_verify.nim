import std/xmltree, std/xmlparser, std/streams
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

proc readPk(path: string): Pk =
  let fs = newFileStream path
  if fs.isNil: raiseAssert("public key unavailable")
  defer: close fs
  if fs.readData(result.addr, sizeof(result)) != sizeof(result):
    raiseAssert("malformed public key")

proc verify(filePath, pkPath: string): XmlNode =
  try:
    let
      pk = readPk(pkPath)
      sig = readFile(filePath & ".sphincs")
    let
      (valid, msg) = pk.verify sig
    if not valid:
      result = <>bad(path=filePath, reason="bad signature")
    else:
      let digest = hashFile(filePath)
      if digest == msg:
        result = <>good(path=filePath)
      else:
        result = <>bad(path=filePath, reason="file digest mismatch")
  except:
    let e = getCurrentException()
    result = <>bad(path=filePath, reason=e.msg)

when defined(genode):
  import genode/reports, genode/roms

  proc xml(rom: RomClient): XmlNode =
    let s = rom.newStream
    result = s.parseXml
    close s

  componentConstructHook = proc (env: GenodeEnv) =
    let report = env.newReportClient("result")

    proc handleConfig(rom: RomClient) =
      let nodes = rom.xml.findAll("verify")
      var results = newSeq[XmlNode](nodes.len)
      for i, x in nodes.pairs:
        results[i] = verify(x.attr("path"), x.attr("pubkey"))

      report.submit do (s: Stream):
        let xml = <>result()
        for r in results.items:
          xml.add(r)
        s.writeLine(xml)
    let
      configHandler = env.newRomHandler("config", handleConfig)

    process configHandler
