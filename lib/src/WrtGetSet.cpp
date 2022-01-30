#include "WrtGetSet.h"

WrtGet::WrtGet(string world_name, string frame_name): world_name(world_name), frame_name(frame_name){

}

WrtGet::~WrtGet(){}

ExpressedInGet WrtGet::Wrt(string ref_frame_name){
    this->ref_frame_name = ref_frame_name;
    return ExpressedInGet(this->world_name, this->frame_name, this->ref_frame_name);
}


WrtSet::WrtSet(string world_name, string frame_name): world_name(world_name), frame_name(frame_name){
    
}

WrtSet::~WrtSet(){}

ExpressedInSet WrtSet::Wrt(string ref_frame_name){
    this->ref_frame_name = ref_frame_name;
    return ExpressedInSet(this->world_name, this->frame_name, this->ref_frame_name);
}