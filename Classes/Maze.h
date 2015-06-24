#ifndef __MAZE_H__
#define __MAZE_H__

#include "cocos2d.h"
#include "Matrix.hpp"
USING_NS_CC;
class Maze : public cocos2d::Layer
{

public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    CREATE_FUNC(Maze);

	~Maze();
	static const std::pair<int, int> screenSize;
	static std::pair<int, int> mazeSize;
	static const Size gridSize;
private:
	Layer *mazeLayer, *playerLayer;
	Size winSize;
	EventDispatcher *dispatcher;
	std::pair<int, int> end, playerPosition, monsterPosition;
	Action *loopMove, *walkingAnimation;
	Matrix<char> *maze;
	Sprite *player, *cover;
	Sprite *monster;

	bool keyIsHolding[4];
	int holdingCount;

	void initKeyboardEvent();
	void initMaze();
	
	std::pair<int, int> directions[4];
	/* 
	 * currentDirection : the direction of current actual graphic
	                      , can only be modified by doMove()
	 * comingDirection : the direction to go next
	 */
	int comingDirection, currentDirection;
	int monsterComingDirection;
	void startMoving(int direction);
	void stopMoving();
	void doMove();
	void chooseMoveAction();

	void createTornado();
	void createMonster();
	void monsterDoMove();
	void monsterChooseMoveAction();
	Vec2 monsterposition(std::pair<int, int> position);

	Vec2 playerLayerRightPosition, mazeLayerRightPosition;
	void calculateRightPosition();
	Layer *layerToSetRightPosition;
	FiniteTimeAction *doSetRightPosition();

	void createInvisibleCloak();
	void createTorch();

	void gameOver(bool winOrLose);
	void back(Ref *ref);
};

#endif // __MAZE_H__
