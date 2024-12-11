package main

import (
	"mockserver/server"
)

func main() {
	v := server.New("/tmp/wampy.sock")
	v.Start()
}
