#include "wavegenerator.h"
#include <random>
#include <chrono>
#include <cmath>

/*
    Function to empty the current spawn list
*/
void WaveGenerator::clearSpawnList(){
    spawnList.clear(); //Don't delete the objects because they will be passed over to the game enemy list
}

/*
    Function to randomly generate a new spawn list
    @param wave use current wave value to calculate how many enemies to spawn
    @param spawnLocation the location to instantiate the new enemies
*/
std::vector<Enemy*> WaveGenerator::generateSpawnList(int wave, QPointF spawnLocation){
    //Start by clearing spawnList
    clearSpawnList();

    //Allocate spawn tokens based on current wave. It increases every 5 waves
    int spawnTokens = std::ceil(wave * 0.2) * 10;

    //Generate an int distribution for each of the enemy types
    std::uniform_int_distribution<int> unif(0,2);

    //randomly generate enemies
    do{
        int token = unif(generator);
        switch( token ){
            case 0:
                if(spawnTokens >= 1){
                    spawnList.push_back(new Enemy(Enemy_Type::NORMAL, spawnLocation));
                    spawnTokens -= 1;
                }
                break;
            case 1:
                if(spawnTokens >= 3){
                    spawnList.push_back(new Enemy(Enemy_Type::BADASS, spawnLocation));
                    spawnTokens -= 3;
                }
                break;
            case 2:
                if(spawnTokens >= 3){
                    spawnList.push_back(new Enemy(Enemy_Type::BAT, spawnLocation));
                    spawnTokens -= 3;
                }
                break;
        }
    }while(spawnTokens > 0);
    return spawnList;
}
