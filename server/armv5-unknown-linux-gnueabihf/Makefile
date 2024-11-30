ARCH=$(shell uname -m)

command.pb.cc:
	../../protobuf/build-$(ARCH)/bin/protoc -I=. --cpp_out=. command.proto

.PHONY: command.pb.cc