// # To execute this run script on your Linux host you have to do some
// # preparation:
// #
// # 1) Setup a TAP device:
// #    ! export USER=[YOUR_USER_NAME]
// #    ! export TAP_DEV=tap0
// #    ! sudo ip tuntap add dev $TAP_DEV mode tap user $USER
// #    ! sudo ip address flush dev $TAP_DEV
// #    ! sudo ip address add 10.0.2.1/24 brd 10.0.2.255 dev $TAP_DEV
// #    ! sudo ip link set dev $TAP_DEV addr 02:00:00:ca:fe:01
// #    ! sudo ip link set dev $TAP_DEV up
// #
// # 2) Now, start the test and connect using your favorite HTTP client, e.g.:
// #    ! cd build/x86_64
// #    ! make run/lwip_lx KERNEL=linux BOARD=linux
// #    ! lynx -dump http://10.0.2.55/world
// #
// # 3) Clean up your Linux when done testing:
// #    ! sudo ip tuntap delete $TAP_DEV mode tap
// #

package main

import (
    "fmt"
    "log"
    "net/http"
)

func main() {

    http.HandleFunc("/", HelloServer)
    fmt.Println("Server started at port 80")
    log.Fatal(http.ListenAndServe(":80", nil))
}

func HelloServer(w http.ResponseWriter, r *http.Request) {
    fmt.Fprintf(w, "Hello, %s!\n", r.URL.Path[1:])
}


