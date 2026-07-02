#include "world/map.hpp"
#include <fstream>

namespace
{
bool isValidMapCharacter(char value) noexcept
{
    return value=='.'||value==' '||
        value=='#'||value=='S'||
        value=='W'||value=='G'||
        value=='P'||value=='B'||
        value=='L'||value=='H';
}
}

bool Map::loadFromFile(const std::string& filePath)
{
    std::ifstream input(filePath);
    if(!input)
    {
        lastError_="failed to open map file: "+filePath;
        return false;
    }

    std::vector<std::string> lines;
    std::string line;
    while(std::getline(input,line))
        lines.push_back(line);

    return loadFromLines(lines);
}

bool Map::loadFromLines(const std::vector<std::string>& lines)
{
    clear();
    if(lines.empty())
    {
        lastError_="map is empty";
        return false;
    }

    int maxWidth=0;
    for(const auto& line:lines)
    {
        if(static_cast<int>(line.size())>maxWidth)
            maxWidth=static_cast<int>(line.size());
    }

    if(maxWidth<=0)
    {
        lastError_="map has no tiles";
        return false;
    }

    resize(maxWidth,static_cast<int>(lines.size()));

    for(int y=0;y<height_;++y)
    {
        const std::string& line=lines[static_cast<std::size_t>(y)];
        for(int x=0;x<width_;++x)
        {
            char value='.';
            if(x<static_cast<int>(line.size()))
                value=line[static_cast<std::size_t>(x)];

            if(!isValidMapCharacter(value))
            {
            const std::string error=
                "invalid map character '"+
                std::string(1,value)+
                "' at ("+
                std::to_string(x)+
                ","+
                std::to_string(y)+
                ")";
            clear();
            lastError_=error;
            return false;
            }
        setTile(x,y,Tile::fromChar(value));
        readSpecialPoint(value,x,y);
        }
    }

    lastError_.clear();
    return true;
}

void Map::clear()
{
    width_=0;
    height_=0;
    tiles_.clear();
    hasPlayerSpawn_=false;
    hasBasePosition_=false;
    playerSpawn_={0.f,0.f};
    basePosition_={0.f,0.f};
    enemySpawns_.clear();
    lastError_.clear();
}

void Map::resize(int width,int height,TileType fillType)
{
    if(width<=0 || height<=0)
    {
        clear();
        return;
    }

    width_=width;
    height_=height;
    tiles_.assign(static_cast<std::size_t>(width_*height_),Tile(fillType));
}

bool Map::empty() const noexcept{return tiles_.empty();}

int Map::width() const noexcept{return width_;}

int Map::height() const noexcept{return height_;}

bool Map::isInBounds(int x,int y) const noexcept
{
    return (x>=0 && y>=0 && x<width_ && y<height_);
}

Tile* Map::tileAt(int x,int y) noexcept
{
    if(!isInBounds(x,y)) return nullptr;
    return &tiles_[static_cast<std::size_t>(index(x,y))];
}

const Tile* Map::tileAt(int x,int y) const noexcept
{
    if(!isInBounds(x,y)) return nullptr;
    return &tiles_[static_cast<std::size_t>(index(x,y))];
}

void Map::setTile(int x,int y,TileType type)
{
    Tile* tile=tileAt(x,y);
    if(tile==nullptr) return;
    tile->setType(type);
}

sf::Vector2f Map::tilePosition(int x,int y) const noexcept
{
    return {static_cast<float>(x)*Tile::DefaultSize,static_cast<float>(y)*Tile::DefaultSize};
}

sf::Vector2f Map::tileCenter(int x,int y) const noexcept
{
    return tilePosition(x,y)+sf::Vector2f(Tile::DefaultSize*0.5f,Tile::DefaultSize*0.5f);
}

bool Map::hasPlayerSpawn() const noexcept{return hasPlayerSpawn_;}

sf::Vector2f Map::playerSpawn() const noexcept{return playerSpawn_;}

bool Map::hasBasePosition() const noexcept{return hasBasePosition_;}

sf::Vector2f Map::basePosition() const noexcept{return basePosition_;}

const std::vector<EnemySpawn>& Map::enemySpawns() const noexcept{return enemySpawns_;}

const std::string& Map::lastError() const noexcept{return lastError_;}

void Map::draw(sf::RenderWindow& window) const
{
    drawLayer(window,0);
    drawLayer(window,1);
}

void Map::drawLayer(sf::RenderWindow& window,int layer) const
{
    sf::RectangleShape shape({Tile::DefaultSize,Tile::DefaultSize});
    for(int y=0;y<height_;++y)
    {
        for(int x=0;x<width_;++x)
        {
            const Tile* tile=tileAt(x,y);
            if(tile==nullptr || !tile->isDrawable()) continue;
            if(tile->drawLayer()!=layer) continue;

            shape.setPosition(tilePosition(x,y));
            shape.setFillColor(tile->color());
            window.draw(shape);
        }
    }
}

int Map::index(int x,int y) const noexcept{return y*width_+x;}

void Map::readSpecialPoint(char value,int x,int y)
{
    if(value=='P')
    {
        hasPlayerSpawn_=true;
        playerSpawn_=tileCenter(x,y);
    }
    else if(value=='B')
    {
        hasBasePosition_=true;
        basePosition_=tileCenter(x,y);
    }
    else if(value=='L')
    {
        enemySpawns_.push_back(
            {
                tileCenter(x,y),
                EnemyType::Light
            }
        );
    }
    else if(value=='H')
    {
        enemySpawns_.push_back(
            {
                tileCenter(x,y),
                EnemyType::Heavy
            }
        );
    }
}
