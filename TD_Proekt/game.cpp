#include "game.h"
#include "waypoint.h"
#include "enemy.h"
#include "wavegenerator.h"

#include <QApplication>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>


Game::Game(QWidget *parent) : QWidget(parent) , state(MENU), helpIndex(0) , curTowerOpt(0), curTowerType(FIRE), wave_value(0), score_value(10) ,
    enemyCount(0), generator(SEED), damageDisplayOffset(-2,2), tooltip(NULL)
{
    setWindowTitle("Tower Defence");
    setFixedSize(CONSTANTS::SCREEN_WIDTH, CONSTANTS::SCREEN_HEIGHT);

    setMouseTracking(true);

    fillCharReferences();
    loadMenu();
    loadHelp();
    loadPause();
    loadInGame();
}

Game::~Game()
{
    cleanMenu();
    cleanHelp();
    cleanPause();
    cleanInGame();
    cleanCharReferences();
}

void Game::cleanCharReferences(){
    for(auto& c : letterChars)
        delete c;
    for(auto& c : letterCharsAct)
        delete c;
    for(auto& c : letterCharsRed)
        delete c;
    for(auto& c : specialChars)
        delete c;
}

void Game::paintEvent(QPaintEvent*){
    QPainter painter(this);

    switch(state){
        case MENU:
            painter.drawImage(*title_line1->getRect(), *title_line1->getImage());
            painter.drawImage(*title_line2->getRect(), *title_line2->getImage());

            if(start_button->isActive())
                painter.drawImage(*start_button->getRect(), start_button->getActiveImage());
            else
                painter.drawImage(*start_button->getRect(), *start_button->getImage());

            if(help_button->isActive())
                painter.drawImage(*help_button->getRect(), help_button->getActiveImage());
            else
                painter.drawImage(*help_button->getRect(), *help_button->getImage());
            if(quit_button->isActive())
                painter.drawImage(*quit_button->getRect(), quit_button->getActiveImage());
            else
                painter.drawImage(*quit_button->getRect(), *quit_button->getImage());
            break;
        case INGAME:
            paintChar(std::to_string(getWave()),1,painter,10,10+wave_title->getRect()->height(),false);
            painter.drawImage(*score_title->getRect(),*score_title->getImage());
            paintChar(std::to_string(getScore()),1,painter,width()-std::to_string(getScore()).length()*6-5, 10+score_title->getRect()->height(),false);
            painter.drawImage(*wave_title->getRect(),*wave_title->getImage());

            for(const auto o : towerOptions)
                painter.drawImage(*o->getRect(), *o->getImage());
            painter.drawImage(*towerOptions[curTowerOpt]->getRect(), *towerOptHighlight->getImage());

            switch(curTowerOpt){
                case 0:
                    for(auto& u : fire_upgrade)
                        painter.drawImage(*u->getRect(), *u->getImage());
                    break;
                case 1:
                    for(auto& u : ice_upgrade)
                        painter.drawImage(*u->getRect(), *u->getImage());
                    break;
                case 2:
                    for(auto& u : earth_upgrade)
                        painter.drawImage(*u->getRect(), *u->getImage());
                    break;
            }

            for(auto& i : upgrade_icon)
                painter.drawImage(*i->getRect(), *i->getImage());

            for(auto& t : map){
                painter.drawImage(*t->getRect(), *t->getImage());
                if(t->isActive())
                    painter.drawImage(*tileHighlight->getRect(), *tileHighlight->getImage());
            }

            for(auto& e : enemies){
                if(!e->isDead())
                    painter.drawImage(*e->getRect(), *e->getImage());
            }
\
            for(const auto t : towers)
                painter.drawImage(*t->getRect(), *t->getImage());

            for(const auto d : damageDisplays)
                painter.drawImage(*d->getRect(), *d->getImage());

            if(tooltip != NULL)
                tooltip->paint(&painter);
            break;
        case CLEARED:
            paintChar("wave "+std::to_string(getWave())+" cleared",0.25,painter,(width()-(13+std::to_string(getWave()).length())*20)/2,100,false);
            if(continue_button->isActive())
                painter.drawImage(*continue_button->getRect(), continue_button->getActiveImage());
            else
                painter.drawImage(*continue_button->getRect(), *continue_button->getImage());
            break;
        case PAUSED:
            for(const auto b : pauseButtons){
                if(b->isActive())
                    painter.drawImage(*b->getRect(), b->getActiveImage());
                else
                    painter.drawImage(*b->getRect(), *b->getImage());
            }
            break;
        case HELP:
            painter.drawImage(*helpImages[helpIndex]->getRect(), *helpImages[helpIndex]->getImage());

            for(const auto b : arrows){
                if(b->isActive())
                    painter.drawImage(*b->getRect(), b->getActiveImage());
                else
                    painter.drawImage(*b->getRect(), *b->getImage());
            }
            break;
    }
}

void Game::timerEvent(QTimerEvent *event){
    if(state == INGAME){
        if(event->timerId() == spawnTimer)
            spawner();
    }
    repaint();
}

void Game::spawner(){
    killTimer(spawnTimer);

    if(!spawnList.empty()){
        enemies.push_back(spawnList.back());
        spawnTimer = startTimer(spawnList.back()->getSpawnDelay());
        spawnList.pop_back();
    }
}

void Game::moveEnemies(){
    for(auto& e : enemies){
        if(e->getRect()->contains(navPath[CONSTANTS::PATH_TILE_COUNT - 1].toPoint())){
            state = MENU;
            break;
        }
        if(e->getRect()->contains(navPath[e->getCurWaypoint()+1].toPoint()))
        {
            e->incrementCurWaypoint();
        }
        e->move(navPath[e->getCurWaypoint()+1]);
    }
}

void Game::keyPressEvent(QKeyEvent* event){
    if(state == INGAME){
        switch(event->key()){
            case Qt::Key_P:
                    state = PAUSED;
                    break;
            case Qt::Key_Escape:
                    qApp->exit();
                    break;
            default:
                QWidget::keyPressEvent(event);
        }
    }
    else
        QWidget::keyPressEvent(event);
}

void Game::mouseMoveEvent(QMouseEvent *event){
    switch(state){
        case MENU:
            if(start_button->getRect()->contains(event->pos())){
                start_button->setActive(true);
                help_button->setActive(false);
                quit_button->setActive(false);
            }
            else if(help_button->getRect()->contains(event->pos())){
                help_button->setActive(true);
                start_button->setActive(false);
                quit_button->setActive(false);
            }
            else if(quit_button->getRect()->contains(event->pos())){
                quit_button->setActive(true);
                start_button->setActive(false);
                help_button->setActive(false);
            }
            else{
                start_button->setActive(false);
                help_button->setActive(false);
                quit_button->setActive(false);
            }
            break;
        case PAUSED:
            if(pauseButtons[0]->getRect()->contains(event->pos())){
                pauseButtons[0]->setActive(true);
                pauseButtons[1]->setActive(false);
            }
            else if(pauseButtons[1]->getRect()->contains(event->pos())){
                pauseButtons[0]->setActive(false);
                pauseButtons[1]->setActive(true);
            }
            else{
                pauseButtons[0]->setActive(false);
                pauseButtons[1]->setActive(false);
            }
            break;
        case HELP:
            if(arrows[0]->getRect()->contains(event->pos())){
                arrows[0]->setActive(true);
                arrows[1]->setActive(false);
                arrows[2]->setActive(false);
            }
            else if(arrows[1]->getRect()->contains(event->pos())){
                arrows[0]->setActive(false);
                arrows[1]->setActive(true);
                arrows[2]->setActive(false);
            }
            else if(arrows[2]->getRect()->contains(event->pos())){
                arrows[0]->setActive(false);
                arrows[1]->setActive(false);
                arrows[2]->setActive(true);
            }
            else{
                arrows[0]->setActive(false);
                arrows[1]->setActive(false);
                arrows[2]->setActive(false);
            }
            break;
        case CLEARED:
            if(continue_button->getRect()->contains(event->pos()))
                continue_button->setActive(true);
            else
                continue_button->setActive(false);
            break;
        case INGAME:
            delete tooltip;
            tooltip = NULL;

            if(towerOptions[0]->getRect()->contains(event->pos())){
                tooltip = new ToolTip(mergeChars("cost", 1, NORMAL),
                                      mergeChars(std::to_string(Tower::getCost(FIRE)), 1, ACTIVE));
                tooltip->moveTo(event->pos());
            }

            else if(towerOptions[1]->getRect()->contains(event->pos())){
                tooltip = new ToolTip(mergeChars("cost", 1, NORMAL),
                                      mergeChars(std::to_string(Tower::getCost(ICE)), 1, ACTIVE));
                tooltip->moveTo(event->pos());
            }

            else if(towerOptions[2]->getRect()->contains(event->pos())){
                tooltip = new ToolTip(mergeChars("cost", 1, NORMAL),
                                      mergeChars(std::to_string(Tower::getCost(EARTH)), 1, ACTIVE));
                tooltip->moveTo(event->pos());
            }

            if(upgrade_icon[0]->getRect()->contains(event->pos())){
                tooltip = new ToolTip(mergeChars("cost", 1, NORMAL),
                                      mergeChars(std::to_string(Tower::getDamageCost(curTowerType)), 1, ACTIVE),
                                      mergeChars("str", 1, NORMAL),
                                      mergeChars(std::to_string(Tower::getDamage(curTowerType)), 1, ACTIVE));
                tooltip->moveTo(event->pos());
            }
            else if(upgrade_icon[1]->getRect()->contains(event->pos())){
                tooltip = new ToolTip(mergeChars("cost", 1, NORMAL),
                                      mergeChars(std::to_string(Tower::getRangeCost(curTowerType)), 1, ACTIVE),
                                      mergeChars("range", 1, NORMAL),
                                      mergeChars(std::to_string(Tower::getRange(curTowerType)), 1, ACTIVE));
                tooltip->moveTo(event->pos());
            }
            else if(upgrade_icon[2]->getRect()->contains(event->pos())){
               tooltip = new ToolTip(mergeChars("cost", 1, NORMAL),
                                      mergeChars(std::to_string(Tower::getCoolDownCost(curTowerType)), 1, ACTIVE),
                                      mergeChars("rate", 1, NORMAL),
                                      mergeChars(std::to_string(Tower::getCoolDown(curTowerType)), 1, ACTIVE));
                tooltip->moveTo(event->pos());
            }
            break;
    }
    repaint();
}

void Game::mousePressEvent(QMouseEvent *event){
    switch(state){
        case MENU:
            if(start_button->getRect()->contains(event->pos())){
                state = INGAME;
                newGame();
            }
            else if(help_button->getRect()->contains(event->pos())){
                state = HELP;
            }
            else if(quit_button->getRect()->contains(event->pos())){
                qApp->quit();
            }
            break;
        case PAUSED:
            if(pauseButtons[0]->getRect()->contains(event->pos())){
                state = INGAME;
                startTimers();

            }
            else if(pauseButtons[1]->getRect()->contains(event->pos())){
                killTimer(paintTimer);
                state = MENU;
            }
            break;
        case HELP:
            if(arrows[0]->getRect()->contains(event->pos())){
                if(helpIndex == 0)
                    helpIndex = helpImages.size()-1;
                else
                    helpIndex--;
            }
            else if(arrows[1]->getRect()->contains(event->pos())){
                if(helpIndex == helpImages.size()-1)
                    helpIndex = 0;
                else
                    helpIndex++;
            }
            else if(arrows[2]->getRect()->contains(event->pos())){
                helpIndex = 0;
                state = MENU;
            }
            repaint();
            break;
    case INGAME:
        for(auto& t : map)
            (!t->isPath() && !t->isOccupied() && t->getRect()->contains(event->pos())) ? selectTile(t) : t->setActive(false);

        for(size_t i=0; i<towerOptions.size(); i++){
            if(towerOptions[i]->getRect()->contains(event->pos())){
                curTowerOpt = i;
                switch(curTowerOpt){
                    case 0:
                        curTowerType = FIRE;
                        break;
                    case 1:
                        curTowerType = ICE;
                        break;
                    case 2:
                        curTowerType = EARTH;
                        break;
                }
            }
        }

        if(getScore() > Tower::getDamageCost(curTowerType) && upgrade_icon[0]->getRect()->contains(event->pos())){
            updateScore(-Tower::getDamageCost(curTowerType));
            Tower::upgradeDamage(curTowerType);
            tooltip = new ToolTip(mergeChars("cost", 1, NORMAL),
                                  mergeChars(std::to_string(Tower::getDamageCost(curTowerType)), 1, ACTIVE),
                                  mergeChars("str", 1, NORMAL),
                                  mergeChars(std::to_string(Tower::getDamage(curTowerType)), 1, ACTIVE));
            tooltip->moveTo(event->pos());
        }
        else if(getScore() > Tower::getRangeCost(curTowerType) && upgrade_icon[1]->getRect()->contains(event->pos())){
            updateScore(-Tower::getRangeCost(curTowerType));
            Tower::upgradeRange(curTowerType);
            tooltip = new ToolTip(mergeChars("cost", 1, NORMAL),
                                  mergeChars(std::to_string(Tower::getRangeCost(curTowerType)), 1, ACTIVE),
                                  mergeChars("range", 1, NORMAL),
                                  mergeChars(std::to_string(Tower::getRange(curTowerType)), 1, ACTIVE));
            tooltip->moveTo(event->pos());
        }
        else if(getScore() > Tower::getCoolDownCost(curTowerType) && upgrade_icon[2]->getRect()->contains(event->pos())){
            updateScore(-Tower::getCoolDownCost(curTowerType));
            Tower::upgradeCoolDown(curTowerType);
            tooltip = new ToolTip(mergeChars("cost", 1, NORMAL),
                                  mergeChars(std::to_string(Tower::getCoolDownCost(curTowerType)), 1, ACTIVE),
                                  mergeChars("rate", 1, NORMAL),
                                  mergeChars(std::to_string(Tower::getCoolDown(curTowerType)), 1, ACTIVE));
            tooltip->moveTo(event->pos());
        }

        break;
    case CLEARED:
        if(continue_button->getRect()->contains(event->pos())){
            newWave(); //start next wave
            state = INGAME;
        }
        break;
    }
}

void Game::newGame(){
    clearGame();
    wave_value = 0;
    newWave();
    score_value = 20;
    paintTimer = startTimer(10);
}

void Game::startTimers(){
    QTimer::singleShot(30,this,SLOT(moveEvent()));
    QTimer::singleShot(150,this,SLOT(moveDecals()));
    QTimer::singleShot(30,this,SLOT(collisionEvent()));
}

void Game::newWave(){
    updateWave();
    for(auto& e : enemies)
        delete e;
    enemies.clear();

    spawnList.clear();

    spawnList = wave_generator.generateSpawnList(getWave(), navPath[0]);
    enemyCount = spawnList.size();

    spawnTimer = startTimer(2000);
    startTimers();
}

void Game::clearGame(){
    Tower::resetUpgrades();

    for(auto& e : enemies)
        delete e;
    enemies.clear();
    for(auto& t : towers)
        delete t;
    towers.clear();
    for(auto& t : map){
        t->setOccupied(false);
    }
    spawnList.clear();
}

void Game::loadMenu(){
    title_line1 = mergeChars("tower",0.125,NORMAL);
    title_line2 = mergeChars("defense",0.125,NORMAL);
    start_button = new Button(mergeChars("start",0.25,NORMAL), mergeChars("start",0.25,ACTIVE));
    help_button = new Button(mergeChars("help",0.25,NORMAL), mergeChars("help",0.25,ACTIVE));
    quit_button = new Button(mergeChars("quit",0.25,NORMAL), mergeChars("quit",0.25,ACTIVE));

    int const top_margin = (height() - (title_line1->getRect()->height() + title_line2->getRect()->height() +
                           start_button->getRect()->height() + help_button->getRect()->height() +
                           quit_button->getRect()->height()))/2;

    title_line1->getRect()->moveTo( (width()-title_line1->getRect()->width())/2 , top_margin );
    title_line2->getRect()->moveTo( (width()-title_line2->getRect()->width())/2 , top_margin + title_line1->getRect()->height());
    start_button->getRect()->moveTo( (width()-start_button->getRect()->width())/2 , top_margin + title_line1->getRect()->height() + title_line2->getRect()->height());
    help_button->getRect()->moveTo( (width()-help_button->getRect()->width())/2 , top_margin + title_line1->getRect()->height() + title_line2->getRect()->height() + start_button->getRect()->height());
    quit_button->getRect()->moveTo( (width()-quit_button->getRect()->width())/2 , top_margin + title_line1->getRect()->height() + title_line2->getRect()->height() + start_button->getRect()->height() + help_button->getRect()->height());
}

void Game::cleanMenu(){
    delete title_line1;
    delete title_line2;
    delete start_button;
    delete help_button;
    delete quit_button;
}

void Game::loadInGame(){
    score_title = mergeChars("score",1,NORMAL);
    wave_title = mergeChars("wave",1,NORMAL);
    tileHighlight = new Image(CONSTANTS::HIGHLIGHT_TILE);
    towerOptions.push_back(new Image(CONSTANTS::TOWER_FIRE));
    towerOptions.push_back(new Image(CONSTANTS::TOWER_ICE));
    towerOptions.push_back(new Image(CONSTANTS::TOWER_EARTH));
    towerOptHighlight = new Image(CONSTANTS::TOWEROPT_H);

    for(int i = 0; i<3; i++){
        fire_upgrade.push_back(new Image(CONSTANTS::UPGRADE_FIRE_BASE));
        ice_upgrade.push_back(new Image(CONSTANTS::UPGRADE_ICE_BASE));
        earth_upgrade.push_back(new Image(CONSTANTS::UPGRADE_EARTH_BASE));
    }
    upgrade_icon.push_back(new Image(CONSTANTS::UPGRADE_STRENGTH));
    upgrade_icon.push_back(new Image(CONSTANTS::UPGRADE_RANGE));
    upgrade_icon.push_back(new Image(CONSTANTS::UPGRADE_RATE));

    continue_button = new Button(mergeChars("continue",0.25,NORMAL), mergeChars("continue",0.25,ACTIVE));

    wave_title->getRect()->moveTo(10,10);
    score_title->getRect()->moveTo(width()-score_title->getRect()->width()-5, 10);
    towerOptions[0]->getRect()->moveTo(width()-towerOptions[0]->getRect()->width()-5, 50);
    towerOptions[1]->getRect()->moveTo(width()-towerOptions[1]->getRect()->width()-5, 50 + towerOptions[0]->getRect()->height());
    towerOptions[2]->getRect()->moveTo(width()-towerOptions[2]->getRect()->width()-5, 50 + towerOptions[0]->getRect()->height() + towerOptions[1]->getRect()->height());

    int x = width()-towerOptions[0]->getRect()->width()-5;
    int y = 75 + towerOptions[0]->getRect()->height() + towerOptions[1]->getRect()->height() + towerOptions[2]->getRect()->height();
    for(size_t i = 0, s = fire_upgrade.size(); i < s; i++){
        fire_upgrade[i]->getRect()->moveTo(x+(fire_upgrade[i]->getRect()->width())/4, y);
        ice_upgrade[i]->getRect()->moveTo(x+(fire_upgrade[i]->getRect()->width())/4, y);
        earth_upgrade[i]->getRect()->moveTo(x+(fire_upgrade[i]->getRect()->width())/4, y);
        upgrade_icon[i]->getRect()->moveTo(x+(fire_upgrade[i]->getRect()->width())/4, y);
        y+= fire_upgrade[i]->getRect()->height()+2;
    }

    continue_button->getRect()->moveTo( (width()-continue_button->getRect()->width())/2 , 264);

    buildMap();
    createNavigationPath();
}

void Game::fillCharReferences(){
    letterChars.push_back(new Image(CHARS::CHAR_0));
    letterChars.push_back(new Image(CHARS::CHAR_1));
    letterChars.push_back(new Image(CHARS::CHAR_2));
    letterChars.push_back(new Image(CHARS::CHAR_3));
    letterChars.push_back(new Image(CHARS::CHAR_4));
    letterChars.push_back(new Image(CHARS::CHAR_5));
    letterChars.push_back(new Image(CHARS::CHAR_6));
    letterChars.push_back(new Image(CHARS::CHAR_7));
    letterChars.push_back(new Image(CHARS::CHAR_8));
    letterChars.push_back(new Image(CHARS::CHAR_9));
    letterChars.push_back(new Image(CHARS::CHAR_A));
    letterChars.push_back(new Image(CHARS::CHAR_B));
    letterChars.push_back(new Image(CHARS::CHAR_C));
    letterChars.push_back(new Image(CHARS::CHAR_D));
    letterChars.push_back(new Image(CHARS::CHAR_E));
    letterChars.push_back(new Image(CHARS::CHAR_F));
    letterChars.push_back(new Image(CHARS::CHAR_G));
    letterChars.push_back(new Image(CHARS::CHAR_H));
    letterChars.push_back(new Image(CHARS::CHAR_I));
    letterChars.push_back(new Image(CHARS::CHAR_J));
    letterChars.push_back(new Image(CHARS::CHAR_K));
    letterChars.push_back(new Image(CHARS::CHAR_L));
    letterChars.push_back(new Image(CHARS::CHAR_M));
    letterChars.push_back(new Image(CHARS::CHAR_N));
    letterChars.push_back(new Image(CHARS::CHAR_O));
    letterChars.push_back(new Image(CHARS::CHAR_P));
    letterChars.push_back(new Image(CHARS::CHAR_Q));
    letterChars.push_back(new Image(CHARS::CHAR_R));
    letterChars.push_back(new Image(CHARS::CHAR_S));
    letterChars.push_back(new Image(CHARS::CHAR_T));
    letterChars.push_back(new Image(CHARS::CHAR_U));
    letterChars.push_back(new Image(CHARS::CHAR_V));
    letterChars.push_back(new Image(CHARS::CHAR_W));
    letterChars.push_back(new Image(CHARS::CHAR_X));
    letterChars.push_back(new Image(CHARS::CHAR_Y));
    letterChars.push_back(new Image(CHARS::CHAR_Z));

    letterCharsAct.push_back(new Image(CHARS::CHAR_0_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_1_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_2_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_3_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_4_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_5_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_6_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_7_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_8_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_9_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_A_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_B_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_C_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_D_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_E_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_F_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_G_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_H_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_I_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_J_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_K_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_L_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_M_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_N_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_O_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_P_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_Q_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_R_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_S_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_T_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_U_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_V_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_W_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_X_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_Y_ACT));
    letterCharsAct.push_back(new Image(CHARS::CHAR_Z_ACT));

    letterCharsRed.push_back(new Image(CHARS::CHAR_0_RED));
    letterCharsRed.push_back(new Image(CHARS::CHAR_1_RED));
    letterCharsRed.push_back(new Image(CHARS::CHAR_2_RED));
    letterCharsRed.push_back(new Image(CHARS::CHAR_3_RED));
    letterCharsRed.push_back(new Image(CHARS::CHAR_4_RED));
    letterCharsRed.push_back(new Image(CHARS::CHAR_5_RED));
    letterCharsRed.push_back(new Image(CHARS::CHAR_6_RED));
    letterCharsRed.push_back(new Image(CHARS::CHAR_7_RED));
    letterCharsRed.push_back(new Image(CHARS::CHAR_8_RED));
    letterCharsRed.push_back(new Image(CHARS::CHAR_9_RED));

    specialChars.push_back(new Image(CHARS::CHAR_SPACE));
}

void Game::cleanInGame(){
    delete score_title;
    delete wave_title;
    delete tileHighlight;
    delete tooltip;
    for(auto& t : map)
        delete t;
    for(auto& o : towerOptions)
        delete o;
    for(auto& d : damageDisplays)
        delete d;
    for(auto& e : enemies)
        delete e;
}

void Game::loadPause(){
    pauseButtons.push_back(new Button(mergeChars("resume",0.25,NORMAL), mergeChars("resume",0.25,ACTIVE)));
    pauseButtons.push_back(new Button(mergeChars("main menu",0.25,NORMAL), mergeChars("main menu",0.25,ACTIVE)));

    int const top_margin = (height() - (pauseButtons[0]->getRect()->height() + pauseButtons[1]->getRect()->height()))/2;
    pauseButtons[0]->getRect()->moveTo( (width()-pauseButtons[0]->getRect()->width())/2 , top_margin);
    pauseButtons[1]->getRect()->moveTo( (width()-pauseButtons[1]->getRect()->width())/2 , top_margin+pauseButtons[0]->getRect()->height());
}

void Game::cleanPause(){
    for(auto& b : pauseButtons)
        delete b;
}

void Game::loadHelp(){
    arrows.push_back(new Button(CONSTANTS::LEFT_PATH, CONSTANTS::LEFT_H_PATH, 0.25));
    arrows.push_back(new Button(CONSTANTS::RIGHT_PATH, CONSTANTS::RIGHT_H_PATH, 0.25));
    arrows.push_back(new Button(mergeChars("back",0.5,NORMAL), mergeChars("back",0.5,ACTIVE)));
    helpImages.push_back(new Image(CONSTANTS::HELP_SELECT_TOWER));
    helpImages.push_back(new Image(CONSTANTS::HELP_UPGRADE));
    helpImages.push_back(new Image(CONSTANTS::HELP_BUILD_TOWER));

    arrows[2]->getRect()->moveTo( 10, 10);
    arrows[0]->getRect()->moveTo( 30, (height()-arrows[0]->getRect()->height())/2);
    arrows[1]->getRect()->moveTo( width()-30-arrows[1]->getRect()->width(), (height()-arrows[1]->getRect()->height())/2);
    for(auto& i : helpImages)
        i->getRect()->moveTo((width()-i->getRect()->width())/2, (height()-i->getRect()->height())/2);
}

void Game::cleanHelp(){
    for(auto& b : arrows)
        delete b;
    for(auto& i : helpImages)
        delete i;
}

void Game::buildMap(){
    for(const auto d : CONSTANTS::MAP)
        d==0 ?  map.push_back(new Tile(CONSTANTS::GRASS_TILE)) : map.push_back(new Tile(CONSTANTS::DIRT_TILE,d));
    int xPos = 50;
    int yPos = 50;
    int colCounter = 0;
    for(auto& t : map){
        t->getRect()->moveTo(xPos, yPos);
        if(++colCounter<CONSTANTS::TILE_COL)
            xPos += t->getRect()->width();
        else{
            xPos = 50;
            colCounter = 0;
            yPos += t->getRect()->height();
        }
    }
}

void Game::selectTile(Tile* t){
    if(!t->isActive()){
        t->setActive(true);
        tileHighlight->getRect()->moveTo(t->getRect()->topLeft());
    }
    else{
        t->setActive(false);
        switch(curTowerOpt){
            case 0:
                if(getScore() >= Tower::getCost(curTowerType)){
                    updateScore(-Tower::getCost(curTowerType));
                    towers.push_back(new Tower(CONSTANTS::TOWER_FIRE, *t->getRect()));
                    t->setOccupied(true);
                }
                break;
            case 1:
                if(getScore() >= Tower::getCost(curTowerType)){
                    updateScore(-Tower::getCost(curTowerType));
                    towers.push_back(new Tower(CONSTANTS::TOWER_ICE, *t->getRect()));
                    t->setOccupied(true);
                }
                break;
            case 2:
                if(getScore() >= Tower::getCost(curTowerType)){
                    updateScore(-Tower::getCost(curTowerType));
                    towers.push_back(new Tower(CONSTANTS::TOWER_EARTH, *t->getRect()));
                    t->setOccupied(true);
                }
                break;
        }
    }
}

void Game::raycast(){
    for(auto& t : towers){
        for(auto& e : enemies){
            int distance = QLineF(t->getRect()->center(), e->getRect()->center()).length();
            if(distance < t->getRange(t->getType()) && !t->isCoolDown()){
                t->setCoolDown(true);
                QTimer::singleShot(t->getCoolDown(t->getType()),t,SLOT(toggleCoolDown()));
                e->inflictDamage(t->getDamage(t->getType()));
                Image* damage = mergeChars(std::to_string(t->getDamage(t->getType())),1,RED);
                damage->getRect()->moveTo(e->getRect()->center().x()+damageDisplayOffset(generator), e->getRect()->top());
                damageDisplays.push_back(damage);
                QTimer::singleShot(1000,this,SLOT(removeDecal()));

                if(e->getHealth() <= 0){
                    e->setDead(true);
                    enemyCount--;
                    cleanEnemyList();
                    //End wave
                    if(enemyCount == 0)
                        state = CLEARED;
                }
                break;
            }
        }
    }
}

void Game::cleanEnemyList(){
    for(size_t i = 0; i<enemies.size(); i++){
        if(enemies[i]->isDead()){
            updateScore(enemies[i]->getScore());
            delete enemies[i];
            enemies.erase(enemies.begin()+i);
        }
    }
}

Image* Game::mergeChars(std::string word, double scale, Chars c){
    Image* image = new Image();

    for(size_t i = 0; i < word.length(); i++){
        if(c == ACTIVE){
            switch(word[i]){
            case '0':
                appendChar(letterCharsAct[0], scale, image);
                break;
            case '1':
                appendChar(letterCharsAct[1], scale, image);
                break;
            case '2':
                appendChar(letterCharsAct[2], scale, image);
                break;
            case '3':
                appendChar(letterCharsAct[3], scale, image);
                break;
            case '4':
                appendChar(letterCharsAct[4], scale, image);
                break;
            case '5':
                appendChar(letterCharsAct[5], scale, image);
                break;
            case '6':
                appendChar(letterCharsAct[6], scale, image);
                break;
            case '7':
                appendChar(letterCharsAct[7], scale, image);
                break;
            case '8':
                appendChar(letterCharsAct[8], scale, image);
                break;
            case '9':
                appendChar(letterCharsAct[9], scale, image);
                break;
            case 'a':
                appendChar(letterCharsAct[10], scale, image);
                break;
            case 'b':
                appendChar(letterCharsAct[11], scale, image);
                break;
            case 'c':
                appendChar(letterCharsAct[12], scale, image);
                break;
            case 'd':
                appendChar(letterCharsAct[13], scale, image);
                break;
            case 'e':
                appendChar(letterCharsAct[14], scale, image);
                break;
            case 'f':
                appendChar(letterCharsAct[15], scale, image);
                break;
            case 'g':
                appendChar(letterCharsAct[16], scale, image);
                break;
            case 'h':
                appendChar(letterCharsAct[17], scale, image);
                break;
            case 'i':
                appendChar(letterCharsAct[18], scale, image);
                break;
            case 'j':
                appendChar(letterCharsAct[19], scale, image);
                break;
            case 'k':
                appendChar(letterCharsAct[20], scale, image);
                break;
            case 'l':
                appendChar(letterCharsAct[21], scale, image);
                break;
            case 'm':
                appendChar(letterCharsAct[22], scale, image);
                break;
            case 'n':
                appendChar(letterCharsAct[23], scale, image);
                break;
            case 'o':
                appendChar(letterCharsAct[24], scale, image);
                break;
            case 'p':
                appendChar(letterCharsAct[25], scale, image);
                break;
            case 'q':
                appendChar(letterCharsAct[26], scale, image);
                break;
            case 'r':
                appendChar(letterCharsAct[27], scale, image);
                break;
            case 's':
                appendChar(letterCharsAct[28], scale, image);
                break;
            case 't':
                appendChar(letterCharsAct[29], scale, image);
                break;
            case 'u':
                appendChar(letterCharsAct[30], scale, image);
                break;
            case 'v':
                appendChar(letterCharsAct[31], scale, image);
                break;
            case 'w':
                appendChar(letterCharsAct[32], scale, image);
                break;
            case 'x':
                appendChar(letterCharsAct[33], scale, image);
                break;
            case 'y':
                appendChar(letterCharsAct[34], scale, image);
                break;
            case 'z':
                appendChar(letterCharsAct[35], scale, image);
                break;
            case ' ':
                appendChar(specialChars[0], scale, image);
                break;
        }
        }
        else if(c == NORMAL){
            switch(word[i]){
                case '0':
                    appendChar(letterChars[0], scale, image);
                    break;
                case '1':
                    appendChar(letterChars[1], scale, image);
                    break;
                case '2':
                    appendChar(letterChars[2], scale, image);
                    break;
                case '3':
                    appendChar(letterChars[3], scale, image);
                    break;
                case '4':
                    appendChar(letterChars[4], scale, image);
                    break;
                case '5':
                    appendChar(letterChars[5], scale, image);
                    break;
                case '6':
                    appendChar(letterChars[6], scale, image);
                    break;
                case '7':
                    appendChar(letterChars[7], scale, image);
                    break;
                case '8':
                    appendChar(letterChars[8], scale, image);
                    break;
                case '9':
                    appendChar(letterChars[9], scale, image);
                    break;
                case 'a':
                    appendChar(letterChars[10], scale, image);
                    break;
                case 'b':
                    appendChar(letterChars[11], scale, image);
                    break;
                case 'c':
                    appendChar(letterChars[12], scale, image);
                    break;
                case 'd':
                    appendChar(letterChars[13], scale, image);
                    break;
                case 'e':
                    appendChar(letterChars[14], scale, image);
                    break;
                case 'f':
                    appendChar(letterChars[15], scale, image);
                    break;
                case 'g':
                    appendChar(letterChars[16], scale, image);
                    break;
                case 'h':
                    appendChar(letterChars[17], scale, image);
                    break;
                case 'i':
                    appendChar(letterChars[18], scale, image);
                    break;
                case 'j':
                    appendChar(letterChars[19], scale, image);
                    break;
                case 'k':
                    appendChar(letterChars[20], scale, image);
                    break;
                case 'l':
                    appendChar(letterChars[21], scale, image);
                    break;
                case 'm':
                    appendChar(letterChars[22], scale, image);
                    break;
                case 'n':
                    appendChar(letterChars[23], scale, image);
                    break;
                case 'o':
                    appendChar(letterChars[24], scale, image);
                    break;
                case 'p':
                    appendChar(letterChars[25], scale, image);
                    break;
                case 'q':
                    appendChar(letterChars[26], scale, image);
                    break;
                case 'r':
                    appendChar(letterChars[27], scale, image);
                    break;
                case 's':
                    appendChar(letterChars[28], scale, image);
                    break;
                case 't':
                    appendChar(letterChars[29], scale, image);
                    break;
                case 'u':
                    appendChar(letterChars[30], scale, image);
                    break;
                case 'v':
                    appendChar(letterChars[31], scale, image);
                    break;
                case 'w':
                    appendChar(letterChars[32], scale, image);
                    break;
                case 'x':
                    appendChar(letterChars[33], scale, image);
                    break;
                case 'y':
                    appendChar(letterChars[34], scale, image);
                    break;
                case 'z':
                    appendChar(letterChars[35], scale, image);
                    break;
                case ' ':
                    appendChar(specialChars[0], scale, image);
                    break;
            }
        }
        else if(c == RED){
            switch(word[i]){
                case '0':
                    appendChar(letterCharsRed[0], scale, image);
                    break;
                case '1':
                    appendChar(letterCharsRed[1], scale, image);
                    break;
                case '2':
                    appendChar(letterCharsRed[2], scale, image);
                    break;
                case '3':
                    appendChar(letterCharsRed[3], scale, image);
                    break;
                case '4':
                    appendChar(letterCharsRed[4], scale, image);
                    break;
                case '5':
                    appendChar(letterCharsRed[5], scale, image);
                    break;
                case '6':
                    appendChar(letterCharsRed[6], scale, image);
                    break;
                case '7':
                    appendChar(letterCharsRed[7], scale, image);
                    break;
                case '8':
                    appendChar(letterCharsRed[8], scale, image);
                    break;
                case '9':
                    appendChar(letterCharsRed[9], scale, image);
                    break;
            }
        }
    }

    return image;
}

void Game::appendChar(Image* character, double scale, Image* i){
    Image* copy = character->scaledCopy(scale);
    i->append(copy);
}

void Game::printChar(Image* character, double scale, QPainter& p, int& x, int& y){
    Image* copy = character->scaledCopy(scale);
    copy->getRect()->moveTo(x,y);
    p.drawImage(*copy->getRect(),*copy->getImage());
    x += copy->getRect()->width();
    delete copy;
}

void Game::paintChar(std::string word, double scale, QPainter& p, int x, int y, bool active){
    for(size_t i = 0; i < word.length(); i++){
        if(active){
            switch(word[i]){
            case '0':
                printChar(letterCharsAct[0], scale, p, x, y);
                break;
            case '1':
                printChar(letterCharsAct[1], scale, p, x, y);
                break;
            case '2':
                printChar(letterCharsAct[2], scale, p, x, y);
                break;
            case '3':
                printChar(letterCharsAct[3], scale, p, x, y);
                break;
            case '4':
                printChar(letterCharsAct[4], scale, p, x, y);
                break;
            case '5':
                printChar(letterCharsAct[5], scale, p, x, y);
                break;
            case '6':
                printChar(letterCharsAct[6], scale, p, x, y);
                break;
            case '7':
                printChar(letterCharsAct[7], scale, p, x, y);
                break;
            case '8':
                printChar(letterCharsAct[8], scale, p, x, y);
                break;
            case '9':
                printChar(letterCharsAct[9], scale, p, x, y);
                break;
            case 'a':
                printChar(letterCharsAct[10], scale, p, x, y);
                break;
            case 'b':
                printChar(letterCharsAct[11], scale, p, x, y);
                break;
            case 'c':
                printChar(letterCharsAct[12], scale, p, x, y);
                break;
            case 'd':
                printChar(letterCharsAct[13], scale, p, x, y);
                break;
            case 'e':
                printChar(letterCharsAct[14], scale, p, x, y);
                break;
            case 'f':
                printChar(letterCharsAct[15], scale, p, x, y);
                break;
            case 'g':
                printChar(letterCharsAct[16], scale, p, x, y);
                break;
            case 'h':
                printChar(letterCharsAct[17], scale, p, x, y);
                break;
            case 'i':
                printChar(letterCharsAct[18], scale, p, x, y);
                break;
            case 'j':
                printChar(letterCharsAct[19], scale, p, x, y);
                break;
            case 'k':
                printChar(letterCharsAct[20], scale, p, x, y);
                break;
            case 'l':
                printChar(letterCharsAct[21], scale, p, x, y);
                break;
            case 'm':
                printChar(letterCharsAct[22], scale, p, x, y);
                break;
            case 'n':
                printChar(letterCharsAct[23], scale, p, x, y);
                break;
            case 'o':
                printChar(letterCharsAct[24], scale, p, x, y);
                break;
            case 'p':
                printChar(letterCharsAct[25], scale, p, x, y);
                break;
            case 'q':
                printChar(letterCharsAct[26], scale, p, x, y);
                break;
            case 'r':
                printChar(letterCharsAct[27], scale, p, x, y);
                break;
            case 's':
                printChar(letterCharsAct[28], scale, p, x, y);
                break;
            case 't':
                printChar(letterCharsAct[29], scale, p, x, y);
                break;
            case 'u':
                printChar(letterCharsAct[30], scale, p, x, y);
                break;
            case 'v':
                printChar(letterCharsAct[31], scale, p, x, y);
                break;
            case 'w':
                printChar(letterCharsAct[32], scale, p, x, y);
                break;
            case 'x':
                printChar(letterCharsAct[33], scale, p, x, y);
                break;
            case 'y':
                printChar(letterCharsAct[34], scale, p, x, y);
                break;
            case 'z':
                printChar(letterCharsAct[35], scale, p, x, y);
                break;
            case ' ':
                printChar(specialChars[0], scale, p, x, y);
                break;
        }
        }
        else{
            switch(word[i]){
                case '0':
                    printChar(letterChars[0], scale, p, x, y);
                    break;
                case '1':
                    printChar(letterChars[1], scale, p, x, y);
                    break;
                case '2':
                    printChar(letterChars[2], scale, p, x, y);
                    break;
                case '3':
                    printChar(letterChars[3], scale, p, x, y);
                    break;
                case '4':
                    printChar(letterChars[4], scale, p, x, y);
                    break;
                case '5':
                    printChar(letterChars[5], scale, p, x, y);
                    break;
                case '6':
                    printChar(letterChars[6], scale, p, x, y);
                    break;
                case '7':
                    printChar(letterChars[7], scale, p, x, y);
                    break;
                case '8':
                    printChar(letterChars[8], scale, p, x, y);
                    break;
                case '9':
                    printChar(letterChars[9], scale, p, x, y);
                    break;
                case 'a':
                    printChar(letterChars[10], scale, p, x, y);
                    break;
                case 'b':
                    printChar(letterChars[11], scale, p, x, y);
                    break;
                case 'c':
                    printChar(letterChars[12], scale, p, x, y);
                    break;
                case 'd':
                    printChar(letterChars[13], scale, p, x, y);
                    break;
                case 'e':
                    printChar(letterChars[14], scale, p, x, y);
                    break;
                case 'f':
                    printChar(letterChars[15], scale, p, x, y);
                    break;
                case 'g':
                    printChar(letterChars[16], scale, p, x, y);
                    break;
                case 'h':
                    printChar(letterChars[17], scale, p, x, y);
                    break;
                case 'i':
                    printChar(letterChars[18], scale, p, x, y);
                    break;
                case 'j':
                    printChar(letterChars[19], scale, p, x, y);
                    break;
                case 'k':
                    printChar(letterChars[20], scale, p, x, y);
                    break;
                case 'l':
                    printChar(letterChars[21], scale, p, x, y);
                    break;
                case 'm':
                    printChar(letterChars[22], scale, p, x, y);
                    break;
                case 'n':
                    printChar(letterChars[23], scale, p, x, y);
                    break;
                case 'o':
                    printChar(letterChars[24], scale, p, x, y);
                    break;
                case 'p':
                    printChar(letterChars[25], scale, p, x, y);
                    break;
                case 'q':
                    printChar(letterChars[26], scale, p, x, y);
                    break;
                case 'r':
                    printChar(letterChars[27], scale, p, x, y);
                    break;
                case 's':
                    printChar(letterChars[28], scale, p, x, y);
                    break;
                case 't':
                    printChar(letterChars[29], scale, p, x, y);
                    break;
                case 'u':
                    printChar(letterChars[30], scale, p, x, y);
                    break;
                case 'v':
                    printChar(letterChars[31], scale, p, x, y);
                    break;
                case 'w':
                    printChar(letterChars[32], scale, p, x, y);
                    break;
                case 'x':
                    printChar(letterChars[33], scale, p, x, y);
                    break;
                case 'y':
                    printChar(letterChars[34], scale, p, x, y);
                    break;
                case 'z':
                    printChar(letterChars[35], scale, p, x, y);
                    break;
                case ' ':
                    printChar(specialChars[0], scale, p, x, y);
                    break;
            }
        }
    }

}

void Game::createNavigationPath(){
    for(auto& t : map){
        if(t->isPath())
            navPath[t->getPathID()-1] = t->getRect()->center();
    }
}

Game::ToolTip::ToolTip(Image* s, Image* s_u, Image* c, Image* c_a) : upgrade(true)
{
    cost = c;
    cost_amount = c_a;
    stat = s;
    stat_upgrade = s_u;
    background = new Image(TOOLTIP::BASE);
}

Game::ToolTip::ToolTip(Image* c, Image* c_a) : upgrade(false)
{
    stat = c;
    stat_upgrade = c_a;
    background = new Image(TOOLTIP::BASE);
}

Game::ToolTip::~ToolTip(){
    delete background;
    delete stat;
    delete stat_upgrade;
    if(upgrade){
        delete cost;
        delete cost_amount;
    }
}

void Game::ToolTip::moveTo(QPointF position){
    int x = position.x();
    int y = position.y();
    resizeBackground();
    background->getRect()->moveTo(x-background->getRect()->width(), y);
    stat->getRect()->moveTo(background->getRect()->x()+2, background->getRect()->y()+2);
    stat_upgrade->getRect()->moveTo(stat->getRect()->right()+3, stat->getRect()->y());
    if(upgrade){
        cost->getRect()->moveTo(stat_upgrade->getRect()->right()+5, stat_upgrade->getRect()->y());
        cost_amount->getRect()->moveTo(cost->getRect()->right()+3, cost->getRect()->y());
    }
}

void Game::ToolTip::paint(QPainter *p){
    p->drawImage(*background->getRect(), *background->getImage());
    p->drawImage(*stat->getRect(), *stat->getImage());
    p->drawImage(*stat_upgrade->getRect(), *stat_upgrade->getImage());
    if(upgrade){
        p->drawImage(*cost->getRect(), *cost->getImage());
        p->drawImage(*cost_amount->getRect(), *cost_amount->getImage());
    }
}

void Game::ToolTip::resizeBackground(){
    int width = 2 + stat->getRect()->width() + 3 + stat_upgrade->getRect()->width();
    upgrade ? width += 5 + cost->getRect()->width() + 3 + cost_amount->getRect()->width() : width += 0;
    int height = stat->getRect()->height() + 4;
    background->setImage(background->getImage()->scaled(width, height, Qt::IgnoreAspectRatio));
    background->setRect(background->getImage()->rect());
}

