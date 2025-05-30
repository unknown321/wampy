#include "server.moc"
#include "moc_server.cpp"

#include "command_names.h"
#include "helpers.h"
#include <QThread>
#include <QtNetwork/QLocalSocket>
#include <stdio.h>
#include <thread>

#include "command.pb.h"

WampyServer::~WampyServer() {}

WampyServer::WampyServer(QObject *parent) {}

void WampyServer::newConnection() {
    //    DLOG("new connection\n");
    auto localSocket = server.nextPendingConnection();
    QObject::connect(localSocket, SIGNAL(readyRead()), this, SLOT(read()), Qt::UniqueConnection);
}

void WampyServer::read() {
    QLocalSocket *senderSocket = (QLocalSocket *)sender();
    auto bytes = senderSocket->readAll();
    //    DLOG("received %d bytes\n", bytes.count());
    auto data = handle(bytes);
    respond(senderSocket, data);
}

void WampyServer::respond(QLocalSocket *socket, QByteArray data) {
    auto count = socket->write(data);
    //    DLOG("wrote %lld bytes\n", count);
    socket->flush();
    socket->close();
}

void WampyServer::Serve() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10 * 1000));
    DLOG("serving\n");

    while (!controller.Ready()) {
        if (controller.Initialize() != 0) {
            DLOG("initialize failed\n");
        }
    }

    auto t = controller.provider.thread();
    this->moveToThread(t);

    QFile sockFile(WAMPY_SOCKET);
    if (sockFile.exists()) {
        sockFile.remove();
    }

    server.listen(WAMPY_SOCKET);
    server.moveToThread(controller.provider.thread());
    QObject::connect(&server, SIGNAL(newConnection()), this, SLOT(newConnection()), Qt::QueuedConnection);

    DLOG("listening: %d, %p %p\n", server.isListening(), controller.provider.thread(), server.thread());
}

QByteArray WampyServer::handle(QByteArray data) {
    Command::Command command;
    command.ParseFromArray(data.constData(), data.length());

    if (command.type() != Command::Type::CMD_GET_STATUS) {
        DLOG("incoming command %s (%02d)\n", commandNames[command.type()].c_str(), command.type());
    }

    switch (command.type()) {
    case Command::Type::CMD_FAILURE:
        DLOG("command not specified\n");
        command.set_code(Command::FAIL);
        break;
    case Command::Type::CMD_GET_WINDOW_STATUS:
        controller.GetWindowStatus(&command);
        break;
    case Command::Type::CMD_HIDE_WINDOW:
        controller.Hide(&command);
        break;
    case Command::Type::CMD_SHOW_WINDOW:
        controller.Show(&command);
        break;
    case Command::Type::CMD_GET_STATUS:
        //        controller.GetStatus(&command);
        break;
    case Command::Type::CMD_TEST:
        controller.TestCommand(&command);
        break;
    case Command::Type::CMD_SET_VOLUME:
        controller.SetVolume(&command);
        break;
    case Command::Type::CMD_SEEK:
        controller.Seek(&command);
        break;
    case Command::Type::CMD_TOGGLE_SHUFFLE:
        controller.Shuffle(&command);
        break;
    case Command::Type::CMD_TOGGLE_REPEAT:
        controller.Repeat(&command);
        break;
    case Command::Type::CMD_PREV_TRACK:
        controller.Prev(&command);
        break;
    case Command::Type::CMD_NEXT_TRACK:
        controller.Next(&command);
        break;
    case Command::Type::CMD_PLAY:
        controller.Play(&command);
        break;
    case Command::Type::CMD_STOP:
        controller.Stop(&command);
        break;
    case Command::Type::CMD_PAUSE:
        controller.Pause(&command);
        break;
    case Command::Type::CMD_FEATURE_BIG_COVER:
        controller.FeatureBigCover(&command);
        break;
    case Command::Type::CMD_FEATURE_SHOW_CLOCK:
        controller.FeatureShowClock(&command);
        break;
    case Command::Type::CMD_FEATURE_SET_MAX_VOLUME:
        controller.FeatureSetMaxVolume(&command);
        break;
    case Command::Type::CMD_SET_CLEAR_AUDIO:
        controller.SetClearAudio(&command);
        break;
    case Command::Type::CMD_SET_EQ_BANDS:
        controller.SetEqBands(&command);
        break;
    case Command::Type::CMD_SET_EQ_PRESET:
        controller.SetEqPreset(&command);
        break;
    case Command::Type::CMD_SET_VPT:
        controller.SetVPT(&command);
        break;
    case Command::Type::CMD_SET_VPT_PRESET:
        controller.SetVPTPreset(&command);
        break;
    case Command::Type::CMD_SET_DSEE:
        controller.SetDsee(&command);
        break;
    case Command::Type::CMD_SET_DCPHASE:
        controller.SetDCPhase(&command);
        break;
    case Command::Type::CMD_SET_DCPHASE_PRESET:
        controller.SetDCPhasePreset(&command);
        break;
    case Command::Type::CMD_SET_VINYL:
        controller.SetVinyl(&command);
        break;
    case Command::Type::CMD_SET_DIRECT_SOURCE:
        controller.SetDirectSource(&command);
        break;
    case Command::Type::CMD_SET_TONE_CONTROL_OR_EQ:
        controller.SetToneControlOrEq(&command);
        break;
    case Command::Type::CMD_SET_TONE_CONTROL_VALUES:
        controller.SetToneControlValues(&command);
        break;
    case Command::Type::CMD_SET_DSEE_CUST:
        controller.SetDseeCust(&command);
        break;
    case Command::Type::CMD_SET_DSEE_CUST_MODE:
        controller.SetDseeCustMode(&command);
        break;
    case Command::Type::CMD_SET_VINYL_MODE:
        controller.SetVinylMode(&command);
        break;
    case Command::Type::CMD_UNKNOWN:
    default:
        DLOG("unknown command\n");
        command.set_code(Command::UNKNOWN);
        break;
    }

    auto res = command.SerializeAsString();
    //    dumpBytes(res);

    return QByteArray(res.c_str(), res.size());
}