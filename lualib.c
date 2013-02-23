#include "lualib.h"

#include "map.h"

const char *LUA_MT_ROBOT = "robot";
const char *LUA_MT_POINT = "point";
const char *LUA_MT_BUILDING = "building";
const char *LUA_MT_BUILDING_MODEL = "building_model";

lua_State *L;

int l_point_new(lua_State *L);

int l_buildings_find_closest(lua_State *L);

int l_map_create_item(lua_State *L);
int l_map_compute_path(lua_State *L);

int l_building_model__index(lua_State *L);

int l_building__index(lua_State *L);
int l_building__tostring(lua_State *L);

int l_robot_walk_along(lua_State *L);
int l_robot__index(lua_State *L);
int l_robot__tostring(lua_State *L);

int l_point__eq(lua_State *L);
int l_point__index(lua_State *L);
int l_point__tostring(lua_State *L);

const struct luaL_reg point_lib[] = {
	{"new", l_point_new},
	{NULL, NULL},
};
const struct luaL_reg buildings_lib[] = {
	{"find_closest", l_buildings_find_closest},
	{NULL, NULL},
};
const struct luaL_reg map_lib[] = {
	{"compute_path", l_map_compute_path},
	{"create_item", l_map_create_item},
	{NULL, NULL},
};

const struct luaL_Reg l_building_model_reg[] = {
	{ "__index", l_building_model__index } ,
	{ NULL, NULL }
};
const struct luaL_Reg l_building_reg[] = {
	{ "__index", l_building__index } ,
	{ "__tostring", l_building__tostring } ,
	{ NULL, NULL }
};
const struct luaL_Reg l_robot_reg[] = {
	{ "__index", l_robot__index } ,
	{ "__tostring", l_robot__tostring } ,
	{ NULL, NULL }
};
const struct luaL_Reg l_point_reg[] = {
	{ "__eq", l_point__eq },
	{ "__index", l_point__index },
	{ "__tostring", l_point__tostring },
	{ NULL, NULL },
};

void lualib_init(lua_State *L)
{
	// create object metatables
	luaL_newmetatable(L, LUA_MT_BUILDING);
	luaL_register(L, NULL, l_building_reg);
	luaL_newmetatable(L, LUA_MT_BUILDING_MODEL);
	luaL_register(L, NULL, l_building_model_reg);
	luaL_newmetatable(L, LUA_MT_ROBOT);
	luaL_register(L, NULL, l_robot_reg);
	luaL_newmetatable(L, LUA_MT_POINT);
	luaL_register(L, NULL, l_point_reg);
	lua_pop(L, 4);

	// create global functions
	luaL_openlib(L, "buildings", buildings_lib, 0);
	luaL_openlib(L, "map", map_lib, 0);
	luaL_openlib(L, "point", point_lib, 0);
	lua_pop(L, 2);
}

building* lh_push_building(lua_State *L, building *src)
{
	building *b = (building *)lua_newuserdata(L, sizeof(building));
	luaL_getmetatable(L, LUA_MT_BUILDING);
	lua_setmetatable(L, -2);
	memcpy(b, src, sizeof(building));
	return b;
}

building_model* lh_push_building_model(lua_State *L, building_model *src)
{
	building_model *b = (building_model *)lua_newuserdata(L, sizeof(building_model));
	luaL_getmetatable(L, LUA_MT_BUILDING_MODEL);
	lua_setmetatable(L, -2);
	memcpy(b, src, sizeof(building_model));
	return b;
}

void lh_push_robot(lua_State *L, robot *src)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, src->lua_ref);
}

point* lh_push_point(lua_State *L, point *src)
{
	point *p = (point *)lua_newuserdata(L, sizeof(point));
	luaL_getmetatable(L, LUA_MT_POINT);
	lua_setmetatable(L, -2);
	if (src != NULL)
		memcpy(p, src, sizeof(point));
	return p;
}

int l_point_new(lua_State *L)
{
	point *p = lh_push_point(L, NULL);
	p->x = luaL_checknumber(L, 1);
	p->y = luaL_checknumber(L, 2);
	return 1;
}

int l_buildings_find_closest(lua_State *L)
{
	point *p = (point*) luaL_checkudata(L, 1, LUA_MT_POINT);
	building *b = building_find_closest(p, BUILDING_QUARRY);
	lh_push_building(L, b);
	return 1;
}

int l_map_compute_path(lua_State *L)
{
	point *start = (point*) luaL_checkudata(L, 1, LUA_MT_POINT);
	point *dest = (point*) luaL_checkudata(L, 2, LUA_MT_POINT);
	TCOD_path_t *path = map_computepath(start, dest);
	lua_pushlightuserdata(L, path);
	return 1;
}

int l_map_create_item(lua_State *L)
{
	point *p = (point*) luaL_checkudata(L, 1, LUA_MT_POINT);
	int type = lua_tointeger(L, 2);
	map_create_item(p->x, p->y, type);
	return 0;
}

int l_building_model__index(lua_State *L)
{
	building_model *m = (building_model *) luaL_checkudata(L, 1, LUA_MT_BUILDING_MODEL);
	const char* prop = lua_tostring(L, 2);
	if (strcmp(prop, "width") == 0) {
		lua_pushinteger(L, m->width);
		return 1;
	} else 	if (strcmp(prop, "height") == 0) {
		lua_pushinteger(L, m->height);
		return 1;
	}

	return 0;
}

int l_building__index(lua_State *L)
{
	building *b = (building*) luaL_checkudata(L, 1, LUA_MT_BUILDING);
	const char* prop = lua_tostring(L, 2);
	if (strcmp(prop, "p") == 0) {
		lh_push_point(L, &b->p);
		return 1;
	} else 	if (strcmp(prop, "model") == 0) {
		lh_push_building_model(L, b->model);
		return 1;
	}

	return 0;
}

int l_building__tostring(lua_State *L)
{
	building *b = (building*) luaL_checkudata(L, 1, LUA_MT_BUILDING);
	lua_pushfstring(L, "building at %f, %f", b->p.x, b->p.y);
	return 1;
}

int l_robot_walk_along(lua_State *L)
{
	if (!lua_islightuserdata(L, 2))
		return 0;
	robot *r = (robot*) luaL_checkudata(L, 1, LUA_MT_ROBOT);
	TCOD_path_t *path = (TCOD_path_t*) lua_topointer(L, 2);
	float frameduration = luaL_checknumber(L, 3);
	map_walk(path, &r->p, r->speed * frameduration);
	return 0;
}

int l_robot__index(lua_State *L)
{
	robot *r = (robot*) luaL_checkudata(L, 1, LUA_MT_ROBOT);
	const char* prop = lua_tostring(L, 2);
	if (strcmp(prop, "p") == 0) {
		lh_push_point(L, &r->p);
		return 1;
	} else if (strcmp(prop, "walk_along") == 0) {
		lua_pushcfunction(L, l_robot_walk_along);
		return 1;
	}
	return 0;
}

int l_robot__tostring(lua_State *L)
{
	robot *r = (robot*) luaL_checkudata(L, 1, LUA_MT_ROBOT);
	lua_pushfstring(L, "robot at %f, %f", r->p.x, r->p.y);
	return 1;
}

int l_point__eq(lua_State *L)
{
	point *p1 = (point*) luaL_checkudata(L, 1, LUA_MT_POINT);
	point *p2 = (point*) luaL_checkudata(L, 2, LUA_MT_POINT);
	lua_pushboolean(L, point_equals(p1, p2));
	return 1;
}

int l_point__index(lua_State *L)
{
	point *p = (point*) luaL_checkudata(L, 1, LUA_MT_POINT);
	const char* prop = lua_tostring(L, 2);
	if (strcmp(prop, "x") == 0) {
		lua_pushnumber(L, p->x);
		return 1;
	} else if (strcmp(prop, "y") == 0) {
		lua_pushnumber(L, p->y);
		return 1;
	}
	return 0;
}

int l_point__tostring(lua_State *L)
{
	point *p = (point*) luaL_checkudata(L, 1, LUA_MT_POINT);
	lua_pushfstring(L, "point at %f, %f", p->x, p->y);
	return 1;
}

