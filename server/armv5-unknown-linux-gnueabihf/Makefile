ARCH=$(shell uname -m)

command.pb.cc:
	../../libs/protobuf/build-$(ARCH)/bin/protoc -I=. --cpp_out=. command.proto

.PHONY: command.pb.cc
