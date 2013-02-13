mine = {
	desc = "Mine the quarry",
	stage = 0,
	building = nil,
	delay = 2,
}

function mine.create(r)
	local self = {
		desc = "Mine the quarry",
		stage = 0,
		building = nil,
		delay = 2,
	}

	self.building = buildings.find_closest(r.p)
	self.building_center = point.new(
		self.building.p.x + self.building.model.width / 2,
		self.building.p.y + self.building.model.height / 2
	)
	print(self.building)
	print(self.building_center)
	return self
end

function mine:act(robot, frameduration)
	if self.stage == 0 then
		point_moveto(robot.p, building_center, robot.speed * frameduration)
		if point_equals(robot.p, building_center) then
			self.stage = self.stage + 1
		end
		return 1
	elseif stage == 1 then
		self.delay = self.delay - frameduration;
		if self.delay <= 0 then
			self.stage = self.stage + 1
		end
		return 1
	elseif stage == 2 then
		map:create_item(center.x, center.y + 3, ITEM_ROCK)
		return 0
	end
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


