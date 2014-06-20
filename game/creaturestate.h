#ifndef CREATURESTATE_H_
#define CREATURESTATE_H_

#include <map>
#include <string>

#include <game/direction.h>

namespace castle
{
    namespace world
    {
        class Creature;
        class SimulationContext;
    }
    
    namespace world
    {
        class CreatureState
        {
        public:
            virtual double GetX() const = 0;
            virtual double GetY() const = 0;
            virtual double GetZ() const = 0;

            virtual const std::string GetSpriteName() const = 0;
            virtual int GetOpacity() const = 0;
        
            virtual void Update(const SimulationContext &context, Creature &creature) = 0;
        };

        struct Peasant
        {
            float posX;
            float posY;
            core::Direction dir;

            enum class PeasantState : int {
                
            };

            PeasantState peasantState;
        };
        
        class DefaultCreatureState : public CreatureState
        {
            double mX;
            double mY;
            
        public:
            explicit DefaultCreatureState(double x, double y);

            virtual double GetX() const;
            virtual double GetY() const;

            virtual const std::string GetSpriteName() const;
            virtual int GetOpacity() const;

            virtual void Update(const SimulationContext &context, Creature &creature);
        };
    }
}

#endif // CREATURESTATE_H_
