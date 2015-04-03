//-----------------------------------------------------------------------------
// luna2d engine
// Copyright 2014-2015 Stepan Prokofjev
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "lunaassets.h"
#include "lunagraphics.h"
#include "lunasizes.h"
#include "lunatextureatlasloader.h"

using namespace luna2d;

LUNAAssets::LUNAAssets() :
	tblAssets(LUNAEngine::SharedLua())
{
	LuaScript* lua = LUNAEngine::SharedLua();
	LuaTable tblLuna = lua->GetGlobalTable().GetTable("luna");

	// Bind assets table to lua
	tblLuna.SetField("assets", tblAssets);

	// Bind assets manager to lua
	LuaTable tblAssetsMgr(lua);
	tblAssetsMgr.SetField("loadAll", LuaFunction(lua, this, &LUNAAssets::LoadAll));
	tblAssetsMgr.SetField("loadFolder", LuaFunction(lua, this, &LUNAAssets::LoadFolder));
	tblAssetsMgr.SetField("load", LuaFunction(lua, this, &LUNAAssets::Load));
	tblAssetsMgr.SetField("unload", LuaFunction(lua, this, &LUNAAssets::Unload));
	tblAssetsMgr.SetField("unloadFolder", LuaFunction(lua, this, &LUNAAssets::UnloadFolder));
	tblAssetsMgr.SetField("unloadAll", LuaFunction(lua, this, &LUNAAssets::UnloadAll));
	tblLuna.SetField("assetsmgr", tblAssetsMgr);

	// Bind base asset type
	LuaClass<LUNAAsset> clsAsset(lua);
}

LUNAAssets::~LUNAAssets()
{
	UnloadAll();
}

// Get parent table for given asset path
// Returns nil if path not found
// if "autoMake" is true, automatically creates non-existent tables
LuaTable LUNAAssets::GetParentTableForPath(const std::string& path, bool autoMake)
{
	if(path.empty()) return tblAssets;

	LuaTable ret = tblAssets;
	size_t prevPos = 0;
	size_t lastPos = path.rfind('/');

	// Parse given path and move by the tables tree
	while(prevPos < lastPos && ret)
	{
		size_t pos = path.find('/', prevPos);
		std::string name = path.substr(prevPos, pos - prevPos);
		LuaTable nextTable = ret.GetTable(name);

		// Make table if it isn't exists
		if(autoMake && !nextTable)
		{
			nextTable = LuaTable(LUNAEngine::SharedLua());
			ret.SetField(name, nextTable);
		}

		ret = nextTable;
		prevPos = pos + 1;
	}

	return ret;
}

// Get asset/folder name for path
std::string LUNAAssets::GetNameForPath(const std::string& path)
{
	return LUNAEngine::SharedFiles()->GetBasename(path);
}

// Check for given file is description
bool LUNAAssets::IsDescription(const std::string& path)
{
	std::string ext = LUNAEngine::SharedFiles()->GetExtension(path);

	if(ext == "atlas") return true;
	return false;
}

// Get loader for given file
std::shared_ptr<LUNAAssetLoader> LUNAAssets::GetLoader(const std::string& path)
{
	std::string ext = LUNAEngine::SharedFiles()->GetExtension(path);

	if(ext == "png")
	{
		// Load texture atlas if atlas desctiption file is exists
		std::string atlasPath = path.substr(0, path.rfind(".") + 1) + "atlas";
		if(LUNAEngine::SharedFiles()->IsFile(atlasPath)) return std::make_shared<LUNATextureAtlasLoader>();

		// Load just texture
		else return std::make_shared<LUNATextureLoader>();
	}

	return nullptr;
}

 // Load all assets
void LUNAAssets::LoadAll()
{
	LoadFolder("", true);
}

// Load all assets in given folder
void LUNAAssets::LoadFolder(const std::string& path, bool recursive)
{

}

// Load specifed asset file
void LUNAAssets::Load(const std::string& path)
{
	if(IsDescription(path)) return; // Skip description files

	auto loader = GetLoader(path);
	if(!loader)
	{
		LUNA_LOGE("Unknown asset type \"%s\"", LUNAEngine::SharedFiles()->GetExtension(path).c_str());
		return;
	}

	LuaTable parentTable = GetParentTableForPath(path, true);
	std::string name = LUNAEngine::SharedFiles()->GetBasename(path);

	// Don't load asset if it already exists
	if(parentTable.HasField(name))
	{
		LUNA_LOGE("Asset from file \"%s\" already loaded", path.c_str());
		return;
	}

	if(!loader->Load(path))
	{
		LUNA_LOGE("Cannot load asset from file \"%s\"", path.c_str());
		return;
	}

	std::string cleanName = LUNAEngine::SharedFiles()->SplitResolutionSuffix(name).first; // Remove resolution suffix
	loader->PushToLua(cleanName, parentTable);
}

// Unload specifed asset
void LUNAAssets::Unload(const std::string& path)
{

}

// Unload all assets in given folder
void LUNAAssets::UnloadFolder(const std::string& path)
{

}

// Unload all assets
void LUNAAssets::UnloadAll()
{
	UnloadFolder("");
}

// Get root table of asset tree
LuaTable LUNAAssets::GetRootTable()
{
	return tblAssets;
}

/*// Add "assets" flag to metatable of given table
// For check when unload for unloaded table is table with assets
void AddAssetFlag(LuaTable& table)
{
	LuaTable meta(table.GetRef()->GetLuaVm());
	meta.SetField("assets", true);
	table.SetMetatable(meta);
}

LUNAAsset::LUNAAsset(LUNAAssetType type) :
	type(type)
{
}

LUNAAssetType LUNAAsset::GetType()
{
	return type;
}

std::string LUNAAsset::GetReloadPath()
{
	return reloadPath;
}

void LUNAAsset::SetReloadPath(const std::string& path)
{
	reloadPath = path;
}


LUNAAssets::LUNAAssets() :
	tblAssets(LUNAEngine::SharedLua()),
	lastId(1) // 0 - is invalid asset id
{
	LuaScript *lua = LUNAEngine::SharedLua();
	LuaTable tblLuna = lua->GetGlobalTable().GetTable("luna");

	AddAssetFlag(tblAssets);
	tblLuna.SetField("assets", tblAssets);

	// Bind assets manager to lua
	LuaTable tblAssetsMgr(lua);
	tblAssetsMgr.SetField("loadAll", LuaFunction(lua, this, &LUNAAssets::LoadAll));
	tblAssetsMgr.SetField("loadFolder", LuaFunction(lua, this, &LUNAAssets::LoadFolder));
	tblAssetsMgr.SetField("load", LuaFunction(lua, this, &LUNAAssets::LoadFile));
	tblAssetsMgr.SetField("unload", LuaFunction(lua, this, &LUNAAssets::Unload));
	tblAssetsMgr.SetField("unloadAll", LuaFunction(lua, this, &LUNAAssets::UnloadAll));
	tblLuna.SetField("assetsmgr", tblAssetsMgr);
}

LUNAAssets::~LUNAAssets()
{
	UnloadAll();
}

// Load asset data
// "filename" - Full path to loaded file
// "cleanName" - Name of file without path and resolution suffix
// "table" - Table into tblAssets where to place loaded asset
void LUNAAssets::LoadAssetData(const std::string& filename, const std::string& cleanName, LuaTable& table)
{
	LUNAFiles *files = LUNAEngine::SharedFiles();
	std::string type = files->GetExtension(filename);

	// Load texture
	if(type == "png")
	{
		LUNAImage image(filename, LUNAPngFormat(), LUNAFileLocation::ASSETS);
		if(!image.IsEmpty())
		{
			std::shared_ptr<LUNATexture> texture = std::make_shared<LUNATexture>(image);
			texture->SetReloadPath(filename); // Set reload path for texture

			int id = GetNewId();
			loadedAssets[id] = texture;
			table.SetField(cleanName, id);
			return;
		}
	}

	// Load texture atlas
	else if(type == "atlas")
	{
		// Image for atlas has same name as description file,
		// just with different extension
		std::string imageName = filename.substr(0, filename.rfind(".") + 1) + "png";

		LUNAImage image(imageName, LUNAPngFormat(), LUNAFileLocation::ASSETS);
		if(!image.IsEmpty())
		{
			std::shared_ptr<LUNATexture> texture = std::make_shared<LUNATexture>(image);
			LUNATextureAtlas atlas(texture, filename);

			if(atlas.IsLoaded())
			{
				// Add atlas texture to assets
				int textureId = GetNewId();
				loadedAssets[textureId] = texture;
				texture->SetReloadPath(imageName);

				// Add regions to assets
				LuaTable atlasTable(LUNAEngine::SharedLua());
				for(auto entry : atlas.GetRegions())
				{
					int id = GetNewId();
					loadedAssets[id] = entry.second;
					atlasTable.SetField(entry.first, id);
				}

				// Save texture id in metatable of atlas table
				LuaTable meta(LUNAEngine::SharedLua());
				meta.SetField("textureId", textureId);
				meta.SetField("assets", true);
				atlasTable.SetMetatable(meta);

				table.SetField(cleanName, atlasTable);
				return;
			}
		}
	}

	else
	{
		LUNA_LOGE("Unknown asset type \"%s\"", type.c_str());
	}

	LUNA_LOGE("Cannot load file \"%s\"", filename.c_str());
}

 // Generate id for new asset
int LUNAAssets::GetNewId()
{
	return lastId++;
}

// Remove unloaded assets and empty tables from given assets table
void LUNAAssets::UpdateAssetsTable(LuaTable& table)
{
	for(auto entry : table)
	{
		int type = entry.second.GetType();
		if(type == LUA_TTABLE)
		{
			LuaTable curTable = entry.second.ToTable();
			if(curTable.IsEmpty()) table.RemoveField(entry.first.ToString());
			else UpdateAssetsTable(curTable);
		}
		else if(type == LUA_TNUMBER && loadedAssets.count(entry.second.ToInt()) == 0)
		{
			table.RemoveField(entry.first.ToString());
		}
	}
}

// Recursively unload all assets in given table
void LUNAAssets::UnloadTable(LuaTable& table)
{
	// If table is atlas table, unload atlas texture
	LuaTable meta = table.GetMetatable();
	if(meta != nil)
	{
		int textureId = meta.GetInt("textureId");
		if(textureId != 0) loadedAssets.erase(textureId);
	}

	for(auto entry : table)
	{
		int type = entry.second.GetType();
		if(type == LUA_TTABLE)
		{
			LuaTable curTable = entry.second.ToTable();
			UnloadTable(curTable);
			table.Clear();
		}
		else if(type == LUA_TNUMBER)
		{
			loadedAssets.erase(entry.second.ToInt());
		}
	}
}

// Load all assets
void LUNAAssets::LoadAll()
{
	LoadFolder("", true);
}

// Load all assets in given folder
void LUNAAssets::LoadFolder(const std::string& folder, bool recursive)
{
	if(folder == "scripts/") return; // Ignore scripts

	LUNAFiles *files = LUNAEngine::SharedFiles();

	if(!files->IsDirectory(folder))
	{
		LUNA_LOGE("Cannot load folder \"%s\". Folder not found", folder.c_str());
		return;
	}

	for(std::string filename : files->GetFileList(folder))
	{
		std::string path = folder + filename;

		if(files->IsFile(path))
		{
			// Check some special cases:
			// 1 case:
			//  Texture atlas is two files: image and description file
			//  To avoid loading image 2 times and namespace conflicts,
			//  skip loading image file for images having same-named description files
			std::string type = files->GetExtension(path);
			if(type == "png")
			{
				std::string atlasName = path.substr(0, path.rfind(".") + 1) + "atlas";
				if(files->IsExists(atlasName)) continue;
			}

			LoadFile(path);
		}
		else if(recursive) LoadFolder(path, recursive);
	}
}

// Load specifed asset file
void LUNAAssets::LoadFile(const std::string& filename)
{
	if(filename == CONFIG_FILENAME) return; // Ignore config file

	LuaScript *lua = LUNAEngine::SharedLua();
	LUNAFiles *files = LUNAEngine::SharedFiles();
	LUNASizes *sizes = LUNAEngine::SharedSizes();

	// Check for file is exists
	if(!files->IsFile(filename))
	{
		LUNA_LOGE("Cannot load file \"%s\". File not found", filename.c_str());
		return;
	}

	// Skip files with different from current resolution suffix
	auto splitted = files->SplitResolutionSuffix(files->GetBasename(filename));
	if(!splitted.second.empty() && splitted.second != sizes->GetResolutionSuffix()) return;

	// Parse path to file and make namespace in "luna.assets" table
	LuaTable table = tblAssets;
	std::string path = files->GetParentPath(filename);
	size_t prevPos = 0;
	while(prevPos < path.length())
	{
		size_t pos = path.find('/', prevPos);
		if(pos == std::string::npos) pos = path.length();
		std::string space = path.substr(prevPos, pos - prevPos);

		if(!table.HasField(space.c_str()))
		{
			LuaTable newTable(lua);
			AddAssetFlag(newTable);
			table.SetField(space.c_str(), newTable);
		}
		table = table.GetTable(space.c_str());

		prevPos = pos + 1;
	}

	// Check for file isn't already loaded
	std::string& cleanName = splitted.first;
	if(table.HasField(cleanName))
	{
		LUNA_LOGE("%s Asset with name \"%s\" already exists", filename.c_str(), cleanName.c_str());
		return;
	}

	// Load asset data
	LoadAssetData(filename, cleanName, table);
}

void LUNAAssets::Unload(const LuaDynamicType& asset)
{
	if(LUNAEngine::SharedGraphics()->GetRenderer()->IsInProgress())
	{
		LUNA_LOGE("Cannot unload any asset while rendering in progress");
		return;
	}

	// Unload all assets in given table
	if(asset.GetType() == LUA_TTABLE)
	{
		LuaTable table = asset.ToTable();

		// Check for given table is table with assets
		LuaTable meta = table.GetMetatable();
		if(meta == nil || !meta.HasField("assets"))
		{
			LUNA_LOGE("Unloaded table isn't asset table");
			return;
		}

		UnloadTable(table);
	}

	// Unload only given asset
	else if(asset.GetType() == LUA_TNUMBER)
	{
		loadedAssets.erase(asset.ToInt());
	}

	else
	{
		LUNA_LOGE("Attempt to unload incorrect asset");
		return;
	}

	UpdateAssetsTable(tblAssets);
}

void LUNAAssets::UnloadAll()
{
	loadedAssets.clear();
	tblAssets.Clear();
	lastId = 1;
}

// Reload some assets if OpenGL context was lost
void LUNAAssets::ReloadAssets()
{
	for(auto entry : loadedAssets) entry.second->Reload();
}

// Check type for assset at given id
bool LUNAAssets::IsAssetA(int assetId, LUNAAssetType type)
{
	return loadedAssets.count(assetId) > 0 && loadedAssets[assetId]->GetType() == type;
}*/