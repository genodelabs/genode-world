#
# \brief  Nitpicker client
# \author Emery Hemingway
# \date   2017-10-15
#

#
# Copyright (C) 2017 Genode Labs GmbH
#
# This file is part of the Genode OS framework, which is distributed
# under the terms of the GNU Affero General Public License version 3.
#

import genode, inputclient

const NitpickerH = "<nitpicker_session/connection.h>"

type
  ConnectionBase {.
    importcpp: "Nitpicker::Connection", header: NitpickerH.} = object
  Connection = Constructible[ConnectionBase]

  NitpickerClient* = ref NitpickerClientObj
  NitpickerClientObj = object
    conn: Connection

proc construct(conn: Connection; label: cstring) {.
  importcpp: "#.construct(#)".}

proc newNitpickerClient*(label = ""): NitpickerClient =
  new result
  construct result.conn, label

proc close*(np: NitpickerClient) =
  destruct np.conn

type Mode* {.importcpp: "Framebuffer::Mode", header: NitpickerH.} = object

proc width*(m: Mode): int {.importcpp.}
proc height*(m: Mode): int {.importcpp.}

proc mode(conn: Connection): Mode {.
  importcpp: "#->mode()".}

proc mode*(np: NitpickerClient): Mode = mode np.conn
  ## Return physical screen mode.

proc mode_sigh(conn: Connection; cap: SignalContextCapability) {.
  importcpp: "#->mode_sigh(#)".}

proc modeSigh*(np: NitpickerClient; cap: SignalContextCapability) =
  ## Register signal handler to be notified about mode changes.
  np.conn.modeSigh cap

proc input(conn: Connection): InputClient {.
  importcpp: "#->input()".}

proc input*(np: NitpickerClient): InputClient =
  ## Return an object representing the Input sub-session.
  result = np.conn.input()