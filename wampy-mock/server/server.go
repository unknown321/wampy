package server

import (
	"fmt"
	"github.com/golang/protobuf/proto"
	"log"
	"log/slog"
	"net"
	"os"
	"os/signal"
	"syscall"
	"time"
	"mockserver/command"
)

type Server struct {
	socketAddr string
}

func New(addr string) *Server {
	return &Server{socketAddr: addr}
}

var ReplyOK = []byte("OK\n")

func (s *Server) Start() {
	_ = os.Remove(s.socketAddr)
	socket, err := net.Listen("unix", s.socketAddr)
	if err != nil {
		slog.Error("cannot start server", "address", s.socketAddr, "err", err.Error())
		return
	} else {
		slog.Info("ok")
	}

	// Cleanup the sockfile.
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt, syscall.SIGTERM)
	go func() {
		<-c
		_ = os.Remove(s.socketAddr)
		os.Exit(1)
	}()

	visible := true
	swap := true
	swapRequests := 5 * 2 // every 5 requests
	counter := 0
	swapToggle := false

	fmt.Printf("started socket server on %s\n", s.socketAddr)
	for {
		socketConnection, err := socket.Accept()
		if err != nil {
			log.Fatal(err)
		}
		slog.Info("new connection")

		// Handle the connection in a separate goroutine.
		go func(conn net.Conn) {
			slog.Info("handling")
			defer func(conn net.Conn) {
				_ = conn.Close()
				slog.Info("closed")
			}(conn)

			buf := make([]byte, 4096)

			inc := []byte{}
			length := 0
			for {
				// Read data from the connection.
				conn.SetReadDeadline(time.Now().Add(time.Millisecond * 20))
				n, err := conn.Read(buf)
				if err != nil {
					//slog.Error("cannot read from socket", "err", err.Error())
					break
				}
				slog.Info("read", "bytes", n)

				if n < 1 {
					break
				} else {
					length += n
					inc = append(inc, buf...)
				}
			}

			if len(inc) > 0 {
				inc = inc[:length]
				//inc = bytes.Trim(inc, "\x00")
				comm := command.Command{}
				if err = proto.Unmarshal(inc, &comm); err != nil {
					slog.Error("cannot unmarshal", "err", err.Error())
					return
				}

				slog.Info("command", "id", comm.GetType().String())

				var res *command.Command
				codeOk := command.ResponseCode(command.ResponseCode_OK)

				var t command.Type

				switch comm.GetType() {
				case command.Type_CMD_GET_WINDOW_STATUS:
					vis := command.WindowVisible_VISIBILITY_NO
					if visible {
						vis = command.WindowVisible_VISIBILITY_YES
					}
					ws := command.Command_WindowStatus{WindowStatus: &command.WindowStatus{Visible: &vis}}
					t = command.Type_CMD_GET_WINDOW_STATUS
					res = &command.Command{Code: &codeOk, Type: &t, Msg: &ws}
				case command.Type_CMD_FEATURE_BIG_COVER:
					t = command.Type_CMD_FEATURE_BIG_COVER
					res = &command.Command{Type: &t, Code: &codeOk}
				case command.Type_CMD_FEATURE_SHOW_CLOCK:
					t = command.Type_CMD_FEATURE_SHOW_CLOCK
					res = &command.Command{Type: &t, Code: &codeOk}
				case command.Type_CMD_SHOW_WINDOW:
					t = command.Type_CMD_SHOW_WINDOW
					res = &command.Command{Type: &t, Code: &codeOk}
					visible = true
				case command.Type_CMD_HIDE_WINDOW:
					t = command.Type_CMD_HIDE_WINDOW
					res = &command.Command{Type: &t, Code: &codeOk}
					visible = false
				case command.Type_CMD_GET_STATUS:
					t = command.Type_CMD_GET_STATUS
					tr1 := &command.Track{
						Track:    proto.Int32(1),
						Artist:   proto.String("Arianne Schreiber"),
						Title:    proto.String("Komm, Süsser Tod / 甘き死よ、来たれ (M-10 Director’s Edit. Version)"),
						Duration: proto.Int32(7*60 + 46),
						Active:   proto.Bool(true),
					}
					tr2 := &command.Track{
						Track:    proto.Int32(2),
						Artist:   proto.String("やくしまるえつこ"),
						Title:    proto.String("絶対ムッシュ制"),
						Duration: proto.Int32(3*60 + 54),
						Active:   proto.Bool(false),
					}
					tr3 := &command.Track{
						Track:    proto.Int32(3),
						Artist:   proto.String("DragonForce"),
						Title:    proto.String("Heart Of Storm (Alternative Chorus Ver.) (Digital Only Bonus Track)"),
						Duration: proto.Int32(3*60 + 54),
						Active:   proto.Bool(false),
					}

					pl := command.Playlist{}
					if swap {
						counter++

						if swapToggle {
							tr3.Track = proto.Int32(1)
							tr3.Active = proto.Bool(true)
							tr1.Track = proto.Int32(3)
							tr1.Active = proto.Bool(false)
							pl.Track = []*command.Track{tr3, tr2, tr1}
						} else {
							tr3.Track = proto.Int32(3)
							tr3.Active = proto.Bool(false)
							tr1.Track = proto.Int32(1)
							tr1.Active = proto.Bool(true)
							pl.Track = []*command.Track{tr1, tr2, tr3}
						}

						if counter > swapRequests {
							counter = 0
							swapToggle = !swapToggle
						}
					} else {
						pl.Track = []*command.Track{tr1, tr2, tr3}
					}

					st := command.Status{
						Codec:      proto.String("flac"),
						Elapsed:    proto.Int32(0),
						PlayState:  proto.Int32(1),
						HiRes:      proto.Bool(false),
						Shuffle:    proto.Bool(false),
						Repeat:     proto.Int32(0),
						Volume:     proto.Int32(50),
						BitRate:    proto.Int32(285),
						SampleRate: proto.Float32(44),
						BitDepth:   proto.Int32(10),
						Playlist:   &pl,
					}

					cst := command.Command_Status{
						Status: &st,
					}

					res = &command.Command{Type: &t, Code: &codeOk, Msg: &cst}
				}

				resp := []byte{}
				if resp, err = proto.Marshal(res); err != nil {
					slog.Error("cannot marshal", "err", err.Error())
					return
				}

				_, err = conn.Write(resp)
				if err != nil {
					slog.Error("cannot write to socket", "err", err.Error())
				}
			}
		}(socketConnection)
	}
}
