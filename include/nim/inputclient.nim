#
# \brief  Input client
# \author Emery Hemingway
# \date   2017-10-15
#

#
# Copyright (C) 2017 Genode Labs GmbH
#
# This file is part of the Genode OS framework, which is distributed
# under the terms of the GNU Affero General Public License version 3.
#

import genode
include inputevent

type
  InputClient* = ptr InputClientObj
  InputClientObj {.importcpp: "Input::Session_client", header: "<input_session/client.h>".} = object
    ## Normally a Connection class is imported, but the Nitpicker connection
    ## contains an Input session in the form of the Rpc_client base class.

proc dataspace*(input: InputClient): DataspaceCapability {.tags: [RpcEffect], importcpp.}

proc pending*(input: InputClient): bool {.tags: [RpcEffect], importcpp.}

proc flush*(input: InputClient): int {.tags: [RpcEffect], importcpp.}

proc sigh*(input: InputClient; cap: SignalContextCapability) {.tags: [RpcEffect], importcpp.}

type EventBuffer {.unchecked.} = array[0, Event]
  ## Type to represent an unbounded Event array.

proc eventBuffer*(input: InputClient): ptr EventBuffer {.
  importcpp: "#->_event_ds.local_addr<void>()".}
  ## Requires breaking the C++ private/public rules.

iterator events*(input: InputClient): Event =
  ## Flush and iterate over input events.
  let
    buf = input.eventBuffer
    n = flush input
  for i in 0..<n:
    yield buf[i]
