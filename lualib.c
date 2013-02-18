#include "lualib.h"

const char *LUA_MT_ROBOT = "robot";
const char *LUA_MT_POINT = "point";
const char *LUA_MT_BUILDING = "building";
const char *LUA_MT_BUILDING_MODEL = "building_model";

lua_State *L;

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

robot* lh_push_robot(lua_State *L, robot *src)
{
	robot *r = (robot *)lua_newuserdata(L, sizeof(robot));
	luaL_getmetatable(L, LUA_MT_ROBOT);
	lua_setmetatable(L, -2);
	memcpy(r, src, sizeof(robot));
	return r;
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

int l_robot_moveto(lua_State *L)
{
	robot *r = (robot*) luaL_checkudata(L, 1, LUA_MT_ROBOT);
	point *dest = (point*) luaL_checkudata(L, 2, LUA_MT_POINT);
	float frameduration = luaL_checknumber(L, 3);
	point_moveto(&r->p, dest, r->speed * frameduration);
	return 0;
}

int l_robot__index(lua_State *L)
{
	robot *r = (robot*) luaL_checkudata(L, 1, LUA_MT_ROBOT);
	const char* prop = lua_tostring(L, 2);
	if (strcmp(prop, "p") == 0) {
		lh_push_point(L, &r->p);
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

