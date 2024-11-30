#include "command_names.h"
#include "command.pb.h"
#include <string>

std::map<int, std::string> commandNames = {
    {Command::Type::CMD_FAILURE, "CMD_FAILURE"},
    {Command::Type::CMD_GET_WINDOW_STATUS, "CMD_GET_WINDOW_STATUS"},
    {Command::Type::CMD_HIDE_WINDOW, "CMD_HIDE_WINDOW"},
    {Command::Type::CMD_SHOW_WINDOW, "CMD_SHOW_WINDOW"},
    {Command::Type::CMD_GET_STATUS, "CMD_GET_STATUS"},
    {Command::Type::CMD_TEST, "CMD_TEST"},
    {Command::Type::CMD_SET_VOLUME, "CMD_SET_VOLUME"},
    {Command::Type::CMD_SEEK, "CMD_SEEK"},
    {Command::Type::CMD_TOGGLE_SHUFFLE, "CMD_TOGGLE_SHUFFLE"},
    {Command::Type::CMD_TOGGLE_REPEAT, "CMD_TOGGLE_REPEAT"},
    {Command::Type::CMD_NEXT_TRACK, "CMD_NEXT_TRACK"},
    {Command::Type::CMD_PREV_TRACK, "CMD_PREV_TRACK"},
    {Command::Type::CMD_PLAY, "CMD_PLAY"},
    {Command::Type::CMD_PAUSE, "CMD_PAUSE"},
    {Command::Type::CMD_STOP, "CMD_STOP"},
    {Command::Type::CMD_FEATURE_BIG_COVER, "CMD_FEATURE_BIG_COVER"},
    {Command::Type::CMD_FEATURE_SHOW_CLOCK, "CMD_FEATURE_SHOW_CLOCK"},
};