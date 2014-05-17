#ifndef RELAX_LUA_PARSER_HDR
#define RELAX_LUA_PARSER_HDR

#include <list>
#include <map>
#include <string>

#include "../math/vec3fwd.hpp"

struct lua_State;

struct LuaTable {
public:
	bool operator < (const LuaTable& t) const { return (this < &t); }
	bool operator == (const LuaTable&) const;

	~LuaTable();

	void Print(int) const;
	void Parse(lua_State*, int);

	// <table key, {table, string, float, bool} value>
	typedef std::pair<LuaTable*, LuaTable*>     TblTblPair;
	typedef std::pair<LuaTable*, std::string>   TblStrPair;
	typedef std::pair<LuaTable*, float>         TblFltPair;
	typedef std::pair<LuaTable*, bool>          TblBoolPair;
	// <string key, {table, string, float, bool} value>
	typedef std::pair<std::string, LuaTable*>   StrTblPair;
	typedef std::pair<std::string, std::string> StrStrPair;
	typedef std::pair<std::string, float>       StrFltPair;
	typedef std::pair<std::string, bool>        StrBoolPair;
	// <int key, {table, string, float, bool} value>
	typedef std::pair<int, LuaTable*>           IntTblPair;
	typedef std::pair<int, std::string>         IntStrPair;
	typedef std::pair<int, float>               IntFltPair;
	typedef std::pair<int, bool>                IntBoolPair;

	// all <table key, {table, string, float, bool} value> pairs
	void GetTblTblKeys(std::list<LuaTable*>*   ) const;
	void GetTblStrKeys(std::list<LuaTable*>*   ) const;
	void GetTblFltKeys(std::list<LuaTable*>*   ) const;
	void GetTblBoolKeys(std::list<LuaTable*>*  ) const;
	// all <string key, {table, string, float, bool} value> pairs
	void GetStrTblKeys(std::list<std::string>* ) const;
	void GetStrStrKeys(std::list<std::string>* ) const;
	void GetStrFltKeys(std::list<std::string>* ) const;
	void GetStrBoolKeys(std::list<std::string>*) const;
	// all <int key, {table, string, float, bool} value> pairs
	void GetIntTblKeys(std::list<int>*         ) const;
	void GetIntStrKeys(std::list<int>*         ) const;
	void GetIntFltKeys(std::list<int>*         ) const;
	void GetIntBoolKeys(std::list<int>*        ) const;

	// {table, string, int} key ==> table value
	const LuaTable* GetTblVal(LuaTable*, LuaTable* defVal = 0) const;
	const LuaTable* GetTblVal(const std::string&, LuaTable* defVal = 0) const;
	const LuaTable* GetTblVal(int, LuaTable* defVal = 0) const;
	// {table, string, int} key ==> string value
	const std::string& GetStrVal(LuaTable*, const std::string& defVal) const;
	const std::string& GetStrVal(const std::string&, const std::string& defVal) const;
	const std::string& GetStrVal(int, const std::string& defVal) const;
	// {table, string, int} key ==> float value
	float GetFltVal(LuaTable*, float defVal) const;
	float GetFltVal(const std::string&, float defVal) const;
	float GetFltVal(int, float defVal) const;
	// {table, string, int} key ==> bool value
	bool GetBoolVal(LuaTable*, bool defVal) const;
	bool GetBoolVal(const std::string&, bool defVal) const;
	bool GetBoolVal(int, bool defVal) const;

	bool HasStrTblKey(const std::string& key) const { return (StrTblPairs.find(key) != StrTblPairs.end()); }
	bool HasStrStrKey(const std::string& key) const { return (StrStrPairs.find(key) != StrStrPairs.end()); }
	bool HasStrFltKey(const std::string& key) const { return (StrFltPairs.find(key) != StrFltPairs.end()); }
	bool HasStrBoolKey(const std::string& key) const { return (StrBoolPairs.find(key) != StrBoolPairs.end()); }



	template<typename T> void GetArray(const LuaTable* tbl, T* array, unsigned int len) const {
		for (unsigned int i = 0; i < len; i++) {
			array[i] = T(tbl->GetFltVal(i + 1, T(0)));
		}
	}

	template<typename V> V GetVec(int key, unsigned int len) const {
		const std::map<int, LuaTable*>::const_iterator it = IntTblPairs.find(key);

		V v;

		if (it != IntTblPairs.end()) {
			GetArray(it->second, &v.x, len);
		}

		return v;
	}
	template<typename V> V GetVec(const std::string& key, unsigned int len) const {
		const std::map<std::string, LuaTable*>::const_iterator it = StrTblPairs.find(key);

		V v;

		if (it != StrTblPairs.end()) {
			GetArray(it->second, &v.x, len);
		}

		return v;
	}

private:
	std::map<LuaTable*, LuaTable*>     TblTblPairs;
	std::map<LuaTable*, std::string>   TblStrPairs;
	std::map<LuaTable*, float>         TblFltPairs;
	std::map<LuaTable*, bool>          TblBoolPairs;
	std::map<std::string, LuaTable*>   StrTblPairs;
	std::map<std::string, std::string> StrStrPairs;
	std::map<std::string, float>       StrFltPairs;
	std::map<std::string, bool>        StrBoolPairs;
	std::map<int, LuaTable*>           IntTblPairs;
	std::map<int, std::string>         IntStrPairs;
	std::map<int, float>               IntFltPairs;
	std::map<int, bool>                IntBoolPairs;
};




struct LuaParser {
public:
	LuaParser(lua_State* state): luaState(state) {}
	~LuaParser();

	bool Execute(const std::string&, bool isFile);

	const LuaTable* GetRootTbl(const std::string& = "") const;
	const std::string& GetError(const std::string& = "") const;

private:
	lua_State* luaState;

	// root-table of most recently parsed file
	LuaTable* root;
	// error in most recently parsed file
	std::string error;

	std::map<std::string, LuaTable*> tables;
	std::map<std::string, std::string> errors;
};

#endif
