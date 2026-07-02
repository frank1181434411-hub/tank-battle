#include "world/map.hpp"

#include <iostream>

namespace {
bool expect(bool condition,const char* message)
{
    if(condition) return true;

    std::cerr << "FAILED: " << message << '\n';
    return false;
}

bool expectTile(const Map& map,int x,int y,TileType expected,const char* message)
{
    const Tile* tile=map.tileAt(x,y);
    if(tile!=nullptr && tile->type()==expected) return true;

    std::cerr << "FAILED: " << message << '\n';
    return false;
}
}

int main()
{
    Map map;
    bool ok=true;

    if(!map.loadFromFile("assets/maps/single/map_test.txt"))
    {
        std::cerr << "FAILED: load map_test.txt: " << map.lastError() << '\n';
        return 1;
    }

    ok=expect(!map.empty(),"map should not be empty") && ok;
    ok=expect(map.width()==20,"map width should be 20") && ok;
    ok=expect(map.height()==10,"map height should be 10") && ok;
    ok=expect(map.hasPlayerSpawn(),"player spawn should exist") && ok;
    ok=expect(map.hasBasePosition(),"base position should exist") && ok;
    ok=expect(map.enemySpawns().size()==2,"enemy spawn count should be 2") && ok;

    ok=expectTile(map,0,0,TileType::Brick,"top-left tile should be Brick") && ok;
    ok=expectTile(map,6,2,TileType::Grass,"grass tile should be Grass") && ok;
    ok=expectTile(map,10,4,TileType::Water,"water tile should be Water") && ok;
    ok=expectTile(map,10,5,TileType::Steel,"steel tile should be Steel") && ok;
    ok=expectTile(map,1,1,TileType::Empty,"player spawn tile should be Empty") && ok;
    ok=expectTile(map,15,6,TileType::Empty,"enemy spawn tile should be Empty") && ok;
    ok=expectTile(map,10,8,TileType::Empty,"base tile should be Empty") && ok;

    if(!ok) return 1;

    std::cout << "map_test passed\n";
    return 0;
}
