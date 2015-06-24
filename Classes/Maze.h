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

	void initUpdateEvent();
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

	void createMonster();
	void monsterDoMove();
	void monsterChooseMoveAction();
	Vec2 monsterposition(std::pair<int, int> position);

	Vec2 playerLayerRightPosition, mazeLayerRightPosition;
	void calculateRightPosition();
	Layer *layerToSetRightPosition;
	FiniteTimeAction *doSetRightPosition();

	void gameOver(bool winOrLose);
	void back(Ref *ref);

	enum ItemType {
		Tornado,
		SpeedUp,
		SpeedDown,
		Torch,
		NumberOfItemTypes
	};
	struct Item {
		ItemType itemType;
		std::pair<int, int> position;
		Sprite *itemSprite;
	};
	/* only compares position */
	struct ItemCompare {
		bool operator()(const Item &a, const Item &b) {
			if (a.position == b.position)
				return false;
			if (a.position.first < b.position.first)
				return true;
			else if (a.position.first == b.position.first)
				return a.position.second < b.position.second;
			else
				return false;
		}
	};
	std::set<Item, ItemCompare> itemSet;
	Sprite* createTornado(std::pair<int, int> position);
	void addItem(const ItemType itemType, const std::pair<int, int> itemPosition);
	void addItemsRandomly();
	void Maze::checkItemTrigger();
	float playerSpeed;  // grids per second
	void tornadoCallback();
	void speedUpCallback();
	void endSpeedUp();
	void endSpeedDown();
	void speedDownCallback();
	int torchCount;
	void torchCallback();
	void endTorch();
};

#endif // __MAZE_H__
