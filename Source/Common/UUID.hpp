#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <string>

using uuid = boost::uuids::uuid;

uuid GenerateUUID() {
	boost::uuids::random_generator generator;
	boost::uuids::uuid uuid = generator();
	return uuid;
}
