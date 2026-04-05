#include "pch.h"
#include "NavMeshFileReader.h"
#include <fstream>

/*---------------------
   NavMeshFileReader
---------------------*/

namespace SE::Navigation
{
   namespace
   {
      template<typename T>
      bool ReadRaw(std::ifstream& ifs, T& outValue)
      {
         return static_cast<bool>(ifs.read(reinterpret_cast<char*>(&outValue), sizeof(T)));
      }
   }
   
   bool NavMeshFileReader::ReadFromFile(const std::filesystem::path& filePath, NavMeshFileData& outData)
   {
      std::ifstream ifs(filePath, std::ios::binary);
      if (!ifs.is_open())
         return false;
      
      NavMeshSetHeader fileHeader{};
      if (!ReadRaw(ifs, fileHeader))
         return false;
      
      if (fileHeader.magic != kNavMeshFileMagic)
         return false;  // 매직 넘버 불일치
      
      if (fileHeader.version != kNavMeshFileVersion)
         return false;  // 버전 불일치
      
      outData = {};
      outData.header = fileHeader;
      outData.tiles.reserve(fileHeader.tileCount);
      
      for (uint32 i = 0; i < fileHeader.tileCount; ++i) {
         
         NavMeshTileBlob tile{};
         if (!ReadRaw(ifs, tile.header))
            return false;
         
         if (tile.header.dataSize == 0)
            return false;
         
         tile.data.resize(tile.header.dataSize);
         if (!ifs.read(reinterpret_cast<char*>(tile.data.data()), tile.header.dataSize))
            return false;
         
         outData.tiles.push_back(std::move(tile));
      }
      
      return true;
   }
}
