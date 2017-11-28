#
# \brief  Base Genode support for Nim
# \author Emery Hemingway
# \date   2017-10-15
#

#
# Copyright (C) 2017 Genode Labs GmbH
#
# This file is part of the Genode OS framework, which is distributed
# under the terms of the GNU Affero General Public License version 3.
#

when not defined(genode) or defined(nimdoc):
  {.error: "Genode only module".}

type RpcEffect* = object of RootEffect
  ## Effect describing a synchronous client-side RPC.

#
# C++ utilities
#

type Constructible* {.
  importcpp: "Genode::Constructible", header: "<util/reconstructible.h>", final, pure.} [T] = object

proc construct*[T](x: Constructible[T]) {.importcpp.}
  ## Construct a constructible C++ object.

proc destruct*[T](x: Constructible[T]) {.importcpp.}
  ## Destruct a constructible C++ object.

#
# Signals
#

const SignalsH = "nim/signals.h"

type
  SignalContextCapability* {.
    importcpp: "Genode::Signal_context_capability",
    header: "<base/signal.h>", final, pure.} = object
    ## Capability to an asynchronous signal context.

  SignalDispatcherCpp {.importcpp, header: SignalsH, final, pure.} = object
    cap {.importcpp.}: SignalContextCapability

  SignalDispatcher* = ref object
    cpp: SignalDispatcherCpp
    handler*: proc() {.closure.}

proc init(cpp: SignalDispatcherCpp; sh: SignalDispatcher)
  {.importcpp: "#.init(&genodeEnv->ep(), #)".}

proc deinit(sd: SignalDispatcherCpp) {.importcpp.}

proc newSignalDispatcher*(): SignalDispatcher =
  new result
  init result.cpp, result

proc dissolve*(sig: SignalDispatcher) =
  ## Dissolve signal dispatcher from entrypoint.
  deinit sig.cpp
  sig.handler = nil

proc cap*(sig: SignalDispatcher): SignalContextCapability = sig.cpp.cap
  ## Signal context capability. Can be delegated to external components.

proc nimHandleSignal(p: pointer) {.exportc.} =
  ## C symbol invoked by entrypoint during signal dispatch.
  let dispatch = cast[SignalDispatcher](p)
  if not dispatch.handler.isNil:
    dispatch.handler()

#
# Sessions
#

type SessionCapability* {.
  importcpp: "Genode::Session_capability",
  header: "<session/capability.h>", final, pure.} = object
  ## Capability to a session.

#
# Dataspaces
#

import streams

type
  DataspaceCapability* {.
    importcpp: "Genode::Dataspace_capability", header: "dataspace/capability.h".} = object

proc isValid*(cap: DataspaceCapability): bool {.
  importcpp: "#.valid()", tags: [RpcEffect].}

proc size*(cap: DataspaceCapability): int {.
  importcpp: "Genode::Dataspace_client(@).size()", header: "dataspace/client.h",
  tags: [RpcEffect].}

proc allocDataspace*(size: int): DataspaceCapability {.
   importcpp: "genodeEnv->pd().alloc(#)", tags: [RpcEffect].}

proc freeDataspace*(cap: DataspaceCapability) {.
   importcpp: "genodeEnv->pd().free(#)", tags: [RpcEffect].}

proc attach*(cap: DataspaceCapability): ByteAddress {.
   importcpp: "genodeEnv->rm().attach(@)", tags: [RpcEffect].}

proc detach*(p: ByteAddress) {.
  importcpp: "genodeEnv->rm().detach(@)", tags: [RpcEffect].}

type
  DataspaceStream* = ref DataspaceStreamObj
    ## a stream that provides safe access to dataspace content
  DataspaceStreamObj* = object of StreamObj
    cap: DataspaceCapability
    base: ByteAddress
    size: int
    pos: int

proc size*(ds: DataspaceStream): int = ds.size

proc dsClose(s: Stream) =
  var s = DataspaceStream(s)
  if s.base != 0:
    detach s.base
    s.base = 0
  s.size = 0
  s.pos = 0

proc dsAtEnd(s: Stream): bool =
  var s = DataspaceStream(s)
  result = s.pos <= s.size

proc dsSetPosition(s: Stream, pos: int) =
  var s = DataspaceStream(s)
  s.pos = clamp(pos, 0, s.size)

proc dsGetPosition(s: Stream): int =
  result = DataspaceStream(s).pos

proc dsPeekData(s: Stream, buffer: pointer, bufLen: int): int =
  var s = DataspaceStream(s)
  result = min(bufLen, s.size - s.pos)
  if result > 0:
    copyMem(buffer, cast[pointer](cast[int](s.base) + s.pos), result)

proc dsReadData(s: Stream, buffer: pointer, bufLen: int): int =
  var s = DataspaceStream(s)
  result = min(bufLen, s.size - s.pos)
  if result > 0:
    copyMem(buffer, cast[pointer](cast[int](s.base) + s.pos), result)
    inc(s.pos, result)

proc dsWriteData(s: Stream, buffer: pointer, bufLen: int) =
  var s = DataspaceStream(s)
  let count = clamp(bufLen, 0, s.size - s.pos)
  copyMem(cast[pointer](cast[int](s.base) + s.pos), buffer, count)
  inc(s.pos, count)

proc newDataspaceStream*(cap: DataspaceCapability): DataspaceStream =
  result = DataspaceStream(
    closeImpl: dsClose,
    atEndImpl: dsAtEnd,
    setPositionImpl: dsSetPosition,
    getPositionImpl: dsGetPosition,
    readDataImpl: dsReadData,
    peekDataImpl: dsPeekData,
    writeDataImpl: dsWriteData)
  if cap.isValid:
    result.cap = cap
    result.base = attach cap
    result.size = cap.size

proc update*(ds: DataspaceStream; cap: DataspaceCapability) =
  ds.pos = 0
  if cap.isValid:
    ds.cap = cap
    ds.base = attach cap
    ds.size = cap.size
  else:
    ds.base = 0
    ds.size = 0
