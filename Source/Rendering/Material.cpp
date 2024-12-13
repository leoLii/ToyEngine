#include "Material.hpp"

Material::Material(std::string name, MaterialConfig& config)
{
	id = GenerateUUID();
	this->name = name;
	this->config = config;
}

Material::~Material()
{
	for (auto pass : passes) {
		delete pass;
	}
	passes.clear();
}

void Material::setShader(std::array<std::string, 2> shaders)
{
	this->shaders = shaders;
}

void Material::setProperty(std::string name, Property property)
{
	properties.insert(std::make_pair(name, property));
}

void Material::addPass(uint32_t, GraphicsPass* pass)
{
	passes.push_back(pass);
}
