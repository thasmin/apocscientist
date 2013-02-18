mine = {
	desc = "Mine the quarry",
	stage = 0,
	building = nil,
	delay = 2,
}

function mine_act(self, robot, frameduration)
end

function mine.create(robot)
	local delay = 2
	local stage = 0
	local building = buildings.find_closest(robot.p)
	local building_center = point.new(
		building.p.x + building.model.width / 2,
		building.p.y + building.model.height / 2
	)

	return {
		desc = "Mine the quarry",
		act = function (robot, frameduration)
			if stage == 0 then
				robot:moveto(building_center, frameduration)
				if point_equals(robot.p, building_center) then
					stage = stage + 1
				end
				return 1
			elseif stage == 1 then
				delay = delay - frameduration;
				if delay <= 0 then
					stage = stage + 1
				end
				return 1
			elseif stage == 2 then
				map:create_item(center.x, center.y + 3, ITEM_ROCK)
				return 0
			end
		end
	}
end

function tmp(i)
	return i * 10
end

mine_task = {
	--create = tmp, --mine.create,
	create = mine.create,
	act = mine.act
}

tasks = { mine_task }


