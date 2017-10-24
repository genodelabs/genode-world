#
# \brief  Audio_out client
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

const
  Header = "<audio_out_session/connection.h>"
  PacketPeriod* = 512
  PacketQueueSize*  = 256

type
  Packet* = ptr PacketObj
  PacketObj {.importcpp: "Audio_out::Packet", header: Header, pure.} = object

  PeriodBuffer* = ptr array[PacketPeriod, cfloat]
  StereoPacket* = tuple
    left, right: Packet

proc content(pkt: Packet | PacketObj): PeriodBuffer {.importcpp.}
proc played(pkt: Packet | PacketObj): bool {.importcpp.}
proc valid(pkt: Packet | PacketObj): bool {.importcpp.}
proc invalidate(pkt: Packet | PacketObj) {.importcpp.}
proc mark_as_played(pkt: Packet | PacketObj) {.importcpp.}

proc buffer*(pkt: Packet): PeriodBuffer = cast[PeriodBuffer](pkt.content())

proc `[]`*(pkt: Packet; i: int): float32 {.importcpp: "#->content()[#] ".}

proc `[]=`*(pkt: Packet; i: int; x: float32) {.importcpp: "#->content()[#] = #".}

proc `[]=`*(pkt: Packet; i: int; x: int16) {.importcpp: "#->content()[#] = # / 32768.0".}

type
  Stream = ptr StreamObj
  StreamObj {.importcpp: "Audio_out::Stream", header: Header, pure.} = object

proc pos(s: Stream): cuint {.importcpp.}
proc tail(s: Stream): cuint {.importcpp.}
proc queued(s: Stream): cuint {.importcpp.}
proc next(s: Stream): Packet {.importcpp.}
proc next(s: Stream; p: Packet): Packet {.importcpp.}
proc packet_position(s: Stream): Packet {.importcpp.}
proc packet_position(s: Stream; p: Packet): cuint {.importcpp.}
proc empty(s: Stream): bool {.importcpp.}
proc full(s: Stream): bool {.importcpp.}
proc get(s: Stream; pos: cuint): Packet {.importcpp.}
proc alloc(s: Stream): Packet {.importcpp.}
proc reset(s: Stream) {.importcpp.}
proc invalidate_all(s: Stream) {.importcpp.}
proc pos(s: Stream; p: cuint) {.importcpp.}
proc increment_position(s: Stream; p: cuint) {.importcpp.}

type
  ConnectionBase {.
    importcpp: "Audio_out::Connection", header: Header.} = object
  Connection = Constructible[ConnectionBase]

  AudioOutClient* = ref AudioOutClientObj
  AudioOutClientObj = object
    left, right: Connection

proc construct(conn: Connection; label: cstring) {.tags: [RpcEffect],
  importcpp: "#.construct(*genodeEnv, @, false, false)".}

proc destruct(conn: Connection) {.tags: [RpcEffect],
  importcpp: "#.destruct()".}

proc stream(conn: Connection): Stream {.importcpp: "#->stream()".}

proc start(conn: Connection) {.tags: [RpcEffect], importcpp: "#->start()".}
proc stop(conn: Connection) {.tags: [RpcEffect], importcpp: "#->stop()".}

proc progress_sigh(conn: Connection; sig: SignalContextCapability) {.tags: [RpcEffect],
  importcpp: "#->progress_sigh(#)".}

proc alloc_sigh(conn: Connection; sig: SignalContextCapability) {.tags: [RpcEffect],
  importcpp: "#->alloc_sigh(#)".}

proc submit(conn: Connection; pkt: Packet) {.tags: [RpcEffect],
  importcpp: "#->submit(#)".}

proc newAudioOutClient*(): AudioOutClient =
  new result
  result.left.construct("left")
  result.right.construct("right")

proc close*(dac: AudioOutClient) {.tags: [RpcEffect].} =
  destruct dac.left
  destruct dac.right

proc start*(dac: AudioOutClient) =
  start dac.left
  start dac.right

proc stop*(dac: AudioOutClient) =
  stop dac.left
  stop dac.right

proc setAllocSigh*(dac: AudioOutClient; cap: SignalContextCapability) =
  ## Install a signal handler to detect when the packet buffer ceases to be full.
  dac.left.alloc_sigh cap

proc setProgressSigh*(dac: AudioOutClient; cap: SignalContextCapability) =
  ## Install a signal handler to detect when a packet had been played.
  dac.left.progress_sigh cap

proc queued*(dac: AudioOutClient): uint =
  ## Return the number of packets queued.
  dac.left.stream.queued

proc full*(dac: AudioOutClient): bool =
  ## Check if the packet stream is full.
  dac.left.stream.full

iterator tailSubmission*(dac: AudioOutClient): Packet =
  ## Return the packets and the end of the buffer then submit.
  doAssert(not dac.full, "cannot submit packets, stream is full")
  let
    lp = dac.left.stream.alloc()
    rp = dac.right.stream.alloc()
  yield lp
  yield rp
  dac.left.submit lp
  dac.left.submit rp

proc next*(dac: AudioOutClient): StereoPacket =
  ## Return the successive packets from the current playback position.
  (dac.left.stream.next, dac.right.stream.next)

proc next*(dac: AudioOutClient, pkts: StereoPacket): StereoPacket =
  ## Return the successive packets from the packet buffers.
  (dac.left.stream.next(pkts.left), dac.right.stream.next(pkts.right))

proc alloc*(dac: AudioOutClient): StereoPacket =
  ## Return the first pair of unused packets.
  (dac.left.stream.alloc(), dac.right.stream.alloc())

proc submit*(dac: AudioOutClient, pkts: StereoPacket) =
  ## Mark a pair of packets as active and wake the servers.
  dac.left.submit pkts.left
  dac.right.submit pkts.right
