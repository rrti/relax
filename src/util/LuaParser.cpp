#include <cassert>
#include <iostream>

#include <boost/foreach.hpp>
#include <lua5.1/lua.hpp>

#include "LuaParser.hpp"

bool LuaTable::operator == (const LuaTable& t) const {
	return (
		TblTblPairs  == t.TblTblPairs &&
		TblStrPairs  == t.TblStrPairs &&
		TblFltPairs  == t.TblFltPairs &&
		TblBoolPairs == t.TblBoolPairs &&
		StrTblPairs  == t.StrTblPairs &&
		StrStrPairs  == t.StrStrPairs &&
		StrFltPairs  == t.StrFltPairs &&
		StrBoolPairs == t.StrBoolPairs &&
		IntTblPairs  == t.IntTblPairs &&
		IntStrPairs  == t.IntStrPairs &&
		IntFltPairs  == t.IntFltPairs &&
		IntBoolPairs == t.IntBoolPairs
	);
}

LuaTable::~LuaTable() {
	for (std::map<LuaTable*, LuaTable*>::iterator it = TblTblPairs.begin(); it != TblTblPairs.end(); it++) {
		delete it->first;
		delete it->second;
	}
	for (std::map<LuaTable*, std::string>::iterator it = TblStrPairs.begin(); it != TblStrPairs.end(); it++) {
		delete it->first;
	}
	for (std::map<LuaTable*, float>::iterator it = TblFltPairs.begin(); it != TblFltPairs.end(); it++) {
		delete it->first;
	}
	for (std::map<LuaTable*, bool>::iterator it = TblBoolPairs.begin(); it != TblBoolPairs.end(); it++) {
		delete it->first;
	}
	for (std::map<std::string, LuaTable*>::iterator it = StrTblPairs.begin(); it != StrTblPairs.end(); it++) {
		delete it->second;
	}
	for (std::map<int, LuaTable*>::iterator it = IntTblPairs.begin(); it != IntTblPairs.end(); it++) {
		delete it->second;
	}
}

void LuaTable::Print(int depth) const {
	std::string tabs = "";
	for (int i = 0; i < depth; i++) {
		tabs += "\t";
	}

	for (std::map<LuaTable*, LuaTable*>::const_iterator it = TblTblPairs.begin(); it != TblTblPairs.end(); it++) {
		std::cout << tabs << "k<tbl>: ";
		std::cout << std::endl;
			it->first->Print(depth + 1);
		std::cout << tabs << "v<tbl>: ";
		std::cout << std::endl;
			it->second->Print(depth + 1);
	}
	for (std::map<LuaTable*, std::string>::const_iterator it = TblStrPairs.begin(); it != TblStrPairs.end(); it++) {
		std::cout << tabs << "k<tbl>: ";
		std::cout << std::endl;
			it->first->Print(depth + 1);
		std::cout << tabs << "v<str>: " << it->second;
		std::cout << std::endl;
	}
	for (std::map<LuaTable*, float>::const_iterator it = TblFltPairs.begin(); it != TblFltPairs.end(); it++) {
		std::cout << tabs << "k<tbl>: ";
		std::cout << std::endl;
			it->first->Print(depth + 1);
		std::cout << tabs << "v<float>: " << it->second;
		std::cout << std::endl;
	}
	for (std::map<LuaTable*, bool>::const_iterator it = TblBoolPairs.begin(); it != TblBoolPairs.end(); it++) {
		std::cout << tabs << "k<tbl>: ";
		std::cout << std::endl;
			it->first->Print(depth + 1);
		std::cout << tabs << "v<bool>: " << it->second;
		std::cout << std::endl;
	}

	for (std::map<std::string, LuaTable*>::const_iterator it = StrTblPairs.begin(); it != StrTblPairs.end(); it++) {
		std::cout << tabs << "k<str>: " << it->first;
		std::cout << ", v<tbl>: ";
		std::cout << std::endl;
			it->second->Print(depth + 1);
	}
	for (std::map<std::string, std::string>::const_iterator it = StrStrPairs.begin(); it != StrStrPairs.end(); it++) {
		std::cout << tabs << "k<str>: " << it->first;
		std::cout << ", v<str>: " << it->second;
		std::cout << std::endl;
	}
	for (std::map<std::string, float>::const_iterator it = StrFltPairs.begin(); it != StrFltPairs.end(); it++) {
		std::cout << tabs << "k<str>: " << it->first;
		std::cout << ", v<float>: " << it->second;
		std::cout << std::endl;
	}
	for (std::map<std::string, bool>::const_iterator it = StrBoolPairs.begin(); it != StrBoolPairs.end(); it++) {
		std::cout << tabs << "k<str>: " << it->first;
		std::cout << ", v<bool>: " << it->second;
		std::cout << std::endl;
	}

	for (std::map<int, LuaTable*>::const_iterator it = IntTblPairs.begin(); it != IntTblPairs.end(); it++) {
		std::cout << tabs << "k<int>: " << it->first;
		std::cout << ", v<tbl>: ";
		std::cout << std::endl;
			it->second->Print(depth + 1);
	}
	for (std::map<int, std::string>::const_iterator it = IntStrPairs.begin(); it != IntStrPairs.end(); it++) {
		std::cout << tabs << "k<int>: " << it->first;
		std::cout << ", v<str>: " << it->second;
		std::cout << std::endl;
	}
	for (std::map<int, float>::const_iterator it = IntFltPairs.begin(); it != IntFltPairs.end(); it++) {
		std::cout << tabs << "k<int>: " << it->first;
		std::cout << ", v<float>: " << it->second;
		std::cout << std::endl;
	}
	for (std::map<int, bool>::const_iterator it = IntBoolPairs.begin(); it != IntBoolPairs.end(); it++) {
		std::cout << tabs << "k<int>: " << it->first;
		std::cout << ", v<bool>: " << it->second;
		std::cout << std::endl;
	}
}

void LuaTable::Parse(lua_State* luaState, int depth) {
	assert(lua_istable(luaState, -1));
	lua_pushnil(luaState);
	assert(lua_istable(luaState, -2));


	while (lua_next(luaState, -2) != 0) {
		assert(lua_istable(luaState, -3));

		switch (lua_type(luaState, -2)) {
			case LUA_TTABLE: {
				LuaTable* key = new LuaTable();

				switch (lua_type(luaState, -1)) {
					case LUA_TTABLE: {
						TblTblPairs[key] = new LuaTable();
						TblTblPairs[key]->Parse(luaState, depth + 1);

						lua_pop(luaState, 1);

						key->Parse(luaState, depth + 1);
					} break;
					case LUA_TSTRING: {
						TblStrPairs[key] = lua_tostring(luaState, -1);
						lua_pop(luaState, 1);

						key->Parse(luaState, depth + 1);
					} break;
					case LUA_TNUMBER: {
						TblFltPairs[key] = lua_tonumber(luaState, -1);
						lua_pop(luaState, 1);

						key->Parse(luaState, depth + 1);
					} break;
					case LUA_TBOOLEAN: {
						TblBoolPairs[key] = lua_toboolean(luaState, -1);

						lua_pop(luaState, 1);

						key->Parse(luaState, depth + 1);
					} break;
				}

				continue;
			} break;

			case LUA_TSTRING: {
				const std::string key = lua_tostring(luaState, -2);

				switch (lua_type(luaState, -1)) {
					case LUA_TTABLE: {
						StrTblPairs[key] = new LuaTable();
						StrTblPairs[key]->Parse(luaState, depth + 1);
					} break;
					case LUA_TSTRING: {
						StrStrPairs[key] = lua_tostring(luaState, -1);
					} break;
					case LUA_TNUMBER: {
						StrFltPairs[key] = lua_tonumber(luaState, -1);
					} break;
					case LUA_TBOOLEAN: {
						StrBoolPairs[key] = lua_toboolean(luaState, -1);
					} break;
				}

				lua_pop(luaState, 1);
				continue;
			} break;

			case LUA_TNUMBER: {
				const int key = lua_tointeger(luaState, -2);

				switch (lua_type(luaState, -1)) {
					case LUA_TTABLE: {
						IntTblPairs[key] = new LuaTable();
						IntTblPairs[key]->Parse(luaState, depth + 1);
					} break;
					case LUA_TSTRING: {
						IntStrPairs[key] = lua_tostring(luaState, -1);
					} break;
					case LUA_TNUMBER: {
						IntFltPairs[key] = lua_tonumber(luaState, -1);
					} break;
					case LUA_TBOOLEAN: {
						IntBoolPairs[key] = lua_toboolean(luaState, -1);
					} break;
				}

				lua_pop(luaState, 1);
				continue;
			} break;
		}
	}

	assert(lua_istable(luaState, -1));
}



void LuaTable::GetTblTblKeys( std::list<LuaTable*>*   keys) const { BOOST_FOREACH(TblTblPair p, TblTblPairs) { keys->push_back(p.first); } }
void LuaTable::GetTblStrKeys( std::list<LuaTable*>*   keys) const { BOOST_FOREACH(TblStrPair p, TblStrPairs) { keys->push_back(p.first); } }
void LuaTable::GetTblFltKeys( std::list<LuaTable*>*   keys) const { BOOST_FOREACH(TblFltPair p, TblFltPairs) { keys->push_back(p.first); } }
void LuaTable::GetTblBoolKeys(std::list<LuaTable*>*   keys) const { BOOST_FOREACH(TblBoolPair p, TblFltPairs) { keys->push_back(p.first); } }
void LuaTable::GetStrTblKeys( std::list<std::string>* keys) const { BOOST_FOREACH(StrTblPair p, StrTblPairs) { keys->push_back(p.first); } }
void LuaTable::GetStrStrKeys( std::list<std::string>* keys) const { BOOST_FOREACH(StrStrPair p, StrStrPairs) { keys->push_back(p.first); } }
void LuaTable::GetStrFltKeys( std::list<std::string>* keys) const { BOOST_FOREACH(StrFltPair p, StrFltPairs) { keys->push_back(p.first); } }
void LuaTable::GetStrBoolKeys(std::list<std::string>* keys) const { BOOST_FOREACH(StrBoolPair p, StrBoolPairs) { keys->push_back(p.first); } }
void LuaTable::GetIntTblKeys(std::list<int>*          keys) const { BOOST_FOREACH(IntTblPair p, IntTblPairs) { keys->push_back(p.first); } }
void LuaTable::GetIntStrKeys(std::list<int>*          keys) const { BOOST_FOREACH(IntStrPair p, IntStrPairs) { keys->push_back(p.first); } }
void LuaTable::GetIntFltKeys(std::list<int>*          keys) const { BOOST_FOREACH(IntFltPair p, IntFltPairs) { keys->push_back(p.first); } }
void LuaTable::GetIntBoolKeys(std::list<int>*         keys) const { BOOST_FOREACH(IntBoolPair p, IntBoolPairs) { keys->push_back(p.first); } }



const LuaTable* LuaTable::GetTblVal(LuaTable* key, LuaTable* defVal) const {
	const std::map<LuaTable*, LuaTable*>::const_iterator it = TblTblPairs.find(key);
	return ((it != TblTblPairs.end())? it->second: defVal);
}
const LuaTable* LuaTable::GetTblVal(const std::string& key, LuaTable* defVal) const {
	const std::map<std::string, LuaTable*>::const_iterator it = StrTblPairs.find(key);
	return ((it != StrTblPairs.end())? it->second: defVal);
}
const LuaTable* LuaTable::GetTblVal(int key, LuaTable* defVal) const {
	const std::map<int, LuaTable*>::const_iterator it = IntTblPairs.find(key);
	return ((it != IntTblPairs.end())? it->second: defVal);
}

const std::string& LuaTable::GetStrVal(LuaTable* key, const std::string& defVal) const {
	const std::map<LuaTable*, std::string>::const_iterator it = TblStrPairs.find(key);
	return ((it != TblStrPairs.end())? it->second: defVal);
}
const std::string& LuaTable::GetStrVal(const std::string& key, const std::string& defVal) const {
	const std::map<std::string, std::string>::const_iterator it = StrStrPairs.find(key);
	return ((it != StrStrPairs.end())? it->second: defVal);
}
const std::string& LuaTable::GetStrVal(int key, const std::string& defVal) const {
	const std::map<int, std::string>::const_iterator it = IntStrPairs.find(key);
	return ((it != IntStrPairs.end())? it->second: defVal);
}

float LuaTable::GetFltVal(LuaTable* key, float defVal) const {
	const std::map<LuaTable*, float>::const_iterator it = TblFltPairs.find(key);
	return ((it != TblFltPairs.end())? it->second: defVal);
}
float LuaTable::GetFltVal(const std::string& key, float defVal) const {
	const std::map<std::string, float>::const_iterator it = StrFltPairs.find(key);
	return ((it != StrFltPairs.end())? it->second: defVal);
}
float LuaTable::GetFltVal(int key, float defVal) const {
	const std::map<int, float>::const_iterator it = IntFltPairs.find(key);
	return ((it != IntFltPairs.end())? it->second: defVal);
}

bool LuaTable::GetBoolVal(LuaTable* key, bool defVal) const {
	const std::map<LuaTable*, bool>::const_iterator it = TblBoolPairs.find(key);
	return ((it != TblBoolPairs.end())? it->second: defVal);
}
bool LuaTable::GetBoolVal(const std::string& key, bool defVal) const {
	const std::map<std::string, bool>::const_iterator it = StrBoolPairs.find(key);
	return ((it != StrBoolPairs.end())? it->second: defVal);
}
bool LuaTable::GetBoolVal(int key, bool defVal) const {
	const std::map<int, bool>::const_iterator it = IntBoolPairs.find(key);
	return ((it != IntBoolPairs.end())? it->second: defVal);
}



LuaParser::~LuaParser() {
	for (std::map<std::string, LuaTable*>::iterator it = tables.begin(); it != tables.end(); it++) {
		delete it->second;
	}

	luaState = NULL;
}

bool LuaParser::Execute(const std::string& luaChunk, bool isFile) {
	bool ret = false;

	if (tables.find(luaChunk) == tables.end()) {
		int (*loadFunc) (lua_State*, const char*) = isFile? luaL_loadfile: luaL_loadstring;
		int (*callFunc) (lua_State*, int, int, int) = lua_pcall;

		int loadErr = 0;
		int callErr = 0;

		static const int nArgs = 0;
		static const int nRets = 1;

		if ((loadErr = loadFunc(luaState, luaChunk.c_str())) != 0 || (callErr = callFunc(luaState, nArgs, nRets, 0)) != 0) {
			errors[luaChunk] = "[LuaParser::Execute] " + std::string(lua_tostring(luaState, -1));
			error = errors[luaChunk];
			lua_pop(luaState, 1);
			return false;
		}

		// expect each chunk to return a single table
		assert(lua_gettop(luaState) == nRets);

		if (lua_istable(luaState, -1) == 1) {
			tables[luaChunk] = new LuaTable();
			tables[luaChunk]->Parse(luaState, 0);
			root = tables[luaChunk];
			ret = true;

			errors[luaChunk] = "[LuaParser::Execute] no error";
			error = errors[luaChunk];
		} else {
			errors[luaChunk] = "[LuaParser::Execute] no table returned in chunk \'" + luaChunk + "\'";
			error = errors[luaChunk];
			root = NULL;
		}

		lua_pop(luaState, 1);
		assert(lua_gettop(luaState) == 0);
	} else {
		ret = true;
	}

	return ret;
}

const LuaTable* LuaParser::GetRootTbl(const std::string& luaChunk) const {
	if (luaChunk.empty()) {
		return root;
	}

	std::map<std::string, LuaTable*>::const_iterator it = tables.find(luaChunk);

	if (it != tables.end()) {
		return it->second;
	}

	return NULL;
}

const std::string& LuaParser::GetError(const std::string& luaChunk) const {
	if (luaChunk.empty()) {
		return error;
	}

	std::map<std::string, std::string>::const_iterator it = errors.find(luaChunk);

	if (it != errors.end()) {
		return it->second;
	}

	const static std::string s = "[" + luaChunk + "] not yet parsed";
	return s;
}
