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
private:
	const Size gridSize = Size(50.0f, 50.0f);

	Layer *viewLayer;
	Size winSize;
	EventDispatcher *dispatcher;
	std::pair<int, int> end, playerPosition;
	Action *loopMove, *walkingAnimation;
	Matrix<char> *maze;
	Sprite *player;

	void initKeyboardEvent();
	void initMaze(std::pair<int, int> Maze_Size);
	Vec2 playerLayerPosition(std::pair<int, int> position);

	std::pair<int, int> directions[4];
	/* 
	 * currentDirection : the direction of current actual graphic
	                      , can only be modified by doMove()
	 * comingDirection : the direction to go next
	 */
	int comingDirection, currentDirection;
	void startMoving(int direction);
	void stopMoving();
	void doMove();
	void chooseMoveAction();
};

#endif // __MAZE_H__
