#include "GetSet.h"

GetSet::GetSet(string world_name): world_name(world_name){}

GetSet::~GetSet(){}

WrtGet GetSet::Get(string subject_name){
    this->subject_name = subject_name;
    return WrtGet(this->world_name, this->subject_name);
}

WrtSet GetSet::Set(string subject_name){
    if(subject_name == "world")
        throw runtime_error("Cannot change the 'world' reference frame as it's assumed to be an inertial/immobile frame.");
    this->subject_name = subject_name;
    return WrtSet(this->world_name, this->subject_name);
}