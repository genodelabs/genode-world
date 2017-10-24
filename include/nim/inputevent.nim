#
# \brief  Input event descriptions
# \author Emery Hemingway
# \date   2017-10-15
#

#
# Copyright (C) 2017 Genode Labs GmbH
#
# This file is part of the Genode OS framework, which is distributed
# under the terms of the GNU Affero General Public License version 3.
#

const InputH = "<input/event.h>"

include keycodes

type
  Event* {.importcpp: "Input::Event", header: InputH.} = object
  Type* {.importcpp: "Input::Event::Type", header: InputH.} = enum
    INVALID, MOTION, PRESS, RELEASE, WHEEL, FOCUS, LEAVE, TOUCH, CHARACTER

proc typ*(ev: Event): Type {.importcpp: "#.type()".}
proc code*(ev: Event): KeyCode {.importcpp.}
proc ax*(ev: Event): cint {.importcpp.}
proc ay*(ev: Event): cint {.importcpp.}
proc rx*(ev: Event): cint {.importcpp.}
proc ry*(ev: Event): cint {.importcpp.}

proc key_name(c: KeyCode): cstring {.
  importcpp: "Input::key_name(#)", header: InputH.}
proc `$`*(c: KeyCode): string = $key_name(c)

proc lookupKey*(s: string): KeyCode =
  result = KEY_UNKNOWN.KeyCode
  for k in 0..<KEY_MAX.KeyCode:
    if $k == s:
      return k
