#include"ai/enemy_ai.hpp"

#include<iostream>
#include<string>
#include<vector>

namespace
{
struct TestResult
{
    int passed=0;
    int failed=0;
};

bool expect(bool condition,const std::string& message,TestResult& result)
{
    if(condition)
    {
        ++result.passed;
        std::cout<<"[PASS] "<<message<<"\n";
        return true;
    }

    ++result.failed;
    std::cout<<"[FAIL] "<<message<<"\n";
    return false;
}

sf::Vector2f cell(int x,int y)
{
    return {static_cast<float>(x)*Tile::DefaultSize+Tile::DefaultSize*0.5f,
            static_cast<float>(y)*Tile::DefaultSize+Tile::DefaultSize*0.5f};
}

Map makeMap(const std::vector<std::string>& lines)
{
    Map map;
    if(!map.loadFromLines(lines))
        std::cout<<"map load failed: "<<map.lastError()<<"\n";

    return map;
}

AIContext makeContext(const Map& map,const std::vector<PlayerTank>& players,const Base& base,const std::vector<EnemyTank>& enemies,const std::vector<Bullet>& bullets)
{
    return {map,players,base,enemies,bullets};
}

void testClearShot(TestResult& result)
{
    Map map=makeMap({
        ".......",
        ".......",
        "......."
    });

    EnemyTank enemy(cell(1,1).x,cell(1,1).y,EnemyType::Light);
    PlayerTank player(cell(5,1).x,cell(5,1).y);
    Base base(cell(6,2).x,cell(6,2).y);
    std::vector<PlayerTank> players{player};
    std::vector<EnemyTank> enemies{enemy};
    std::vector<Bullet> bullets;
    AIContext context=makeContext(map,players,base,enemies,bullets);

    NormalAI ai(1);
    TankCommand command=ai.decide(enemy,context,0.2f);

    expect(command.fire,"normal ai fires when target is aligned and unobstructed",result);
    expect(!command.move,"normal ai stops moving while firing directly",result);
    expect(command.direction==Direction::Right,"normal ai faces target before direct fire",result);
}

void testSteelBlocksShot(TestResult& result)
{
    Map map=makeMap({
        ".......",
        "...S...",
        "......."
    });

    EnemyTank enemy(cell(1,1).x,cell(1,1).y,EnemyType::Light);
    PlayerTank player(cell(5,1).x,cell(5,1).y);
    Base base(cell(6,2).x,cell(6,2).y);
    std::vector<PlayerTank> players{player};
    std::vector<EnemyTank> enemies{enemy};
    std::vector<Bullet> bullets;
    AIContext context=makeContext(map,players,base,enemies,bullets);

    NormalAI ai(2);
    TankCommand command=ai.decide(enemy,context,0.2f);

    expect(command.move,"normal ai does not stop to direct-fire through steel",result);
}

void testWaterBlocksPath(TestResult& result)
{
    Map map=makeMap({
        ".....",
        ".....",
        ".W...",
        ".....",
        "....."
    });

    EnemyTank enemy(cell(1,1).x,cell(1,1).y,EnemyType::Light);
    PlayerTank player(cell(3,3).x,cell(3,3).y);
    Base base(cell(4,4).x,cell(4,4).y);
    std::vector<PlayerTank> players{player};
    std::vector<EnemyTank> enemies{enemy};
    std::vector<Bullet> bullets;
    AIContext context=makeContext(map,players,base,enemies,bullets);

    NormalAI ai(3);
    TankCommand command=ai.decide(enemy,context,0.2f);

    expect(command.move,"normal ai moves toward an unaligned target",result);
    expect(command.direction!=Direction::Down,"normal ai avoids water as the next path tile",result);
}

void testSealedTarget(TestResult& result)
{
    Map map=makeMap({
        ".......",
        ".......",
        "..SSS..",
        "..S.S..",
        "..SSS..",
        "......."
    });

    EnemyTank enemy(cell(1,1).x,cell(1,1).y,EnemyType::Light);
    PlayerTank player(cell(3,3).x,cell(3,3).y);
    Base base(cell(6,5).x,cell(6,5).y);
    std::vector<PlayerTank> players{player};
    std::vector<EnemyTank> enemies{enemy};
    std::vector<Bullet> bullets;
    AIContext context=makeContext(map,players,base,enemies,bullets);

    NormalAI ai(4);
    TankCommand command=ai.decide(enemy,context,0.2f);

    expect(command.move,"normal ai falls back to patrol when target is sealed",result);
}

void testNearestLivingPlayer(TestResult& result)
{
    Map map=makeMap({
        ".......",
        ".......",
        ".......",
        ".......",
        "......."
    });

    EnemyTank enemy(cell(3,3).x,cell(3,3).y,EnemyType::Light);
    PlayerTank nearPlayer(cell(3,1).x,cell(3,1).y);
    PlayerTank farPlayer(cell(6,3).x,cell(6,3).y);
    Base base(cell(0,4).x,cell(0,4).y);
    std::vector<PlayerTank> players{farPlayer,nearPlayer};
    std::vector<EnemyTank> enemies{enemy};
    std::vector<Bullet> bullets;
    AIContext context=makeContext(map,players,base,enemies,bullets);

    NormalAI ai(5);
    TankCommand command=ai.decide(enemy,context,0.2f);

    expect(command.direction==Direction::Up,"normal ai selects nearest living player",result);

    players[1].setHp(0);
    AIContext nextContext=makeContext(map,players,base,enemies,bullets);
    ai.reset();
    command=ai.decide(enemy,nextContext,0.2f);

    expect(command.direction==Direction::Right,"normal ai switches target after nearest player dies",result);
}

void testIndependentAiInstances(TestResult& result)
{
    Map map=makeMap({
        ".......",
        ".......",
        ".......",
        ".......",
        "......."
    });

    EnemyTank leftEnemy(cell(1,3).x,cell(1,3).y,EnemyType::Light);
    EnemyTank rightEnemy(cell(5,3).x,cell(5,3).y,EnemyType::Light);
    PlayerTank player(cell(3,3).x,cell(3,3).y);
    Base base(cell(0,4).x,cell(0,4).y);
    std::vector<PlayerTank> players{player};
    std::vector<EnemyTank> enemies{leftEnemy,rightEnemy};
    std::vector<Bullet> bullets;
    AIContext context=makeContext(map,players,base,enemies,bullets);

    NormalAI leftAi(6);
    NormalAI rightAi(7);
    TankCommand leftCommand=leftAi.decide(leftEnemy,context,0.2f);
    TankCommand rightCommand=rightAi.decide(rightEnemy,context,0.2f);

    expect(leftCommand.direction==Direction::Right,"left enemy ai makes its own target decision",result);
    expect(rightCommand.direction==Direction::Left,"right enemy ai makes its own target decision",result);
}

void testHardAiDirectPlayerShot(TestResult& result)
{
    Map map=makeMap({
        ".......",
        ".......",
        "......."
    });

    EnemyTank enemy(cell(1,1).x,cell(1,1).y,EnemyType::Heavy);
    PlayerTank player(cell(5,1).x,cell(5,1).y);
    Base base(cell(6,2).x,cell(6,2).y);
    base.setHp(0);
    std::vector<PlayerTank> players{player};
    std::vector<EnemyTank> enemies{enemy};
    std::vector<Bullet> bullets;
    AIContext context=makeContext(map,players,base,enemies,bullets);

    HardAI ai(8);
    TankCommand command=ai.decide(enemy,context,0.2f);

    expect(command.fire,"hard ai fires at an unobstructed player",result);
    expect(!command.move,"hard ai stops before direct player fire",result);
    expect(command.direction==Direction::Right,"hard ai faces player before direct fire",result);
}

void testHardAiAttacksBrickOnLine(TestResult& result)
{
    Map map=makeMap({
        ".......",
        "...#...",
        "......."
    });

    EnemyTank enemy(cell(1,1).x,cell(1,1).y,EnemyType::Heavy);
    PlayerTank player(cell(5,1).x,cell(5,1).y);
    Base base(cell(6,2).x,cell(6,2).y);
    base.setHp(0);
    std::vector<PlayerTank> players{player};
    std::vector<EnemyTank> enemies{enemy};
    std::vector<Bullet> bullets;
    AIContext context=makeContext(map,players,base,enemies,bullets);

    HardAI ai(9);
    TankCommand command=ai.decide(enemy,context,0.2f);

    expect(command.fire,"hard ai fires to break a brick on target line",result);
    expect(!command.move,"hard ai stops before firing at brick",result);
    expect(command.direction==Direction::Right,"hard ai faces brick blocking target line",result);
}

void testHardAiDirectBaseShot(TestResult& result)
{
    Map map=makeMap({
        ".......",
        ".......",
        "......."
    });

    EnemyTank enemy(cell(1,1).x,cell(1,1).y,EnemyType::Heavy);
    PlayerTank player(cell(5,2).x,cell(5,2).y);
    player.setHp(0);
    Base base(cell(5,1).x,cell(5,1).y);
    std::vector<PlayerTank> players{player};
    std::vector<EnemyTank> enemies{enemy};
    std::vector<Bullet> bullets;
    AIContext context=makeContext(map,players,base,enemies,bullets);

    HardAI ai(10);
    TankCommand command=ai.decide(enemy,context,0.2f);

    expect(command.fire,"hard ai fires at base when no player is alive",result);
    expect(command.direction==Direction::Right,"hard ai faces base before direct fire",result);
}

void testHardAiMovesToShotCell(TestResult& result)
{
    Map map=makeMap({
        ".......",
        ".......",
        ".......",
        ".......",
        "......."
    });

    EnemyTank enemy(cell(1,1).x,cell(1,1).y,EnemyType::Heavy);
    PlayerTank player(cell(5,3).x,cell(5,3).y);
    Base base(cell(0,4).x,cell(0,4).y);
    base.setHp(0);
    std::vector<PlayerTank> players{player};
    std::vector<EnemyTank> enemies{enemy};
    std::vector<Bullet> bullets;
    AIContext context=makeContext(map,players,base,enemies,bullets);

    HardAI ai(11);
    TankCommand command=ai.decide(enemy,context,0.2f);

    expect(command.move,"hard ai moves toward a valid shot cell when not aligned",result);
}
}

int main()
{
    TestResult result;

    testClearShot(result);
    testSteelBlocksShot(result);
    testWaterBlocksPath(result);
    testSealedTarget(result);
    testNearestLivingPlayer(result);
    testIndependentAiInstances(result);
    testHardAiDirectPlayerShot(result);
    testHardAiAttacksBrickOnLine(result);
    testHardAiDirectBaseShot(result);
    testHardAiMovesToShotCell(result);

    std::cout<<"AI tests passed: "<<result.passed<<"\n";
    std::cout<<"AI tests failed: "<<result.failed<<"\n";

    return result.failed==0?0:1;
}
