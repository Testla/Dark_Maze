#ifndef __SELECT_LEVEL_H__
#define __SELECT_LEVEL_H__

#include "cocos2d.h"
USING_NS_CC;
class SelectLevel : public cocos2d::Layer
{

public:
	static cocos2d::Scene* createScene();

	virtual bool init();

	CREATE_FUNC(SelectLevel);

private:
	Size winSize;
	EventDispatcher *dispatcher;
};

#endif // __SELECT_LEVEL_H__
