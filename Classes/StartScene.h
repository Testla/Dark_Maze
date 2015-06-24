#ifndef __START_SCENE_H__
#define __START_SCENE_H__

#include "cocos2d.h"
USING_NS_CC;
class StartScene : public cocos2d::Layer
{

public:
	static cocos2d::Scene* createScene();

	virtual bool init();

	CREATE_FUNC(StartScene);

private:
	Size winSize;
	Sprite *information;
	Menu *menu;
	MenuItemLabel *item;

	void start(Ref* ref);
	void showInformation1();
	void showInformation2();
	void returnToMenu();
};

#endif // __START_SCENE_H__
