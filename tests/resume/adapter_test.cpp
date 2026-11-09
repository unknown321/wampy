#include "../../src/resume_policy.hpp"
#include "../../src/bounded_socket.hpp"
#include "command.pb.h"
#include <cassert>
#include <iostream>
#define DLOG(...) ((void)0)
static Command::Command reply;
namespace Hagoromo {struct HagoromoConnector {static bool sendCMD(Command::Command* c){*c=reply;return true;}};}
int main(){wampy_review::ResumePolicy resume;resume.observeDisplay(true,true,0);resume.preferWampy(true,0);
    auto observeNative=[&](bool hide,std::uint64_t generation){
        Command::Command command;
        command.set_type(hide?Command::CMD_HIDE_WINDOW:Command::CMD_GET_WINDOW_STATUS);
        if(hide){auto request=command.mutable_windowstatus();request->set_visible(Command::VISIBILITY_UNKNOWN);request->set_handoff_generation(generation);request->set_deadline_monotonic_ms(wampy_resume::monotonicMs()+200);}
        bool ok=Hagoromo::HagoromoConnector::sendCMD(&command)&&command.code()==Command::OK;
        const auto& status=command.windowstatus();auto visible=wampy_review::Visibility::Unknown;
        if(ok&&status.visible()==Command::VISIBILITY_YES)visible=wampy_review::Visibility::Visible;
        if(ok&&status.visible()==Command::VISIBILITY_NO)visible=wampy_review::Visibility::Hidden;
        bool protocol=ok&&status.has_handoff_protocol()&&status.handoff_protocol()==1;
        bool drained=protocol&&status.handoff_generation()==generation&&status.render_quiescent();
        resume.observeNative(generation,visible,protocol,drained);
        if(!protocol)DLOG("resume: missing compatible handoff acknowledgment\n");
    };
reply.set_type(Command::CMD_GET_WINDOW_STATUS);reply.set_code(Command::OK);auto status=reply.mutable_windowstatus();status->set_visible(Command::VISIBILITY_NO);
observeNative(false,resume.generation());assert(!resume.step(0).render);
status->set_handoff_protocol(1);status->set_handoff_generation(resume.generation()-1);status->set_render_quiescent(true);observeNative(false,resume.generation());assert(!resume.step(100).render);
status->set_handoff_generation(resume.generation());status->set_render_quiescent(false);observeNative(false,resume.generation());assert(!resume.step(200).render);
status->set_render_quiescent(true);observeNative(false,resume.generation());assert(resume.step(300).render);
reply.set_code(Command::FAIL);observeNative(false,resume.generation());assert(!resume.step(400).render);
std::cout<<"PASS: production native adapter rejects old protocol/stale generation/no drain/failure, accepts matching acknowledgment\n";}
