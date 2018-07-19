#pragma once
#include <iostream>
#include <vector>

#include "tao/json.hpp"
#include <opencv2/opencv.hpp>
#include "yolo_v2_class.hpp"


class BoxSerializer
{
public:
	static std::vector<bbox_t> fromString(const std::string& str);
	static std::string toString(const std::vector<bbox_t>& boxes);

private:
	static bbox_t deserializeBox(const tao::json::value& obj);
	static std::vector<bbox_t> deserializeBoxes(const tao::json::value& arr);
	static tao::json::value serialize(bbox_t box);
	static tao::json::value serialize(const std::vector<bbox_t>& boxes);
};