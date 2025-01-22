#pragma once
#include"Framework.h"

class Order : public Framework
{
public:
	
	void Initialize()override;
	void Update()override;
	void Finalize()override;
	void Draw()override;

};

