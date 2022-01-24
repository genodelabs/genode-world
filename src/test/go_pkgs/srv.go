package main

import (
    "fmt"
    "net/http"
)

func HelloServer(w http.ResponseWriter, r *http.Request) {
    fmt.Fprintf(w, "Hello, %s!\n", r.URL.Path[1:])
}
