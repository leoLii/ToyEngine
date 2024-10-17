#pragma once

#include <string>
#include <typeindex>

class Component {
public:
	Component() = default;
	Component(const std::string&);
	Component(Component&& other) = default;
	virtual ~Component() = default;

	const std::string& getName() const;
	virtual std::type_index getType() = 0;

	virtual void update(float deltaTime) = 0;

protected:
	std::string name;
};