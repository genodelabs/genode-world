#
# \brief  Hotkey XML editor
# \author Emery Hemingway
# \date   2017-09-28
#

#
# Copyright (C) 2017 Genode Labs GmbH
#
# This file is part of the Genode OS framework, which is distributed
# under the terms of the GNU General Public License version 2.
#

import streams, tables, strtabs, xmlparser, xmltree,
  genode, reportclient, romclient, nitpickerclient, inputclient

var
  keyActions = initTable[KeyCode, XmlNode]()
let
  configRom = newRomClient("config")
  editReport = newReportClient("xml_editor")
  nitClient = newNitpickerClient("input")

let
  input = nitClient.input
  inputDispatch = newSignalDispatcher()

input.sigh inputDispatch.cap

proc submit(action: XmlNode) =
  editReport.stream.setPosition 0
  editReport.stream.writeLine "<edit>", action, "</edit>", 0.char
  submit editReport

inputDispatch.handler = proc() =
  for ev in input.events:
    if ev.typ == RELEASE and keyActions.contains ev.code:
      submit keyActions[ev.code]

proc xml(rom: RomClient): XmlNode =
  ## Parse ROM content as XML.
  rom.stream.setPosition 0
  try: result = parseXml rom.stream
  except XmlError: result = newElement "empty"

proc configHandler() =
  update configRom
  clear keyActions
  let config = configRom.xml
  for node in config.findAll("key").items:
    try:
      let
        keyName = node.attrs["name"]
        keyCode = lookupKey keyName
      if keyActions.contains keyCode:
        echo "discarding duplicate action for key ", keyName
      else:
        keyActions[keyCode] = node[0]
    except:
      discard

configRom.handler = configHandler
  # set the ROM callback
configHandler()
  # parse the config

echo "--- hotkey_edit returning to entrypoint ---"
