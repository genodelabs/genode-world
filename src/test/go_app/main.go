// package main

// import (
//     "fmt"
// 	"strings"
// 	"runtime"
// )

// func main() {
// 	fmt.Println("cpu number", runtime.NumCPU())
//     c0 := make(chan string)
//     c1 := make(chan string)
//     go sourceGopher(c0)
//     go splitWords(c0, c1)
//     printGopher(c1)
// }

// func sourceGopher(downstream chan string) {
//     for _, v := range []string{"hello world", "a bad apple", "goodbye all"} {
//         downstream <- v
//     }
//     close(downstream)
// }

// func splitWords(upstream, downstream chan string) {
//     for v := range upstream {
//         for _, word := range strings.Fields(v) {
//             downstream <- word
//         }
//     }
//     close(downstream)
// }

// func printGopher(upstream chan string) {
//     for v := range upstream {
//         fmt.Println(v)
//     }
// }

// package main

// import "fmt"

// func main() {
// 	println("main !")
// 	fmt.Printf("hello, world\n")
// }

// package main

// import (
// 	"fmt"
// 	"time"
// )

// const Towers = 3
// const Disks = 5

// type Hanoi [Towers][]int

// func main() {
// 	var state Hanoi
// 	state.init(Disks)
// 	state.move(Disks, 0, 1, 2)
// }

// func (h *Hanoi) init(n int) {
// 	h[0] = make([]int, n)
// 	for i := range h[0] {
// 		h[0][i] = n - i
// 	}
// 	h.print()
// }

// func (h *Hanoi) move(n, a, b, c int) {
// 	if n <= 0 {
// 		return
// 	}
// 	h.move(n-1, a, c, b)
// 	disk := h[a][len(h[a])-1]
// 	h[a] = h[a][:len(h[a])-1]
// 	h[c] = append(h[c], disk)
// 	h.print()
// 	h.move(n-1, b, a, c)
// }

// func (h *Hanoi) print() {
// 	fmt.Print("\f")
// 	for i := Disks; i >= 0; i-- {
// 		for j := 0; j < Towers; j++ {
// 			if i == 0 {
// 				fmt.Print("_/||\\_")
// 			} else if len(h[j]) >= i {
// 				fmt.Printf("  %02d  ", h[j][i-1])
// 			} else {
// 				fmt.Print("  ||  ")
// 			}
// 		}
// 		fmt.Println()
// 	}
// 	fmt.Println()
// 	time.Sleep(time.Second / 5)
// }

// 

package main

import (
	"fmt"
	"sync"
	"time"
)

const (
	sleeping = iota
	checking
	cutting
)

var stateLog = map[int]string{
	0: "Sleeping",
	1: "Checking",
	2: "Cutting",
}
var wg *sync.WaitGroup // Amount of potentional customers

type Barber struct {
	name string
	sync.Mutex
	state    int // Sleeping/Checking/Cutting
	customer *Customer
}

type Customer struct {
	name string
}

func (c *Customer) String() string {
	return fmt.Sprintf("%7p", c)[7:]
}

func NewBarber() (b *Barber) {
	return &Barber{
		name:  "Sam",
		state: sleeping,
	}
}

// Barber goroutine
// Checks for customers
// Sleeps - wait for wakers to wake him up
func barber(b *Barber, wr chan *Customer, wakers chan *Customer) {
	for {
		b.Lock()
		defer b.Unlock()
		b.state = checking
		b.customer = nil

		// checking the waiting room
		fmt.Printf("Checking waiting room: %d\n", len(wr))
		time.Sleep(time.Millisecond * 100)
		select {
		case c := <-wr:
			HairCut(c, b)
			b.Unlock()
		default: // Waiting room is empty
			fmt.Printf("Sleeping Barber - %s\n", b.customer)
			b.state = sleeping
			b.customer = nil
			b.Unlock()
			c := <-wakers
			b.Lock()
			fmt.Printf("Woken by %s\n", c)
			HairCut(c, b)
			b.Unlock()
		}
	}
}

func HairCut(c *Customer, b *Barber) {
	b.state = cutting
	b.customer = c
	b.Unlock()
	fmt.Printf("Cutting  %s hair\n", c)
	time.Sleep(time.Millisecond * 100)
	b.Lock()
	wg.Done()
	b.customer = nil
}

// customer goroutine
// just fizzles out if it's full, otherwise the customer
// is passed along to the channel handling it's haircut etc
func customer(c *Customer, b *Barber, wr chan<- *Customer, wakers chan<- *Customer) {
	// arrive
	time.Sleep(time.Millisecond * 50)
	// Check on barber
	b.Lock()
	fmt.Printf("Customer %s checks %s barber | room: %d, w %d - customer: %s\n",
		c, stateLog[b.state], len(wr), len(wakers), b.customer)
	switch b.state {
	case sleeping:
		select {
		case wakers <- c:
		default:
			select {
			case wr <- c:
			default:
				wg.Done()
			}
		}
	case cutting:
		select {
		case wr <- c:
		default: // Full waiting room, leave shop
			wg.Done()
		}
	case checking:
		panic("Customer shouldn't check for the Barber when Barber is Checking the waiting room")
	}
	b.Unlock()
}

func main() {
	b := NewBarber()
	b.name = "Rocky"
	WaitingRoom := make(chan *Customer, 5) // 5 chairs
	Wakers := make(chan *Customer, 1)      // Only one waker at a time
	go barber(b, WaitingRoom, Wakers)

	time.Sleep(time.Millisecond * 100)
	wg = new(sync.WaitGroup)
	n := 10
	wg.Add(10)
	// Spawn customers
	for i := 0; i < n; i++ {
		time.Sleep(time.Millisecond * 50)
		c := new(Customer)
		go customer(c, b, WaitingRoom, Wakers)
	}

	wg.Wait()
	fmt.Println("No more customers for the day")
}
