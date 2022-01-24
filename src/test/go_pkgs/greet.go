package main

import (
    "fmt"
    "log"
    "net/http"
)

func main() {

    http.HandleFunc("/", HelloServer)
    fmt.Println("Server started at port 8080")
    log.Fatal(http.ListenAndServe(":8080", nil))
}

