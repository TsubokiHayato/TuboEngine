#pragma once
class DirectXCommon;
class SrvManager
{

public:

	void Initialize(DirectXCommon* dxCommon);


private:

	DirectXCommon* dxCommon = nullptr;
};

