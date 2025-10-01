#pragma once
class StageScene; 
#include "IStageState.h"

class GameOverState : public IStageState
{
public:
    void Enter(StageScene* scene) override;
    void Update(StageScene* scene) override;
    void Exit(StageScene* scene) override;
};
