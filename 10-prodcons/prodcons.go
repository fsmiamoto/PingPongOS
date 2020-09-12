package main

import (
	"fmt"
	"math/rand"
)

func consumer(id int, c chan int) {
	for {
		value := <-c
		fmt.Printf("                C%d consumed %d\n", id, value)
	}
}

func producer(id int, c chan int) {
	for {
		value := rand.Int() % 100
		c <- value
		fmt.Printf("P%d produced %d\n", id, value)
	}
}

func main() {
	const numOfConsumers = 2
	const numOfProducers = 3

	c := make(chan int, numOfConsumers+numOfProducers)

	for i := 0; i < numOfProducers; i++ {
		go producer(i+1, c)
	}
	for i := 0; i < numOfConsumers; i++ {
		go consumer(i+1, c)
	}

	for {
	}
}
