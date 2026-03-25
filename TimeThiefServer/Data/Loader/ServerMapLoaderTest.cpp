#include "pch.h"
#include "Data/Loader/ServerMapLoaderTest.h"
#include "Data/Loader/ServerMapLoader.h"

#include <fstream>
#include <iostream>

namespace se::map
{
    namespace
    {
        bool WriteTestMapFile(const std::string& filePath)
        {
            MapHeader header{};
            header.colliderCount = 1;
            
            ColliderData collider{};
            collider.type = ColliderType::OBB;
            collider.flags = ColliderFlags::Collider_BlockMovement | ColliderFlags::Collider_BlockProjectile;
            collider.position = { 100.f, 200.f, 300.f };
            collider.rotationDeg = { 0.f, 45.f, 0.f };
            collider.extents = { 50.f, 60.f, 70.f };
            collider.radius = 0.f;
            collider.halfHeight = 0.f;
            
            std::ofstream file(filePath, std::ios::binary);
            if (not file.is_open()) {
                return false;
            }
            
            file.write(reinterpret_cast<const char*>(&header), sizeof(MapHeader));
            file.write(reinterpret_cast<const char*>(&collider), sizeof(ColliderData));
            
            return static_cast<bool>(file);
        }
    }
    
    bool RunServerMapLoaderTest()
    {
        const std::string filePath = "TestMap_Tagged.servermap";

        // if (not WriteTestMapFile(filePath)) {
        //     std::cout << "[Test] WriteTestMapFile failed\n";
        //     return false;
        // }

        ServerMapLoader loader;
        LoadedMapData loadedMapData;

        if (not loader.LoadFromFile(filePath, loadedMapData)) {
            std::cout << "[Test] LoadFromFile failed\n";
            return false;
        }

        std::cout << "[Test] colliderCount = " << loadedMapData.header.colliderCount << "\n";

        if (loadedMapData.colliders.empty()) {
            std::cout << "[Test] No colliders loaded\n";
            return false;
        }
        
        for (size_t i = 0; i < loadedMapData.colliders.size(); ++i) {
            const ColliderData& collider = loadedMapData.colliders[i];
            std::cout << "[Test] Collider " << i << ":\n";
            std::cout << "  type = " << static_cast<int>(collider.type) << "\n";
            std::cout << "  flags = " << collider.flags << "\n";
            std::cout << "  position = "
                      << collider.position.x << ", "
                      << collider.position.y << ", "
                      << collider.position.z << "\n";
            std::cout << "  rotationDeg = "
                      << collider.rotationDeg.x << ", "
                      << collider.rotationDeg.y << ", "
                      << collider.rotationDeg.z << "\n";
            std::cout << "  extents = "
                      << collider.extents.x << ", "
                      << collider.extents.y << ", "
                      << collider.extents.z << "\n";
            std::cout << "  radius = " << collider.radius << "\n";
            std::cout << "  halfHeight = " << collider.halfHeight << "\n";
        }

        return true;
    }
    
}
