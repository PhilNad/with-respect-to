#include "WrtGetSet.h"

WrtGet::WrtGet(string world_name, string subject_name): world_name(world_name), subject_name(subject_name){

}

WrtGet::~WrtGet(){}

ExpressedInGet WrtGet::Wrt(string basis_name){
    this->basis_name = basis_name;
    return ExpressedInGet(this->world_name, this->subject_name, this->basis_name);
}


WrtSet::WrtSet(string world_name, string subject_name): world_name(world_name), subject_name(subject_name){
    
}

WrtSet::~WrtSet(){}

ExpressedInSet WrtSet::Wrt(string basis_name){
    this->basis_name = basis_name;
    if(this->subject_name == this->basis_name)
        throw runtime_error("The reference frame "+this->basis_name+" must be different than the target frame "+this->subject_name+".");
    return ExpressedInSet(this->world_name, this->subject_name, this->basis_name);
}