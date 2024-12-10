mockserver-arm:
	GOARCH=arm GOARM=5 go build -o mockserver-arm  .

mockserver:
	go build .

clean:
	rm mockserver mockserver-arm

veryclean: clean
	rm command/command.pb.go

command/command.pb.go:
	protoc -I=../server/armv5-unknown-linux-gnueabihf --go_out=. --go_opt=Mcommand.proto=/command command.proto
