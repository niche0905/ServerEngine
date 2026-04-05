#pragma once
#include <filesystem>
#include "Navigation/File/NavMeshFileFormat.h"

/*---------------------
   NavMeshFileReader
---------------------*/
//
// NavMeshFileReader는 네비게이션 메시 파일을 읽고 파싱하는 역할을 담당하는 클래스입니다.
// 

namespace SE::Navigation
{
   class NavMeshFileReader
   {
   public:
      static bool ReadFromFile(const std::filesystem::path& filePath, NavMeshFileData& outData);
    
   };

}
