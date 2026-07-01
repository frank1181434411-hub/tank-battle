#pragma once

#include"ai/tank_command.hpp"

class EnemyTank;
struct AIContext;

class AIController
{
public:
    virtual ~AIController()=default;
    virtual TankCommand decide(const EnemyTank& self,const AIContext& context,float deltaTime)=0;
};
