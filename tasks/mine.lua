ITEM_NONE		= 0
ITEM_BUCKET		= 1
ITEM_WELL		= 2
ITEM_SCRAP		= 3
ITEM_TREE		= 4
ITEM_ROCK		= 5

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
	local building = buildings.find_closest(robot.p)
	local building_center = point.new(
		math.floor(building.p.x + building.model.width / 2),
		math.floor(building.p.y + building.model.height / 2)
	)
	local stage = 0
	local path = map.compute_path(robot.p, building_center)

	return {
		desc = "Mine the quarry",
		act = function (frameduration)
			if stage == 0 then
				if robot.p == building_center then
					stage = stage + 1
				else
					robot:walk_along(path, frameduration)
				end
				return 1
			elseif stage == 1 then
				delay = delay - frameduration;
				if delay <= 0 then
					stage = stage + 1
				end
				return 1
			elseif stage == 2 then
				item_loc = point.new(building_center.x, building.p.y + building.model.height)
				map.create_item(item_loc, ITEM_ROCK)
				return 0
			end
		end
	}
end

mine_task = {
	create = mine.create,
}

tasks = {
	mine = mine_task
}


