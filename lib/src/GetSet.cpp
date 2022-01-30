#include "GetSet.h"

GetSet::GetSet(string world_name): world_name(world_name){}

GetSet::~GetSet(){}

WrtGet GetSet::Get(string frame_name){
    this->frame_name = frame_name;
    return WrtGet(this->world_name, this->frame_name);
}

WrtSet GetSet::Set(string frame_name){
    if(frame_name == "world")
        throw runtime_error("Cannot change the 'world' reference frame as it's assumed to be an inertial/immobile frame.");
    this->frame_name = frame_name;
    return WrtSet(this->world_name, this->frame_name);
}