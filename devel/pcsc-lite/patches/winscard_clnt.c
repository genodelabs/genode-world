--- src/src/winscard_clnt.c.orig	2025-05-22 19:25:26.023887058 +0200
+++ src/src/winscard_clnt.c	2025-05-22 19:25:22.209838950 +0200
@@ -126,6 +126,8 @@ THIS SOFTWARE, EVEN IF ADVISED OF THE PO
 #include "winscard_msg.h"
 #include "utils.h"
 
+#include <stdio.h>
+
 /* Display, on stderr, a trace of the WinSCard calls with arguments and
  * results */
 //#define DO_TRACE
@@ -142,6 +144,16 @@ static bool sharing_shall_block = true;
 #define COLOR_MAGENTA "\33[35m"
 #define COLOR_NORMAL "\33[0m"
 
+#define GENODE 1
+
+#ifdef GENODE
+
+static int event_pipe[2];
+
+extern void initialize_pcsc_lite();
+
+#endif
+
 #ifdef DO_TRACE
 
 #include <stdio.h>
@@ -573,6 +585,7 @@ static LONG SCardEstablishContextTH(DWOR
 	if (!isExecuted)
 		return SCARD_E_NO_MEMORY;
 
+#ifndef GENODE
 	/* Establishes a connection to the server */
 	if (ClientSetupSession(&dwClientID) != 0)
 	{
@@ -609,6 +622,17 @@ static LONG SCardEstablishContextTH(DWOR
 			goto cleanup;
 		}
 	}
+#else
+
+	initialize_pcsc_lite();
+
+	rv = pipe(event_pipe);
+	if (rv < 0) {
+		fprintf(stderr, "Could not create pipe\n");
+		return rv;
+	}
+
+#endif
 
 again:
 	/*
