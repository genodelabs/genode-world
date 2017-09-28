TARGET  = hotkey_edit
LIBS    = base libc
SRC_NIM = main.nim

# Peek inside Input::Client
CC_OPT += -Dprivate=public
