#
# \brief  Report client
# \author Emery Hemingway
# \date   2017-10-15
#

#
# Copyright (C) 2017 Genode Labs GmbH
#
# This file is part of the Genode OS framework, which is distributed
# under the terms of the GNU Affero General Public License version 3.
#

import genode, streams

const ReportH = "<report_session/connection.h>"

type
  ConnectionBase {.
    importcpp: "Report::Connection", header: ReportH.} = object
  Connection = Constructible[ConnectionBase]

  ReportClient* = ref ReportClientObj
  ReportClientObj = object
    conn: Connection
    stream*: DataspaceStream

proc construct(c: Connection, label: cstring, bufferSize: csize) {.
  importcpp: "#.construct(*genodeEnv, @)", tags: [RpcEffect].}

proc destruct(c: Connection) {.tags: [RpcEffect],
  importcpp: "#.destruct()".}

proc dataspace(c: Connection): DataspaceCapability {.tags: [RpcEffect],
  importcpp: "#->dataspace()".}

proc submit(c: Connection, n: csize) {.tags: [RpcEffect],
  importcpp: "#->submit(#)".}

proc newReportClient*(label: string, bufferSize = 4096): ReportClient=
  new result
  construct result.conn, label, bufferSize
  result.stream = newDataspaceStream(result.conn.dataspace)

proc submit*(report: ReportClient) =
  report.conn.submit(report.stream.size)

proc close*(report: ReportClient) =
  close report.stream
  destruct report.conn
