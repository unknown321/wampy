#include "../../src/resume_policy.hpp"
#include <cassert>
#include <random>
#include <iostream>
using namespace wampy_review;
int main(){ResumePolicy p;std::mt19937 rng(55);long now=0;bool known=false,on=false,wanted=false,exported=false,graphics=true,ready=false,quiet=false;Visibility vis=Visibility::Unknown;std::uint64_t proof=0;unsigned draws=0;
 for(int i=0;i<100000;++i){now+=rng()%100;auto before=p.generation();switch(rng()%7){
 case 0:known=rng()%2;on=rng()%2;p.observeDisplay(known,on,now);break;
 case 1:wanted=rng()%2;p.preferWampy(wanted,now);break;
 case 2:exported=rng()%2;p.setStorageExported(exported,now);break;
 case 3:graphics=rng()%2;p.setGraphicsHealthy(graphics);break;
 case 4:p.powerReleased();break;
 case 5:vis=rng()%2?Visibility::Hidden:Visibility::Visible;ready=rng()%2;quiet=ready&&vis==Visibility::Hidden&&rng()%2;proof=p.generation();p.observeNative(proof,vis,ready,quiet);break;
 default:if(p.generation()>0)p.observeNative(p.generation()-1,Visibility::Hidden,true,true);break;}
 if(p.generation()!=before)proof=0;
 auto d=p.step(now);assert(p.hideAttempts()<=3);
 if(!known||!on)assert(d.action==Action::None&&!d.render);
 if(d.render){++draws;assert(known&&on&&wanted&&!exported&&graphics&&ready&&quiet&&vis==Visibility::Hidden&&proof==d.generation);}
 }
 assert(draws>0);std::cout<<"PASS: 100000 randomized lifecycle transitions preserve display, intent, generation, GPU proof and retry bounds; draws="<<draws<<"\n";
}
