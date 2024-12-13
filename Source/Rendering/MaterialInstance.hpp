#pragma once

#include "Material.hpp"

class MaterialInstance {
public:
	MaterialInstance();
	~MaterialInstance();

protected:
	Material* material;

};