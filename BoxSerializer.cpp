#include "BoxSerializer.h"


std::vector<bbox_t> BoxSerializer::fromString(const std::string& str)
{
	auto val = tao::json::from_string(str);
	return BoxSerializer::deserializeBoxes(val);
}

std::string BoxSerializer::toString(const std::vector<bbox_t>& boxes)
{
	auto val = BoxSerializer::serialize(boxes);
	return tao::json::to_string(val);
}

bbox_t BoxSerializer::deserializeBox(const tao::json::value& obj)
{
	if (!obj.is_object())
	{
		throw std::runtime_error("JSON value is not an object.");
	}

	bbox_t box;
	try
	{
		box.x = obj.at("x").get_unsigned();
		box.y = obj.at("y").get_unsigned();
		box.w = obj.at("w").get_unsigned();
		box.h = obj.at("h").get_unsigned();
		box.obj_id = obj.at("obj_id").get_unsigned();
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		throw;
	}
	return box;
}

std::vector<bbox_t> BoxSerializer::deserializeBoxes(const tao::json::value& arr)
{
	if (!arr.is_array())
	{
		throw std::runtime_error("JSON object is not an array.");
	}

	std::vector<bbox_t> boxes;
	auto vec = arr.get_array();
	for (auto& val : vec)
	{
		try
		{
			auto box = BoxSerializer::deserializeBox(val);
			boxes.push_back(box);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Failed to deserialize box: " << ex.what() << std::endl;
		}
	}
	return boxes;
}

tao::json::value BoxSerializer::serialize(bbox_t box)
{
	tao::json::value val;
	val.emplace("x", box.x);
	val.emplace("y", box.y);
	val.emplace("w", box.w);
	val.emplace("h", box.h);
	val.emplace("obj_id", box.obj_id);
	return val;
}

tao::json::value BoxSerializer::serialize(const std::vector<bbox_t>& boxes)
{
	tao::json::value arr = tao::json::empty_array;
	for (auto& box : boxes)
	{
		auto obj = BoxSerializer::serialize(box);
		arr.push_back(obj);
	}
	return arr;
}