#include "creature.h"

#include <game/simulationcontext.h>
#include <game/creaturestate.h>

namespace Castle
{
    Creature::Creature(int identity, std::unique_ptr<CreatureState> state)
        : mOwner()
        , mIdentity(identity)
        , mState(std::move(state))
    {}
    
    void Creature::Update(const SimulationContext &context)
    {
        mState->Update(context, *this);
    }

    void Creature::SetState(std::unique_ptr<CreatureState> state)
    {
        mState = std::move(state);
    }
}