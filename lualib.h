#ifndef LUALIB_H
#define LUALIB_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
extern lua_State *L;

#include "building.h"
#include "robot.h"
#include "point.h"

const char *LUA_MT_ROBOT;
const char *LUA_MT_POINT;
const char *LUA_MT_BUILDING;
const char *LUA_MT_BUILDING_MODEL;

building* lh_push_building(lua_State *L, building *src);
building_model* lh_push_building_model(lua_State *L, building_model *src);
robot* lh_push_robot(lua_State *L, robot *src);
point* lh_push_point(lua_State *L, point *src);

int l_point_new(lua_State *L);

int l_buildings_find_closest(lua_State *L);

int l_building_model__index(lua_State *L);

int l_building__index(lua_State *L);
int l_building__tostring(lua_State *L);

int l_robot_moveto(lua_State *L);
int l_robot__index(lua_State *L);
int l_robot__tostring(lua_State *L);

int l_point__index(lua_State *L);
int l_point__tostring(lua_State *L);

#endif
